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

#include "editor.h"
#include "object.h"
#include "layer.h"
#include "keyframe.h"
#include "layermanager.h"
#include "layerbitmap.h"
#include "layervector.h"
#include "layercamera.h"
#include "bitmapimage.h"
#include "vectorimage.h"
#include "beziercurve.h"
#include "selectionmanager.h"
#include "undoredocommand.h"

#include <QUndoStack>

static bool areCameraStatesEqual(const Camera& lhs, const Camera& rhs)
{
    return lhs.compare(rhs);
}

TEST_CASE("PasteFramesCommand round-trip preserves displaced contiguous frames", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    // Layer with contiguous frames that will be displaced by paste collisions.
    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);
    editor->scrubTo(5);

    REQUIRE(layer->addNewKeyFrameAt(10));
    REQUIRE(layer->addNewKeyFrameAt(11));

    REQUIRE(layer->keyExists(10));
    REQUIRE(layer->keyExists(11));
    REQUIRE(!layer->keyExists(12));
    REQUIRE(!layer->keyExists(13));

    QList<QPair<int, KeyFrame*>> beforeFrames;
    layer->foreachKeyFrame([&beforeFrames](KeyFrame* frame) {
        beforeFrames.append(qMakePair(frame->pos(), frame->clone()));
    });

    const QList<int> pastePositions = {10, 11};
    for (int pastePos : pastePositions)
    {
        if (layer->getKeyFrameWhichCovers(pastePos) != nullptr)
        {
            layer->newSelectionOfConnectedFrames(pastePos);
            layer->moveSelectedFrames(1);
        }

        KeyFrame* source = layer->getKeyFrameAt(1);
        REQUIRE(source != nullptr);
        REQUIRE(layer->addKeyFrame(pastePos, source->clone()));

    }

    QList<QPair<int, KeyFrame*>> afterFrames;
    layer->foreachKeyFrame([&afterFrames](KeyFrame* frame) {
        afterFrames.append(qMakePair(frame->pos(), frame->clone()));
    });

    REQUIRE(layer->keyExists(10));
    REQUIRE(layer->keyExists(11));
    REQUIRE(layer->keyExists(12));
    REQUIRE(layer->keyExists(13));

    PasteFramesCommand command(beforeFrames,
                               afterFrames,
                               layer->id(),
                               "Paste",
                               editor);

    // Simulate QUndoStack push-time redo invocation.
    command.redo();

    command.undo();
    REQUIRE(layer->keyExists(10));
    REQUIRE(layer->keyExists(11));
    REQUIRE(!layer->keyExists(12));
    REQUIRE(!layer->keyExists(13));

    command.redo();
    REQUIRE(layer->keyExists(10));
    REQUIRE(layer->keyExists(11));
    REQUIRE(layer->keyExists(12));
    REQUIRE(layer->keyExists(13));

    delete editor;
}

TEST_CASE("PasteFramesCommand cascading collisions preserve bitmap frame content", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    LayerBitmap* layer = static_cast<LayerBitmap*>(object->addNewBitmapLayer());
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);
    editor->scrubTo(5);
    editor->scrubTo(5);

    REQUIRE(layer->addNewKeyFrameAt(10));
    REQUIRE(layer->addNewKeyFrameAt(11));
    REQUIRE(layer->addNewKeyFrameAt(12));

    BitmapImage* image10 = layer->getBitmapImageAtFrame(10);
    BitmapImage* image11 = layer->getBitmapImageAtFrame(11);
    BitmapImage* image12 = layer->getBitmapImageAtFrame(12);
    REQUIRE(image10 != nullptr);
    REQUIRE(image11 != nullptr);
    REQUIRE(image12 != nullptr);

    image10->drawLine(QPointF(0, 0), QPointF(10, 0), QPen(Qt::black, 1), QPainter::CompositionMode_SourceOver, true);
    image11->drawLine(QPointF(0, 1), QPointF(10, 1), QPen(Qt::black, 1), QPainter::CompositionMode_SourceOver, true);
    image12->drawLine(QPointF(0, 2), QPointF(10, 2), QPen(Qt::black, 1), QPainter::CompositionMode_SourceOver, true);

    const QRect bounds10 = image10->bounds();
    const QRect bounds11 = image11->bounds();
    const QRect bounds12 = image12->bounds();

    QList<QPair<int, KeyFrame*>> beforeFrames;
    layer->foreachKeyFrame([&beforeFrames](KeyFrame* frame) {
        beforeFrames.append(qMakePair(frame->pos(), frame->clone()));
    });

    const QList<int> pastePositions = {10, 11, 12};
    for (int pastePos : pastePositions)
    {
        if (layer->getKeyFrameWhichCovers(pastePos) != nullptr)
        {
            layer->newSelectionOfConnectedFrames(pastePos);
            layer->moveSelectedFrames(1);
        }

        KeyFrame* source = layer->getKeyFrameAt(1);
        REQUIRE(source != nullptr);
        REQUIRE(layer->addKeyFrame(pastePos, source->clone()));
    }

    QList<QPair<int, KeyFrame*>> afterFrames;
    layer->foreachKeyFrame([&afterFrames](KeyFrame* frame) {
        afterFrames.append(qMakePair(frame->pos(), frame->clone()));
    });

    PasteFramesCommand command(beforeFrames,
                               afterFrames,
                               layer->id(),
                               "Paste",
                               editor);

    command.redo();

    command.undo();
    REQUIRE(layer->keyExists(10));
    REQUIRE(layer->keyExists(11));
    REQUIRE(layer->keyExists(12));
    REQUIRE(!layer->keyExists(13));
    REQUIRE(!layer->keyExists(14));
    REQUIRE(!layer->keyExists(15));
    REQUIRE(layer->getBitmapImageAtFrame(10)->bounds() == bounds10);
    REQUIRE(layer->getBitmapImageAtFrame(11)->bounds() == bounds11);
    REQUIRE(layer->getBitmapImageAtFrame(12)->bounds() == bounds12);

    command.redo();
    REQUIRE(layer->keyExists(10));
    REQUIRE(layer->keyExists(11));
    REQUIRE(layer->keyExists(12));
    REQUIRE(layer->keyExists(13));
    REQUIRE(layer->keyExists(14));
    REQUIRE(layer->keyExists(15));
    REQUIRE(layer->getBitmapImageAtFrame(13)->bounds() == bounds10);
    REQUIRE(layer->getBitmapImageAtFrame(14)->bounds() == bounds11);
    REQUIRE(layer->getBitmapImageAtFrame(15)->bounds() == bounds12);

    delete editor;
}

