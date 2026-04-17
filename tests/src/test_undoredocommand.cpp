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
#include "layermanager.h"
#include "layerbitmap.h"
#include "layercamera.h"
#include "undoredocommand.h"

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
