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
#include "undoredomanager.h"
#include "layerbitmap.h"
#include "layersound.h"
#include "bitmapimage.h"
#include "soundclip.h"

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
    REQUIRE(restored->isValid()); // file name intact and media player recreated
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