TEST_CASE("PasteFramesCommand redo reapplies snapshot after intermediate manual edits", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    REQUIRE(layer->addNewKeyFrameAt(10));
    REQUIRE(layer->addNewKeyFrameAt(11));

    QList<QPair<int, KeyFrame*>> beforeFrames;
    layer->foreachKeyFrame([&beforeFrames](KeyFrame* frame) {
        beforeFrames.append(qMakePair(frame->pos(), frame->clone()));
    });

    const QList<int> pastePositions = {10, 11};
    for (int pastePos : pastePositions)
    {
        if (layer->getKeyFrameWhichCovers(pastePos) != nullptr)
        {
            layer->newSelectionOfConnectedFrames(pastePos);
            layer->moveSelectedFrames(1);
        }

        KeyFrame* source = layer->getKeyFrameAt(1);
        REQUIRE(source != nullptr);
        REQUIRE(layer->addKeyFrame(pastePos, source->clone()));
    }

    QList<QPair<int, KeyFrame*>> afterFrames;
    layer->foreachKeyFrame([&afterFrames](KeyFrame* frame) {
        afterFrames.append(qMakePair(frame->pos(), frame->clone()));
    });

    PasteFramesCommand command(beforeFrames,
                               afterFrames,
                               layer->id(),
                               "Paste",
                               editor);

    command.redo();
    command.undo();

    // Introduce a non-command manual mutation between undo and redo.
    REQUIRE(layer->addNewKeyFrameAt(30));
    REQUIRE(layer->keyExists(30));

    command.redo();

    // Redo should fully restore command's after-snapshot state.
    REQUIRE(layer->keyExists(10));
    REQUIRE(layer->keyExists(11));
    REQUIRE(layer->keyExists(12));
    REQUIRE(layer->keyExists(13));
    REQUIRE(!layer->keyExists(30));

    delete editor;
}

TEST_CASE("RemoveKeyFramesCommand restores and re-removes selected frames", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    REQUIRE(layer->addNewKeyFrameAt(5));
    REQUIRE(layer->addNewKeyFrameAt(10));

    const QList<int> positions = {5, 10};

    RemoveKeyFramesCommand command(positions,
                                   layer->id(),
                                   "Remove Selected Frames",
                                   editor);

    // Simulate caller mutation before command is pushed.
    REQUIRE(layer->removeKeyFrame(5));
    REQUIRE(layer->removeKeyFrame(10));

    // Simulate QUndoStack push-time redo invocation.
    command.redo();

    REQUIRE(!layer->keyExists(5));
    REQUIRE(!layer->keyExists(10));

    command.undo();
    REQUIRE(layer->keyExists(5));
    REQUIRE(layer->keyExists(10));

    command.redo();
    REQUIRE(!layer->keyExists(5));
    REQUIRE(!layer->keyExists(10));

    delete editor;
}

