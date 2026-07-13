/*

Pencil2D - Traditional Animation Software
Copyright (C) 2012-2020 Matthew Chiawen Chang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/
#include "catch.hpp"

#include <QAction>
#include <QIcon>

#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>

#include "object.h"
#include "editor.h"
#include "scribblearea.h"
#include "layermanager.h"
#include "soundmanager.h"
#include "clipboardmanager.h"
#include "selectionmanager.h"
#include "undoredomanager.h"
#include "undoredocommand.h"
#include "layerbitmap.h"
#include "layersound.h"
#include "layercamera.h"
#include "bitmapimage.h"
#include "soundclip.h"
#include "camera.h"

namespace
{

// Editor wired up like the application does it, with the new undo/redo
// system enabled and a single bitmap layer holding a keyframe at 1.
struct UndoRedoTestScene
{
    UndoRedoTestScene()
    {
        object = new Object;
        object->init();
        object->addNewBitmapLayer();

        editor = new Editor;
        scribbleArea = new ScribbleArea(nullptr);
        scribbleArea->setEditor(editor);
        editor->setScribbleArea(scribbleArea);
        editor->setObject(object); // editor takes ownership
        editor->init();
        editor->undoRedo()->setNewBackupSystemEnabled(true);

        undoAction = editor->undoRedo()->createUndoAction(nullptr, QIcon());
        redoAction = editor->undoRedo()->createRedoAction(nullptr, QIcon());
    }

    ~UndoRedoTestScene()
    {
        delete undoAction;
        delete redoAction;
        delete editor;
        delete scribbleArea;
    }

    LayerBitmap* bitmapLayer() const
    {
        return static_cast<LayerBitmap*>(editor->layers()->currentLayer());
    }

    Object* object = nullptr;
    Editor* editor = nullptr;
    ScribbleArea* scribbleArea = nullptr;
    QAction* undoAction = nullptr;
    QAction* redoAction = nullptr;
};

const QRgb red = qPremultiply(QColor(255, 0, 0).rgba());

// Writes a minimal but valid 16-bit mono PCM WAV file.
void writeTestWavFile(const QString& path)
{
    QFile file(path);
    REQUIRE(file.open(QIODevice::WriteOnly));

    const quint32 sampleRate = 8000;
    const quint16 channels = 1;
    const quint16 bitsPerSample = 16;
    const QByteArray samples(800, '\0'); // 400 samples of silence

    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);
    out.writeRawData("RIFF", 4);
    out << quint32(36 + samples.size());
    out.writeRawData("WAVE", 4);
    out.writeRawData("fmt ", 4);
    out << quint32(16) << quint16(1) << channels << sampleRate
        << quint32(sampleRate * channels * bitsPerSample / 8)
        << quint16(channels * bitsPerSample / 8) << bitsPerSample;
    out.writeRawData("data", 4);
    out << quint32(samples.size());
    out.writeRawData(samples.constData(), samples.size());
}

// The media backend probes sound files on a worker thread and posts results
// back via queued events. Give those events a chance to be delivered before
// asserting or tearing the scene down, otherwise the pooled thread can
// outlive the objects it reports to.
void pumpEvents(int ms = 100)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < ms)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
}

} // namespace

TEST_CASE("UndoTransaction commit records an undoable bitmap change")
{
    UndoRedoTestScene scene;
    LayerBitmap* layer = scene.bitmapLayer();
    REQUIRE(layer->type() == Layer::BITMAP);

    BitmapImage* image = layer->getBitmapImageAtFrame(1);
    REQUIRE(image != nullptr);
    REQUIRE(image->constScanLine(3, 4) == 0);

    UndoTransaction transaction = scene.editor->undoRedo()->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY);
    REQUIRE(transaction.isActive());

    image->setPixel(3, 4, red);
    transaction.commit("stroke");

    REQUIRE_FALSE(transaction.isActive());
    REQUIRE(scene.editor->undoRedo()->hasUnsavedChanges());

    SECTION("undo restores the previous image")
    {
        scene.undoAction->trigger();

        image = layer->getBitmapImageAtFrame(1);
        REQUIRE(image != nullptr);
        REQUIRE(image->constScanLine(3, 4) == 0);
    }

    SECTION("undo then redo reapplies the change")
    {
        scene.undoAction->trigger();
        scene.redoAction->trigger();

        image = layer->getBitmapImageAtFrame(1);
        REQUIRE(image != nullptr);
        REQUIRE(image->constScanLine(3, 4) == red);
    }
}

TEST_CASE("UndoTransaction going out of scope uncommitted records nothing")
{
    UndoRedoTestScene scene;
    BitmapImage* image = scene.bitmapLayer()->getBitmapImageAtFrame(1);

    {
        UndoTransaction transaction = scene.editor->undoRedo()->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY);
        REQUIRE(transaction.isActive());
        image->setPixel(3, 4, red);
        // no commit — e.g. a cancelled stroke
    }

    REQUIRE_FALSE(scene.editor->undoRedo()->hasUnsavedChanges());
    REQUIRE_FALSE(scene.undoAction->isEnabled());
}

TEST_CASE("UndoTransaction captures an explicit layer even when the current layer changes")
{
    UndoRedoTestScene scene;
    LayerBitmap* firstLayer = scene.bitmapLayer();
    scene.object->addNewBitmapLayer();

    BitmapImage* image = firstLayer->getBitmapImageAtFrame(1);

    UndoTransaction transaction = scene.editor->undoRedo()->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY,
                                                                             firstLayer->id(), 1);
    REQUIRE(transaction.isActive());

    // The current layer moves away mid-gesture; the transaction must not follow it.
    scene.editor->layers()->setCurrentLayer(1);

    image->setPixel(3, 4, red);
    transaction.commit("stroke");

    scene.undoAction->trigger();
    REQUIRE(firstLayer->getBitmapImageAtFrame(1)->constScanLine(3, 4) == 0);

    scene.redoAction->trigger();
    REQUIRE(firstLayer->getBitmapImageAtFrame(1)->constScanLine(3, 4) == red);
}

TEST_CASE("UndoTransaction is inactive when the new undo/redo system is disabled")
{
    UndoRedoTestScene scene;
    scene.editor->undoRedo()->setNewBackupSystemEnabled(false);

    UndoTransaction transaction = scene.editor->undoRedo()->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY);
    REQUIRE_FALSE(transaction.isActive());

    transaction.commit("stroke"); // must be a harmless no-op
    REQUIRE_FALSE(scene.editor->undoRedo()->hasUnsavedChanges());
}

TEST_CASE("A default-constructed UndoTransaction is inert")
{
    UndoTransaction transaction;
    REQUIRE_FALSE(transaction.isActive());
    transaction.commit("nothing");
    transaction.discard();
}

TEST_CASE("Removing a sound keyframe is undoable and restores a playable clip")
{
    UndoRedoTestScene scene;

    LayerSound* soundLayer = scene.object->addNewSoundLayer();
    scene.editor->layers()->setCurrentLayer(soundLayer);

    const QString soundPath = QDir::temp().filePath("pencil2d_test_undo_sound.wav");
    writeTestWavFile(soundPath);

    SoundClip* clip = new SoundClip;
    REQUIRE(scene.editor->sound()->loadSound(clip, soundPath).ok());
    soundLayer->addKeyFrame(1, clip);
    REQUIRE(soundLayer->keyExists(1));
    pumpEvents();

    scene.editor->scrubTo(1);
    scene.editor->removeKey();
    REQUIRE_FALSE(soundLayer->keyExists(1));
    pumpEvents();

    scene.undoAction->trigger();
    REQUIRE(soundLayer->keyExists(1));
    SoundClip* restored = static_cast<SoundClip*>(soundLayer->getKeyFrameAt(1));
    REQUIRE(scene.editor->sound()->hasValidPlayer(restored)); // media player recreated
    pumpEvents();

    scene.redoAction->trigger();
    REQUIRE_FALSE(soundLayer->keyExists(1));
    pumpEvents();

    QFile::remove(soundPath);
}

TEST_CASE("Clearing the image is undoable")
{
    UndoRedoTestScene scene;
    LayerBitmap* layer = scene.bitmapLayer();
    BitmapImage* image = layer->getBitmapImageAtFrame(1);
    image->setPixel(3, 4, red);

    scene.scribbleArea->clearImage();
    REQUIRE(layer->getBitmapImageAtFrame(1)->constScanLine(3, 4) == 0);

    scene.undoAction->trigger();
    REQUIRE(layer->getBitmapImageAtFrame(1)->constScanLine(3, 4) == red);

    scene.redoAction->trigger();
    REQUIRE(layer->getBitmapImageAtFrame(1)->constScanLine(3, 4) == 0);
}

TEST_CASE("Modifying a camera keyframe is undoable")
{
    UndoRedoTestScene scene;
    LayerCamera* cameraLayer = scene.object->addNewCameraLayer(); // has a keyframe at 1

    Camera* camera = cameraLayer->getCameraAtFrame(1);
    REQUIRE(camera != nullptr);
    const QPointF originalTranslation = camera->translation();

    UndoTransaction transaction = scene.editor->undoRedo()->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY,
                                                                             cameraLayer->id(), 1);
    REQUIRE(transaction.isActive());

    camera->translate(QPointF(50, 25));
    camera->updateViewTransform();
    transaction.commit("camera move");
    const QPointF movedTranslation = cameraLayer->getCameraAtFrame(1)->translation();
    REQUIRE(movedTranslation != originalTranslation);

    scene.undoAction->trigger();
    REQUIRE(cameraLayer->getCameraAtFrame(1)->translation() == originalTranslation);

    scene.redoAction->trigger();
    REQUIRE(cameraLayer->getCameraAtFrame(1)->translation() == movedTranslation);
}

TEST_CASE("A camera reset touching several keyframes undoes as one step")
{
    UndoRedoTestScene scene;
    LayerCamera* cameraLayer = scene.object->addNewCameraLayer(); // keyframe at 1
    cameraLayer->addNewKeyFrameAt(10);

    Camera* firstCamera = cameraLayer->getCameraAtFrame(1);
    Camera* secondCamera = cameraLayer->getCameraAtFrame(10);
    firstCamera->translate(QPointF(30, 0));
    secondCamera->translate(QPointF(0, 40));
    const QPointF firstMoved = firstCamera->translation();
    const QPointF secondMoved = secondCamera->translation();

    // Mimic CameraContextMenu::resetTransform's multi-keyframe capture.
    UndoRedoManager* undoRedo = scene.editor->undoRedo();
    undoRedo->beginMacro("Camera transform reset");
    UndoTransaction firstTransaction = undoRedo->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY,
                                                                  cameraLayer->id(), 1);
    UndoTransaction secondTransaction = undoRedo->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY,
                                                                   cameraLayer->id(), 10);
    firstCamera->translate(QPointF(0, 0));
    secondCamera->translate(QPointF(0, 0));
    firstTransaction.commit("Camera transform reset");
    secondTransaction.commit("Camera transform reset");
    undoRedo->endMacro();

    REQUIRE(cameraLayer->getCameraAtFrame(1)->translation() == QPointF(0, 0));
    REQUIRE(cameraLayer->getCameraAtFrame(10)->translation() == QPointF(0, 0));

    // One undo step restores both keyframes.
    scene.undoAction->trigger();
    REQUIRE(cameraLayer->getCameraAtFrame(1)->translation() == firstMoved);
    REQUIRE(cameraLayer->getCameraAtFrame(10)->translation() == secondMoved);

    scene.redoAction->trigger();
    REQUIRE(cameraLayer->getCameraAtFrame(1)->translation() == QPointF(0, 0));
    REQUIRE(cameraLayer->getCameraAtFrame(10)->translation() == QPointF(0, 0));
}

TEST_CASE("A macro that records nothing leaves no undo entry")
{
    UndoRedoTestScene scene;
    LayerCamera* cameraLayer = scene.object->addNewCameraLayer();

    UndoRedoManager* undoRedo = scene.editor->undoRedo();
    undoRedo->beginMacro("Camera transform reset");
    UndoTransaction transaction = undoRedo->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY,
                                                             cameraLayer->id(), 1);
    // Nothing is modified — e.g. "Reset rotation" on an already-default
    // camera. The unchanged-comparison drops the command, and the empty
    // macro must not become a phantom undo step.
    transaction.commit("Camera transform reset");
    undoRedo->endMacro();

    REQUIRE_FALSE(undoRedo->hasUnsavedChanges());
    REQUIRE_FALSE(scene.undoAction->isEnabled());
}

TEST_CASE("Pasting frames is undoable as a single step")
{
    UndoRedoTestScene scene;
    LayerBitmap* layer = scene.bitmapLayer();
    layer->getBitmapImageAtFrame(1)->setPixel(3, 4, red);

    layer->setFrameSelected(1, true);
    scene.editor->clipboards()->copySelectedFrames(layer);
    layer->deselectAll();

    scene.editor->scrubTo(5);
    scene.editor->paste();
    REQUIRE(layer->keyExists(5));
    REQUIRE(layer->getBitmapImageAtFrame(5)->constScanLine(3, 4) == red);

    // One undo removes the pasted frame.
    scene.undoAction->trigger();
    REQUIRE_FALSE(layer->keyExists(5));
    REQUIRE(layer->keyExists(1));

    // Redo restores it including its content, not as an empty frame.
    scene.redoAction->trigger();
    REQUIRE(layer->keyExists(5));
    REQUIRE(layer->getBitmapImageAtFrame(5)->constScanLine(3, 4) == red);
}

TEST_CASE("Pasting onto an occupied frame shifts it; undo restores the layout")
{
    const QRgb blue = qPremultiply(QColor(0, 0, 255).rgba());

    UndoRedoTestScene scene;
    LayerBitmap* layer = scene.bitmapLayer();
    layer->getBitmapImageAtFrame(1)->setPixel(3, 4, red);

    layer->setFrameSelected(1, true);
    scene.editor->clipboards()->copySelectedFrames(layer);
    layer->deselectAll();

    // Make the original distinguishable from the pasted copy.
    layer->getBitmapImageAtFrame(1)->setPixel(5, 5, blue);

    // Paste onto frame 1: the original moves to 2, the copy lands on 1.
    scene.editor->paste();
    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(2));
    REQUIRE(layer->getBitmapImageAtFrame(1)->constScanLine(5, 5) == 0);
    REQUIRE(layer->getBitmapImageAtFrame(2)->constScanLine(5, 5) == blue);

    // One undo removes the copy and moves the original back.
    scene.undoAction->trigger();
    REQUIRE(layer->keyExists(1));
    REQUIRE_FALSE(layer->keyExists(2));
    REQUIRE(layer->getBitmapImageAtFrame(1)->constScanLine(5, 5) == blue);

    scene.redoAction->trigger();
    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(2));
    REQUIRE(layer->getBitmapImageAtFrame(1)->constScanLine(5, 5) == 0);
    REQUIRE(layer->getBitmapImageAtFrame(2)->constScanLine(5, 5) == blue);
}

TEST_CASE("Undo navigates to the affected layer and frame")
{
    UndoRedoTestScene scene;
    LayerBitmap* firstLayer = scene.bitmapLayer();
    scene.object->addNewBitmapLayer();

    // Record a change on the first layer at frame 1.
    UndoTransaction transaction = scene.editor->undoRedo()->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY,
                                                                             firstLayer->id(), 1);
    firstLayer->getBitmapImageAtFrame(1)->setPixel(3, 4, red);
    transaction.commit("stroke");

    // Wander off to another layer and frame.
    scene.editor->layers()->setCurrentLayer(1);
    scene.editor->scrubTo(9);

    // Undo brings the view back to where the change happened.
    scene.undoAction->trigger();
    REQUIRE(scene.editor->layers()->currentLayer() == firstLayer);
    REQUIRE(scene.editor->currentFrame() == 1);
}

TEST_CASE("A gesture that changes nothing records nothing")
{
    UndoRedoTestScene scene;

    UndoTransaction transaction = scene.editor->undoRedo()->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY);
    REQUIRE(transaction.isActive());
    // No pixels touched, no selection change — e.g. a plain click with a
    // drawing tool.
    transaction.commit("click");

    REQUIRE_FALSE(scene.editor->undoRedo()->hasUnsavedChanges());
    REQUIRE_FALSE(scene.undoAction->isEnabled());
}

TEST_CASE("A selection-only change is undoable even though no pixel changed")
{
    UndoRedoTestScene scene;
    auto selectMan = scene.editor->select();

    UndoTransaction transaction = scene.editor->undoRedo()->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY);
    selectMan->setSelection(QRectF(2, 2, 10, 10));
    transaction.commit("selection");

    REQUIRE(scene.editor->undoRedo()->hasUnsavedChanges());

    scene.undoAction->trigger();
    REQUIRE(selectMan->mySelectionRect() == QRectF());

    scene.redoAction->trigger();
    QRectF r = selectMan->mySelectionRect();
    INFO("rect: " << r.x() << "," << r.y() << " " << r.width() << "x" << r.height());
    REQUIRE(r == QRectF(2, 2, 10, 10));
}

TEST_CASE("Changing keyframe opacity is undoable and consecutive changes merge")
{
    UndoRedoTestScene scene;
    LayerBitmap* layer = scene.bitmapLayer();
    BitmapImage* image = layer->getBitmapImageAtFrame(1);
    REQUIRE(image->getOpacity() == 1.0);

    // Two consecutive changes to the same keyframe, recorded the way the
    // opacity dialog does (apply first, then push old -> new).
    image->setOpacity(0.5);
    scene.editor->undoRedo()->push(new KeyFrameOpacityCommand(layer->id(), { 1 }, { 1.0 }, { 0.5 },
                                                              "Change opacity", scene.editor));
    image->setOpacity(0.25);
    scene.editor->undoRedo()->push(new KeyFrameOpacityCommand(layer->id(), { 1 }, { 0.5 }, { 0.25 },
                                                              "Change opacity", scene.editor));

    // The commands merged: one undo restores the original value and the
    // stack is exhausted.
    scene.undoAction->trigger();
    REQUIRE(layer->getBitmapImageAtFrame(1)->getOpacity() == 1.0);
    REQUIRE_FALSE(scene.undoAction->isEnabled());

    scene.redoAction->trigger();
    REQUIRE(layer->getBitmapImageAtFrame(1)->getOpacity() == 0.25);
}

TEST_CASE("Changing the camera field size is undoable")
{
    UndoRedoTestScene scene;
    LayerCamera* cameraLayer = scene.object->addNewCameraLayer();

    const QRect oldViewRect = cameraLayer->getViewRect();
    const QRect newViewRect(-320, -240, 640, 480);
    REQUIRE(oldViewRect != newViewRect);

    cameraLayer->setViewRect(newViewRect);
    scene.editor->undoRedo()->push(new CameraViewRectCommand(cameraLayer->id(), oldViewRect, newViewRect,
                                                             1, "Camera size change", scene.editor));

    scene.undoAction->trigger();
    REQUIRE(cameraLayer->getViewRect() == oldViewRect);

    scene.redoAction->trigger();
    REQUIRE(cameraLayer->getViewRect() == newViewRect);
}

TEST_CASE("Renaming a layer is undoable")
{
    UndoRedoTestScene scene;
    Layer* layer = scene.bitmapLayer();
    layer->setName("Original");

    scene.editor->layers()->renameLayer(layer, "Renamed");
    REQUIRE(layer->name() == "Renamed");

    scene.undoAction->trigger();
    REQUIRE(layer->name() == "Original");

    scene.redoAction->trigger();
    REQUIRE(layer->name() == "Renamed");
}

TEST_CASE("Moving a layer is undoable")
{
    UndoRedoTestScene scene;
    Layer* firstLayer = scene.bitmapLayer();
    Layer* secondLayer = scene.object->addNewBitmapLayer();
    scene.editor->layers()->setCurrentLayer(1);

    // Simulate a timeline drag from index 1 to index 0 and record it the
    // way TimeLineCells does.
    scene.editor->swapLayers(0, 1);
    REQUIRE(scene.editor->layers()->getLayer(0) == secondLayer);
    scene.editor->undoRedo()->push(new LayerMoveCommand(1, 0, "Move layer", scene.editor));

    scene.undoAction->trigger();
    REQUIRE(scene.editor->layers()->getLayer(0) == firstLayer);
    REQUIRE(scene.editor->layers()->getLayer(1) == secondLayer);

    scene.redoAction->trigger();
    REQUIRE(scene.editor->layers()->getLayer(0) == secondLayer);
    REQUIRE(scene.editor->layers()->getLayer(1) == firstLayer);
}

TEST_CASE("Creating a layer is undoable")
{
    UndoRedoTestScene scene;
    const int countBefore = scene.editor->layers()->count();

    Layer* newLayer = scene.editor->layers()->createBitmapLayer("Extra");
    const int newLayerId = newLayer->id();
    REQUIRE(scene.editor->layers()->count() == countBefore + 1);

    scene.undoAction->trigger();
    REQUIRE(scene.editor->layers()->count() == countBefore);
    REQUIRE(scene.editor->layers()->findLayerById(newLayerId) == nullptr);

    scene.redoAction->trigger();
    REQUIRE(scene.editor->layers()->count() == countBefore + 1);
    Layer* restored = scene.editor->layers()->findLayerById(newLayerId);
    REQUIRE(restored != nullptr);
    REQUIRE(restored->name() == "Extra");
    REQUIRE(scene.editor->layers()->getIndex(restored) == countBefore);
}

TEST_CASE("Deleting a layer is undoable and earlier history survives it")
{
    UndoRedoTestScene scene;

    // Create a second layer (recorded), draw on it (recorded), delete it
    // (recorded) — then unwind the whole stack and play it forward again.
    Layer* extraLayer = scene.editor->layers()->createBitmapLayer("Extra");
    const int extraLayerId = extraLayer->id();

    BitmapImage* image = static_cast<LayerBitmap*>(extraLayer)->getBitmapImageAtFrame(1);
    UndoTransaction transaction = scene.editor->undoRedo()->beginTransaction(UndoRedoRecordType::KEYFRAME_MODIFY);
    image->setPixel(3, 4, red);
    transaction.commit("stroke");

    REQUIRE(scene.editor->layers()->deleteLayer(1) == Status::OK);
    REQUIRE(scene.editor->layers()->findLayerById(extraLayerId) == nullptr);

    // Undo the deletion: the layer is back, same id, drawing intact.
    scene.undoAction->trigger();
    Layer* restored = scene.editor->layers()->findLayerById(extraLayerId);
    REQUIRE(restored != nullptr);
    REQUIRE(static_cast<LayerBitmap*>(restored)->getBitmapImageAtFrame(1)->constScanLine(3, 4) == red);

    // Undo the stroke on the restored layer — the #864/#1412 scenario:
    // history recorded before a layer deletion used to be lost for good.
    scene.undoAction->trigger();
    REQUIRE(static_cast<LayerBitmap*>(restored)->getBitmapImageAtFrame(1)->constScanLine(3, 4) == 0);

    // Undo the layer creation itself.
    scene.undoAction->trigger();
    REQUIRE(scene.editor->layers()->findLayerById(extraLayerId) == nullptr);
    REQUIRE(scene.editor->layers()->count() == 1);

    // And forward again: create, stroke, delete.
    scene.redoAction->trigger();
    restored = scene.editor->layers()->findLayerById(extraLayerId);
    REQUIRE(restored != nullptr);

    scene.redoAction->trigger();
    REQUIRE(static_cast<LayerBitmap*>(restored)->getBitmapImageAtFrame(1)->constScanLine(3, 4) == red);

    scene.redoAction->trigger();
    REQUIRE(scene.editor->layers()->findLayerById(extraLayerId) == nullptr);
}

TEST_CASE("Adding a keyframe through the editor is undoable")
{
    UndoRedoTestScene scene;
    Layer* layer = scene.bitmapLayer();

    REQUIRE_FALSE(layer->keyExists(5));
    scene.editor->addKeyFrame(0, 5);
    REQUIRE(layer->keyExists(5));

    scene.undoAction->trigger();
    REQUIRE_FALSE(layer->keyExists(5));

    scene.redoAction->trigger();
    REQUIRE(layer->keyExists(5));
}
