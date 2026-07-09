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

#include "object.h"
#include "editor.h"
#include "scribblearea.h"
#include "layermanager.h"
#include "undoredomanager.h"
#include "layerbitmap.h"
#include "bitmapimage.h"

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