TEST_CASE("DeleteLayerCommand restores deleted layer and id on undo", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* camera = object->addNewCameraLayer();
    Layer* bitmap = object->addNewBitmapLayer();
    Q_UNUSED(camera)

    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(1);

    const int deletedLayerIndex = 1;
    const int deletedLayerId = bitmap->id();

    DeleteLayerCommand command(deletedLayerIndex,
                               deletedLayerId,
                               "Delete Layer",
                               editor);

    // Constructor performs initial deletion.
    REQUIRE(object->getLayerCount() == 1);
    REQUIRE(object->findLayerById(deletedLayerId) == nullptr);

    // Simulate QUndoStack push-time redo invocation.
    command.redo();

    command.undo();
    REQUIRE(object->getLayerCount() == 2);
    REQUIRE(object->getLayer(deletedLayerIndex) != nullptr);
    REQUIRE(object->getLayer(deletedLayerIndex)->id() == deletedLayerId);

    command.redo();
    REQUIRE(object->getLayerCount() == 1);
    REQUIRE(object->findLayerById(deletedLayerId) == nullptr);

    delete editor;
}

TEST_CASE("MoveFrameCommand moves frame forward and round-trips correctly", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    REQUIRE(layer->addNewKeyFrameAt(5));
    KeyFrame* frame = layer->getKeyFrameAt(5);
    REQUIRE(frame != nullptr);
    REQUIRE(!layer->keyExists(6));

    // Constructor performs the move: frame should now be at 6
    MoveFrameCommand command(5, 1, layer->id(), "Move Frame Forward", editor);
    REQUIRE(!layer->keyExists(5));
    REQUIRE(layer->keyExists(6));
    REQUIRE(layer->getKeyFrameAt(6) == frame);

    // Simulate QUndoStack push-time redo invocation (no-op on first call)
    command.redo();
    REQUIRE(!layer->keyExists(5));
    REQUIRE(layer->keyExists(6));

    command.undo();
    REQUIRE(layer->keyExists(5));
    REQUIRE(!layer->keyExists(6));
    REQUIRE(layer->getKeyFrameAt(5) == frame);

    command.redo();
    REQUIRE(!layer->keyExists(5));
    REQUIRE(layer->keyExists(6));
    REQUIRE(layer->getKeyFrameAt(6) == frame);

    delete editor;
}

TEST_CASE("MoveFrameCommand backward move is a no-op at frame 1", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    // Frame 1 is created by default; trying to move it to 0 should be a no-op
    REQUIRE(layer->keyExists(1));

    MoveFrameCommand command(1, -1, layer->id(), "Move Frame Backward", editor);
    REQUIRE(layer->keyExists(1)); // still at 1 — move was rejected

    // undo/redo of a no-op command should be silent
    command.redo();
    REQUIRE(layer->keyExists(1));

    command.undo();
    REQUIRE(layer->keyExists(1));

    delete editor;
}

TEST_CASE("ReverseFrameOrderCommand reverses frame content and round-trips", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    REQUIRE(layer->addNewKeyFrameAt(2));
    REQUIRE(layer->addNewKeyFrameAt(5));
    REQUIRE(layer->addNewKeyFrameAt(8));

    // Capture frame pointers before reversal
    KeyFrame* frame2 = layer->getKeyFrameAt(2);
    KeyFrame* frame5 = layer->getKeyFrameAt(5);
    KeyFrame* frame8 = layer->getKeyFrameAt(8);
    REQUIRE(frame2 != nullptr);
    REQUIRE(frame5 != nullptr);
    REQUIRE(frame8 != nullptr);

    // Constructor performs the reversal: {2,5,8} → 2↔8 swapped, 5 unchanged
    ReverseFrameOrderCommand command({2, 5, 8}, layer->id(), "Reverse Frames", editor);
    REQUIRE(layer->getKeyFrameAt(2) == frame8);
    REQUIRE(layer->getKeyFrameAt(5) == frame5);
    REQUIRE(layer->getKeyFrameAt(8) == frame2);

    // Simulate QUndoStack push-time redo invocation (no-op on first call)
    command.redo();
    REQUIRE(layer->getKeyFrameAt(2) == frame8);

    // Undo restores original order
    command.undo();
    REQUIRE(layer->getKeyFrameAt(2) == frame2);
    REQUIRE(layer->getKeyFrameAt(5) == frame5);
    REQUIRE(layer->getKeyFrameAt(8) == frame8);

    // Redo re-applies the reversal
    command.redo();
    REQUIRE(layer->getKeyFrameAt(2) == frame8);
    REQUIRE(layer->getKeyFrameAt(5) == frame5);
    REQUIRE(layer->getKeyFrameAt(8) == frame2);

    delete editor;
}

