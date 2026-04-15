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
