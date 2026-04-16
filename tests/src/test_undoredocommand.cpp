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
#include "layercamera.h"
#include "undoredocommand.h"

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

    REQUIRE(layer->addNewKeyFrameAt(10));
    REQUIRE(layer->addNewKeyFrameAt(11));

    REQUIRE(layer->keyExists(10));
    REQUIRE(layer->keyExists(11));
    REQUIRE(!layer->keyExists(12));
    REQUIRE(!layer->keyExists(13));

    QList<int> addedPositions;
    QList<int> collisionPositions;
    QList<QPair<int, KeyFrame*>> pastedClones;

    const QList<int> pastePositions = {10, 11};
    for (int pastePos : pastePositions)
    {
        if (layer->getKeyFrameWhichCovers(pastePos) != nullptr)
        {
            collisionPositions.append(pastePos);
            layer->newSelectionOfConnectedFrames(pastePos);
            layer->moveSelectedFrames(1);
        }

        KeyFrame* source = layer->getKeyFrameAt(1);
        REQUIRE(source != nullptr);
        REQUIRE(layer->addKeyFrame(pastePos, source->clone()));

        addedPositions.append(pastePos);
        pastedClones.append(qMakePair(pastePos, source->clone()));
    }

    REQUIRE(layer->keyExists(10));
    REQUIRE(layer->keyExists(11));
    REQUIRE(layer->keyExists(12));
    REQUIRE(layer->keyExists(13));

    PasteFramesCommand command(addedPositions,
                               collisionPositions,
                               pastedClones,
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