TEST_CASE("KeyFrameRemoveCommand removes and restores keyframe", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    REQUIRE(layer->addNewKeyFrameAt(5));
    editor->scrubTo(5);
    KeyFrame* originalFrame = layer->getKeyFrameAt(5);
    REQUIRE(originalFrame != nullptr);

    // Constructor clones the keyframe before removal
    KeyFrameRemoveCommand command(originalFrame, layer->id(), "Remove Frame", editor);

    // Simulate caller mutation before command is pushed.
    REQUIRE(layer->removeKeyFrame(5));

    // The frame should already be removed by caller-side mutation.
    REQUIRE(!layer->keyExists(5));

    // Simulate QUndoStack push-time redo invocation (no-op)
    command.redo();
    REQUIRE(!layer->keyExists(5));

    // Undo restores the frame
    command.undo();
    REQUIRE(layer->keyExists(5));
    KeyFrame* restoredFrame = layer->getKeyFrameAt(5);
    REQUIRE(restoredFrame != nullptr);

    // Redo removes it again
    command.redo();
    REQUIRE(!layer->keyExists(5));

    delete editor;
}

TEST_CASE("KeyFrameAddCommand adds and removes keyframe", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    REQUIRE(!layer->keyExists(10));
    editor->scrubTo(10);

    // Constructor captures state only; caller performs initial addition.
    KeyFrameAddCommand command(10, layer->id(), "Add Frame", editor);

    // Simulate caller mutation before command is pushed.
    REQUIRE(layer->addNewKeyFrameAt(10));

    // The frame should already be present by caller-side mutation.
    REQUIRE(layer->keyExists(10));

    // Simulate QUndoStack push-time redo invocation (no-op)
    command.redo();
    REQUIRE(layer->keyExists(10));

    // Undo removes the frame
    command.undo();
    REQUIRE(!layer->keyExists(10));

    // Redo adds it back
    command.redo();
    REQUIRE(layer->keyExists(10));

    delete editor;
}

TEST_CASE("MoveKeyFramesCommand moves multiple frames forward", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    REQUIRE(layer->addNewKeyFrameAt(5));
    REQUIRE(layer->addNewKeyFrameAt(8));
    REQUIRE(layer->addNewKeyFrameAt(12));

    KeyFrame* frame5 = layer->getKeyFrameAt(5);
    KeyFrame* frame8 = layer->getKeyFrameAt(8);
    KeyFrame* frame12 = layer->getKeyFrameAt(12);

    // Move frames at positions 5, 8, 12 forward by 3
    QList<int> positions = {5, 8, 12};
    MoveKeyFramesCommand command(3, positions, layer->id(), "Move Frames", editor);

    // Simulate caller mutation before command is pushed.
    layer->setFrameSelected(5, true);
    layer->setFrameSelected(8, true);
    layer->setFrameSelected(12, true);
    REQUIRE(layer->moveSelectedFrames(3));

    // Frames should already be moved by caller-side mutation.
    REQUIRE(!layer->keyExists(5));
    REQUIRE(!layer->keyExists(12));
    REQUIRE(layer->getKeyFrameAt(8) == frame5);
    REQUIRE(layer->getKeyFrameAt(11) == frame8);
    REQUIRE(layer->getKeyFrameAt(15) == frame12);

    // Simulate QUndoStack push-time redo invocation (no-op)
    command.redo();

    // Undo restores original positions
    command.undo();
    REQUIRE(layer->getKeyFrameAt(5) == frame5);
    REQUIRE(layer->getKeyFrameAt(8) == frame8);
    REQUIRE(layer->getKeyFrameAt(12) == frame12);
    REQUIRE(!layer->keyExists(11));
    REQUIRE(!layer->keyExists(15));

    // Redo moves them forward again
    command.redo();
    REQUIRE(layer->getKeyFrameAt(8) == frame5);
    REQUIRE(layer->getKeyFrameAt(11) == frame8);
    REQUIRE(layer->getKeyFrameAt(15) == frame12);

    delete editor;
}

TEST_CASE("MoveKeyFramesCommand moves frames backward", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    REQUIRE(layer->addNewKeyFrameAt(10));
    REQUIRE(layer->addNewKeyFrameAt(15));

    KeyFrame* frame10 = layer->getKeyFrameAt(10);
    KeyFrame* frame15 = layer->getKeyFrameAt(15);

    // Move frames backward by 5
    QList<int> positions = {10, 15};
    MoveKeyFramesCommand command(-5, positions, layer->id(), "Move Frames Back", editor);

    // Simulate caller mutation before command is pushed.
    layer->setFrameSelected(10, true);
    layer->setFrameSelected(15, true);
    REQUIRE(layer->moveSelectedFrames(-5));

    REQUIRE(layer->getKeyFrameAt(5) == frame10);
    REQUIRE(layer->getKeyFrameAt(10) == frame15);

    command.undo();
    REQUIRE(layer->getKeyFrameAt(10) == frame10);
    REQUIRE(layer->getKeyFrameAt(15) == frame15);

    delete editor;
}

TEST_CASE("SetExposureCommand extends and contracts frame exposure", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    // Create frames at positions 5, 10, 15
    REQUIRE(layer->addNewKeyFrameAt(5));
    REQUIRE(layer->addNewKeyFrameAt(10));
    REQUIRE(layer->addNewKeyFrameAt(15));

    KeyFrame* frame5 = layer->getKeyFrameAt(5);
    KeyFrame* frame10 = layer->getKeyFrameAt(10);
    KeyFrame* frame15 = layer->getKeyFrameAt(15);

    // Select frame at position 5 and extend exposure by 3
    QList<int> selectedByPos = {5};
    QList<int> selectedByLast = {5};
    SetExposureCommand command(3, layer->id(), selectedByPos, selectedByLast, true, 5, "Set Exposure", editor);

    // Constructor performs the exposure change: frame at 10 should move to 13
    REQUIRE(layer->getKeyFrameAt(5) == frame5);
    REQUIRE(!layer->keyExists(10));
    REQUIRE(layer->getKeyFrameAt(13) == frame10);
    REQUIRE(layer->getKeyFrameAt(18) == frame15);

    // Simulate QUndoStack push-time redo invocation (no-op)
    command.redo();

    // Undo restores original positions
    command.undo();
    REQUIRE(layer->getKeyFrameAt(5) == frame5);
    REQUIRE(layer->getKeyFrameAt(10) == frame10);
    REQUIRE(layer->getKeyFrameAt(15) == frame15);

    // Redo applies the exposure change again
    command.redo();
    REQUIRE(layer->getKeyFrameAt(5) == frame5);
    REQUIRE(layer->getKeyFrameAt(13) == frame10);
    REQUIRE(layer->getKeyFrameAt(18) == frame15);

    delete editor;
}

TEST_CASE("SetExposureCommand contracts exposure (negative offset)", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    REQUIRE(layer->addNewKeyFrameAt(5));
    REQUIRE(layer->addNewKeyFrameAt(15));

    KeyFrame* frame5 = layer->getKeyFrameAt(5);
    KeyFrame* frame15 = layer->getKeyFrameAt(15);

    // Contract exposure by 5: frame at 15 should move to 10
    QList<int> selectedByPos = {5};
    QList<int> selectedByLast = {5};
    SetExposureCommand command(-5, layer->id(), selectedByPos, selectedByLast, true, 5, "Contract Exposure", editor);

    REQUIRE(layer->getKeyFrameAt(5) == frame5);
    REQUIRE(layer->getKeyFrameAt(10) == frame15);
    REQUIRE(!layer->keyExists(15));

    command.undo();
    REQUIRE(layer->getKeyFrameAt(5) == frame5);
    REQUIRE(layer->getKeyFrameAt(15) == frame15);
    REQUIRE(!layer->keyExists(10));

    delete editor;
}

TEST_CASE("InsertExposureCommand inserts blank exposure and shifts frames", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    REQUIRE(layer->addNewKeyFrameAt(5));
    REQUIRE(layer->addNewKeyFrameAt(10));

    KeyFrame* frame5 = layer->getKeyFrameAt(5);
    KeyFrame* frame10 = layer->getKeyFrameAt(10);

    // Insert exposure at position 5 where a keyframe exists.
    InsertExposureCommand command(5, layer->id(), "Insert Exposure", editor);

    // Constructor performs insertion by creating a new key at 6.
    // Frame at 10 is unchanged because there is no keyframe at 6 to shift.
    REQUIRE(layer->getKeyFrameAt(5) == frame5);
    REQUIRE(layer->keyExists(6));
    REQUIRE(layer->getKeyFrameAt(10) == frame10);

    // Simulate QUndoStack push-time redo invocation (no-op)
    command.redo();

    // Undo removes the inserted frame and restores positions
    command.undo();
    REQUIRE(layer->getKeyFrameAt(5) == frame5);
    REQUIRE(!layer->keyExists(6));
    REQUIRE(layer->getKeyFrameAt(10) == frame10);

    // Redo inserts again
    command.redo();
    REQUIRE(layer->keyExists(6));
    REQUIRE(layer->getKeyFrameAt(10) == frame10);

    delete editor;
}

TEST_CASE("BitmapReplaceCommand replaces and restores bitmap image", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    LayerBitmap* layer = static_cast<LayerBitmap*>(object->addNewBitmapLayer());
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    REQUIRE(layer->addNewKeyFrameAt(5));
    BitmapImage* image = layer->getBitmapImageAtFrame(5);
    REQUIRE(image != nullptr);

    // Draw something on the original image
    image->drawLine(QPointF(0, 0), QPointF(100, 100), QPen(Qt::black, 5), QPainter::CompositionMode_SourceOver, true);
    QRect originalBounds = image->bounds();
    REQUIRE(!originalBounds.isEmpty());

    // Create backup and modify the image
    BitmapImage backup = *image;
    image->clear();
    REQUIRE(image->bounds().isEmpty());

    QUndoStack stack;
    stack.push(new BitmapReplaceCommand(&backup, layer->id(), "Draw", editor));
    REQUIRE(stack.canUndo());
    REQUIRE(!stack.canRedo());

    // Push-time redo is a no-op for this command, so image should remain cleared.
    REQUIRE(layer->keyExists(5));

    // Undo restores the original drawn content
    stack.undo();
    image = layer->getBitmapImageAtFrame(5);
    REQUIRE(image != nullptr);
    REQUIRE(image->bounds() == originalBounds);

    // Redo applies the cleared state
    stack.redo();
    image = layer->getBitmapImageAtFrame(5);
    REQUIRE(image != nullptr);
    REQUIRE(image->bounds().isEmpty());

    delete editor;
}

TEST_CASE("VectorReplaceCommand replaces and restores vector image", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    LayerVector* layer = static_cast<LayerVector*>(object->addNewVectorLayer());
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);
    editor->scrubTo(5);

    REQUIRE(layer->addNewKeyFrameAt(5));
    VectorImage* image = layer->getVectorImageAtFrame(5);
    REQUIRE(image != nullptr);

    // Add a curve to the original image
    BezierCurve curve({QPointF(0, 0), QPointF(50, 50), QPointF(100, 0)});
    image->addCurve(curve, 1.0);
    REQUIRE(!image->isEmpty());

    // Create backup and clear the image
    VectorImage backup = *image;
    image->clear();
    REQUIRE(image->isEmpty());

    QUndoStack stack;
    stack.push(new VectorReplaceCommand(&backup, layer->id(), "Draw", editor));

    // Push-time redo is a no-op for this command, so image should remain cleared.
    image = layer->getVectorImageAtFrame(5);
    REQUIRE(image != nullptr);
    REQUIRE(image->isEmpty());

    // Undo restores the original drawn content
    stack.undo();
    image = layer->getVectorImageAtFrame(5);
    REQUIRE(image != nullptr);
    REQUIRE(!image->isEmpty());

    // Redo applies the cleared state
    stack.redo();
    image = layer->getVectorImageAtFrame(5);
    REQUIRE(image != nullptr);
    REQUIRE(image->isEmpty());

    delete editor;
}

TEST_CASE("TransformCommand applies and reverses selection transformation", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);
    editor->scrubTo(1);

    // Create a selection
    QRectF originalRect(0, 0, 100, 100);
    QPointF originalTranslation(0, 0);
    qreal originalRotation = 0.0;
    qreal originalScaleX = 1.0;
    qreal originalScaleY = 1.0;
    QPointF originalAnchor(50, 50);

    editor->select()->setSelection(originalRect, true);

    // Apply transformation: translate, rotate, scale
    QPointF newTranslation(50, 30);
    qreal newRotation = 45.0;
    qreal newScaleX = 1.5;
    qreal newScaleY = 1.5;
    QPointF newAnchor(50, 50);

    editor->select()->setTranslation(newTranslation);
    editor->select()->setRotation(newRotation);
    editor->select()->setScale(newScaleX, newScaleY);

    // Constructor captures the current transformed state
    TransformCommand command(
        originalRect,
        originalTranslation,
        originalRotation,
        originalScaleX,
        originalScaleY,
        originalAnchor,
        false,
        "Transform",
        editor
    );

    // Simulate QUndoStack push-time redo invocation (no-op)
    command.redo();

    // Verify transformed state
    REQUIRE(editor->select()->myTranslation() == newTranslation);
    REQUIRE(editor->select()->myRotation() == newRotation);

    // Undo restores original state
    command.undo();
    REQUIRE(editor->select()->myTranslation() == originalTranslation);
    REQUIRE(editor->select()->myRotation() == originalRotation);

    // Redo applies transformation again
    command.redo();
    REQUIRE(editor->select()->myTranslation() == newTranslation);
    REQUIRE(editor->select()->myRotation() == newRotation);

    delete editor;
}

TEST_CASE("SetExposureCommand add-exposure round-trip restores frame positions", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    // Layer already has frame at 1 from addNewBitmapLayer. Add frames at 3, 5, 7.
    // Select frames 3 and 5 (not the trailing frame 7, which should be pushed out).
    REQUIRE(layer->addNewKeyFrameAt(3));
    REQUIRE(layer->addNewKeyFrameAt(5));
    REQUIRE(layer->addNewKeyFrameAt(7));

    layer->setFrameSelected(3, true);
    layer->setFrameSelected(5, true);

    const QList<int> selectedByPos  = layer->selectedKeyFramesPositions();
    const QList<int> selectedByLast = layer->selectedKeyFramesByLast();

    // Constructor performs the mutation (+1 exposure).
    // Expected result: frame at 3 unchanged (first selected), 5→6, 7 pushed to 9.
    SetExposureCommand command(1,
                               layer->id(),
                               selectedByPos,
                               selectedByLast,
                               /*hadSelectedFrames=*/true,
                               /*currentFramePos=*/0,
                               "Add Exposure",
                               editor);

    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(3));
    REQUIRE(!layer->keyExists(5));
    REQUIRE(layer->keyExists(6));
    REQUIRE(!layer->keyExists(7));
    REQUIRE(!layer->keyExists(8));
    REQUIRE(layer->keyExists(9));

    // Simulate QUndoStack push-time redo invocation (no-op for first redo).
    command.redo();

    command.undo();
    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(3));
    REQUIRE(layer->keyExists(5));
    REQUIRE(layer->keyExists(7));
    REQUIRE(!layer->keyExists(6));
    REQUIRE(!layer->keyExists(9));

    command.redo();
    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(3));
    REQUIRE(!layer->keyExists(5));
    REQUIRE(layer->keyExists(6));
    REQUIRE(!layer->keyExists(7));
    REQUIRE(layer->keyExists(9));

    delete editor;
}

TEST_CASE("SetExposureCommand subtract-exposure round-trip restores frame positions", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    // Layer already has frame at 1 from addNewBitmapLayer. Add frames at 6 and 9.
    // Select both: subtract -1 keeps the first selected (6) in place and moves 9 to 8.
    REQUIRE(layer->addNewKeyFrameAt(6));
    REQUIRE(layer->addNewKeyFrameAt(9));

    layer->setFrameSelected(6, true);
    layer->setFrameSelected(9, true);

    const QList<int> selectedByPos  = layer->selectedKeyFramesPositions();
    const QList<int> selectedByLast = layer->selectedKeyFramesByLast();

    // Constructor performs the mutation (-1 exposure).
    // Expected result: frame at 6 unchanged (first selected), 9→8.
    SetExposureCommand command(-1,
                               layer->id(),
                               selectedByPos,
                               selectedByLast,
                               /*hadSelectedFrames=*/true,
                               /*currentFramePos=*/0,
                               "Subtract Exposure",
                               editor);

    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(6));
    REQUIRE(!layer->keyExists(9));
    REQUIRE(layer->keyExists(8));

    // Simulate QUndoStack push-time redo invocation (no-op for first redo).
    command.redo();

    command.undo();
    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(6));
    REQUIRE(layer->keyExists(9));
    REQUIRE(!layer->keyExists(8));

    command.redo();
    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(6));
    REQUIRE(!layer->keyExists(9));
    REQUIRE(layer->keyExists(8));

    delete editor;
}

TEST_CASE("SetExposureCommand single unselected frame uses currentFramePos", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    // Layer already has frame at 1 from addNewBitmapLayer. Add frames at 4 and 6.
    // No frames selected; expose frame 4 using the currentFramePos path.
    // Expected: frame at 4 unchanged (first selected), 6 pushed to 7.
    REQUIRE(layer->addNewKeyFrameAt(4));
    REQUIRE(layer->addNewKeyFrameAt(6));

    // Constructor performs the mutation via the currentFramePos path (hadSelectedFrames=false).
    SetExposureCommand command(1,
                               layer->id(),
                               /*selectedByPos=*/{},
                               /*selectedByLast=*/{},
                               /*hadSelectedFrames=*/false,
                               /*currentFramePos=*/4,
                               "Add Exposure (no selection)",
                               editor);

    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(4));
    REQUIRE(!layer->keyExists(6));
    REQUIRE(layer->keyExists(7));
    // Selection should have been cleaned up by the command.
    REQUIRE(!layer->hasAnySelectedFrames());

    // Simulate QUndoStack push-time redo invocation (no-op for first redo).
    command.redo();

    command.undo();
    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(4));
    REQUIRE(layer->keyExists(6));
    REQUIRE(!layer->keyExists(7));

    command.redo();
    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(4));
    REQUIRE(!layer->keyExists(6));
    REQUIRE(layer->keyExists(7));

    delete editor;
}

TEST_CASE("SetExposureCommand large positive offset round-trips frame positions", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    REQUIRE(layer->addNewKeyFrameAt(5));
    REQUIRE(layer->addNewKeyFrameAt(10));
    REQUIRE(layer->addNewKeyFrameAt(50));

    KeyFrame* frame5 = layer->getKeyFrameAt(5);
    KeyFrame* frame10 = layer->getKeyFrameAt(10);
    KeyFrame* frame50 = layer->getKeyFrameAt(50);
    REQUIRE(frame5 != nullptr);
    REQUIRE(frame10 != nullptr);
    REQUIRE(frame50 != nullptr);

    layer->setFrameSelected(5, true);

    const QList<int> selectedByPos = layer->selectedKeyFramesPositions();
    const QList<int> selectedByLast = layer->selectedKeyFramesByLast();

    SetExposureCommand command(20,
                               layer->id(),
                               selectedByPos,
                               selectedByLast,
                               /*hadSelectedFrames=*/true,
                               /*currentFramePos=*/0,
                               "Set Exposure Large Offset",
                               editor);

    REQUIRE(layer->getKeyFrameAt(5) == frame5);
    REQUIRE(layer->getKeyFrameAt(30) == frame10);
    REQUIRE(layer->getKeyFrameAt(70) == frame50);
    REQUIRE(!layer->keyExists(10));
    REQUIRE(!layer->keyExists(50));

    command.redo();
    command.undo();

    REQUIRE(layer->getKeyFrameAt(5) == frame5);
    REQUIRE(layer->getKeyFrameAt(10) == frame10);
    REQUIRE(layer->getKeyFrameAt(50) == frame50);
    REQUIRE(!layer->keyExists(30));
    REQUIRE(!layer->keyExists(70));

    command.redo();
    REQUIRE(layer->getKeyFrameAt(5) == frame5);
    REQUIRE(layer->getKeyFrameAt(30) == frame10);
    REQUIRE(layer->getKeyFrameAt(70) == frame50);

    delete editor;
}

TEST_CASE("InsertExposureCommand round-trip inserts and removes key", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    // Layer already has frame at 1 from addNewBitmapLayer. Add frames at 3, 4, 5.
    // Insert exposure at 3: shifts 4 and 5 right by 1 and creates a new key at 4.
    REQUIRE(layer->addNewKeyFrameAt(3));
    REQUIRE(layer->addNewKeyFrameAt(4));
    REQUIRE(layer->addNewKeyFrameAt(5));

    // Constructor performs insertExposureAt(3) + addNewKeyFrameAt(4).
    InsertExposureCommand command(3,
                                  layer->id(),
                                  "Insert Exposure",
                                  editor);

    // After construction: 3 stays, new key at 4, original 4→5 and 5→6.
    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(3));
    REQUIRE(layer->keyExists(4));
    REQUIRE(layer->keyExists(5));
    REQUIRE(layer->keyExists(6));

    // Simulate QUndoStack push-time redo invocation (no-op for first redo).
    command.redo();

    command.undo();
    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(3));
    REQUIRE(layer->keyExists(4));
    REQUIRE(layer->keyExists(5));
    REQUIRE(!layer->keyExists(6));

    command.redo();
    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(3));
    REQUIRE(layer->keyExists(4));
    REQUIRE(layer->keyExists(5));
    REQUIRE(layer->keyExists(6));

    delete editor;
}

TEST_CASE("InsertExposureCommand round-trip with no frames after insert position", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    Layer* layer = object->addNewBitmapLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);

    // Layer already has frame at 1 from addNewBitmapLayer. Add a frame at 5.
    // Insert exposure at 5: no other frames to shift; just creates new key at 6.
    REQUIRE(layer->addNewKeyFrameAt(5));

    InsertExposureCommand command(5,
                                  layer->id(),
                                  "Insert Exposure",
                                  editor);

    // After construction: key at 5 and new key at 6.
    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(5));
    REQUIRE(layer->keyExists(6));

    // Simulate QUndoStack push-time redo invocation (no-op for first redo).
    command.redo();

    command.undo();
    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(5));
    REQUIRE(!layer->keyExists(6));

    command.redo();
    REQUIRE(layer->keyExists(1));
    REQUIRE(layer->keyExists(5));
    REQUIRE(layer->keyExists(6));

    delete editor;
}

TEST_CASE("CameraTransformCommand round-trip restores camera state", "[undo-redo-new]")
{
    Editor* editor = new Editor;
    REQUIRE(editor->init());

    Object* object = new Object;
    object->init();

    LayerCamera* layer = object->addNewCameraLayer();
    REQUIRE(editor->setObject(object) == Status::OK);
    editor->layers()->setCurrentLayer(0);
    editor->scrubTo(1);

    Camera* camera = layer->getCameraAtFrame(1);
    REQUIRE(camera != nullptr);

    Camera before(*camera);

    camera->translate(QPointF(32.0, -8.0));
    camera->rotate(18.0);
    camera->scale(1.25);
    camera->updateViewTransform();

    Camera after(*camera);
    REQUIRE(!areCameraStatesEqual(before, after));

    QUndoStack stack;
    stack.push(new CameraTransformCommand(before,
                                          after,
                                          layer->id(),
                                          1,
                                          "Transform Camera",
                                          editor));

    // First redo is skipped because mutation already happened before push.
    Camera* current = layer->getCameraAtFrame(1);
    REQUIRE(current != nullptr);
    REQUIRE(areCameraStatesEqual(*current, after));

    stack.undo();
    current = layer->getCameraAtFrame(1);
    REQUIRE(current != nullptr);
    REQUIRE(areCameraStatesEqual(*current, before));

    stack.redo();
    current = layer->getCameraAtFrame(1);
    REQUIRE(current != nullptr);
    REQUIRE(areCameraStatesEqual(*current, after));

    delete editor;
}
