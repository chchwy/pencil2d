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

#include <QIcon>

#include "editor.h"
#include "object.h"
#include "layermanager.h"
#include "preferencemanager.h"
#include "undoredomanager.h"

#include "layerbitmap.h"
#include "bitmapimage.h"
#include "preferencesdef.h"

// Creates an editor with the new undo/redo system enabled.
// UndoRedoManager::init() reads NEW_UNDO_REDO_SYSTEM_ON from PreferenceManager,
// so the preference must be set before re-calling init().
static Editor* makeEditorWithNewUndoSystem()
{
    Editor* editor = new Editor;
    editor->init();
    editor->preference()->set(SETTING::NEW_UNDO_REDO_SYSTEM_ON, true);
    editor->undoRedo()->init();
    return editor;
}

static Object* makeObjectWithBitmapLayer()
{
    Object* obj = new Object;
    obj->init();
    obj->addNewBitmapLayer(); // adds a keyframe at frame 1
    return obj;
}

// Records a single pixel modification on the current layer at frame 1.
// Returns the color that was written.
static QRgb recordPixelDraw(Editor* editor, QRgb color)
{
    const UndoSaveState* s = editor->undoRedo()->state(UndoRedoRecordType::KEYFRAME_MODIFY);
    LayerBitmap* layer = static_cast<LayerBitmap*>(editor->layers()->currentLayer());
    layer->getBitmapImageAtFrame(1)->setPixel(0, 0, color);
    editor->undoRedo()->record(s, "draw pixel");
    return color;
}

TEST_CASE("UndoRedoManager::init()")
{
    SECTION("init returns true")
    {
        Editor* editor = new Editor;
        editor->init();
        UndoRedoManager mgr(editor);
        REQUIRE(mgr.init() == true);
        delete editor;
    }

    SECTION("new undo system is disabled by default")
    {
        // With the default preference (NEW_UNDO_REDO_SYSTEM_ON = false),
        // state() must return nullptr.
        Editor* editor = new Editor;
        editor->init();
        editor->setObject(makeObjectWithBitmapLayer());

        REQUIRE(editor->undoRedo()->state(UndoRedoRecordType::KEYFRAME_MODIFY) == nullptr);
        delete editor;
    }

    SECTION("hasUnsavedChanges is false after init (new system)")
    {
        Editor* editor = makeEditorWithNewUndoSystem();
        editor->setObject(makeObjectWithBitmapLayer());

        REQUIRE(editor->undoRedo()->hasUnsavedChanges() == false);
        delete editor;
    }
}

TEST_CASE("UndoRedoManager::load()")
{
    SECTION("load returns Status::OK")
    {
        Editor* editor = makeEditorWithNewUndoSystem();
        Object* obj = makeObjectWithBitmapLayer();
        editor->setObject(obj);

        REQUIRE(editor->undoRedo()->load(obj) == Status::OK);
        delete editor;
    }

    SECTION("load clears the undo stack")
    {
        Editor* editor = makeEditorWithNewUndoSystem();
        Object* obj = makeObjectWithBitmapLayer();
        editor->setObject(obj);

        recordPixelDraw(editor, qRgba(255, 0, 0, 255));
        REQUIRE(editor->undoRedo()->hasUnsavedChanges() == true);

        editor->undoRedo()->load(obj);

        REQUIRE(editor->undoRedo()->hasUnsavedChanges() == false);
        delete editor;
    }
}

TEST_CASE("UndoRedoManager::save()")
{
    SECTION("save returns Status::OK")
    {
        Editor* editor = makeEditorWithNewUndoSystem();
        Object* obj = makeObjectWithBitmapLayer();
        editor->setObject(obj);

        REQUIRE(editor->undoRedo()->save(obj) == Status::OK);
        delete editor;
    }

    SECTION("save marks the stack as clean")
    {
        Editor* editor = makeEditorWithNewUndoSystem();
        Object* obj = makeObjectWithBitmapLayer();
        editor->setObject(obj);

        recordPixelDraw(editor, qRgba(255, 0, 0, 255));
        REQUIRE(editor->undoRedo()->hasUnsavedChanges() == true);

        editor->undoRedo()->save(obj);

        REQUIRE(editor->undoRedo()->hasUnsavedChanges() == false);
        delete editor;
    }
}

TEST_CASE("UndoRedoManager::clearStack()")
{
    SECTION("clearStack makes hasUnsavedChanges return false (new system)")
    {
        Editor* editor = makeEditorWithNewUndoSystem();
        Object* obj = makeObjectWithBitmapLayer();
        editor->setObject(obj);

        recordPixelDraw(editor, qRgba(0, 255, 0, 255));
        REQUIRE(editor->undoRedo()->hasUnsavedChanges() == true);

        editor->undoRedo()->clearStack();

        REQUIRE(editor->undoRedo()->hasUnsavedChanges() == false);
        delete editor;
    }
}

TEST_CASE("UndoRedoManager::state()")
{
    SECTION("returns nullptr when new system is disabled")
    {
        Editor* editor = new Editor;
        editor->init();
        editor->setObject(makeObjectWithBitmapLayer());

        const UndoSaveState* s = editor->undoRedo()->state(UndoRedoRecordType::KEYFRAME_MODIFY);
        REQUIRE(s == nullptr);
        delete editor;
    }

    SECTION("returns nullptr for INVALID record type")
    {
        Editor* editor = makeEditorWithNewUndoSystem();
        editor->setObject(makeObjectWithBitmapLayer());

        const UndoSaveState* s = editor->undoRedo()->state(UndoRedoRecordType::INVALID);
        REQUIRE(s == nullptr);
        delete editor;
    }

    SECTION("returns valid state on a bitmap layer")
    {
        Editor* editor = makeEditorWithNewUndoSystem();
        editor->setObject(makeObjectWithBitmapLayer());

        const UndoSaveState* s = editor->undoRedo()->state(UndoRedoRecordType::KEYFRAME_MODIFY);
        REQUIRE(s != nullptr);
        REQUIRE(s->layerType == Layer::BITMAP);
        REQUIRE(s->recordType == UndoRedoRecordType::KEYFRAME_MODIFY);
        REQUIRE(s->selectionState != nullptr);
        delete s;
        delete editor;
    }

    SECTION("captures keyframe at the current frame")
    {
        Editor* editor = makeEditorWithNewUndoSystem();
        editor->setObject(makeObjectWithBitmapLayer());

        // Frame 1 has a keyframe (added by addNewBitmapLayer)
        const UndoSaveState* s = editor->undoRedo()->state(UndoRedoRecordType::KEYFRAME_MODIFY);
        REQUIRE(s != nullptr);
        REQUIRE(s->keyframe != nullptr);
        delete s;
        delete editor;
    }
}

TEST_CASE("UndoRedoManager::record()")
{
    SECTION("record with nullptr state is a no-op")
    {
        Editor* editor = makeEditorWithNewUndoSystem();
        editor->setObject(makeObjectWithBitmapLayer());

        const UndoSaveState* s = nullptr;
        editor->undoRedo()->record(s, "noop");

        REQUIRE(editor->undoRedo()->hasUnsavedChanges() == false);
        delete editor;
    }

    SECTION("record sets state pointer to nullptr")
    {
        Editor* editor = makeEditorWithNewUndoSystem();
        editor->setObject(makeObjectWithBitmapLayer());

        const UndoSaveState* s = editor->undoRedo()->state(UndoRedoRecordType::KEYFRAME_MODIFY);
        REQUIRE(s != nullptr);

        LayerBitmap* layer = static_cast<LayerBitmap*>(editor->layers()->currentLayer());
        layer->getBitmapImageAtFrame(1)->setPixel(0, 0, qRgba(255, 0, 0, 255));
        editor->undoRedo()->record(s, "draw");

        REQUIRE(s == nullptr);
        delete editor;
    }

    SECTION("record marks the stack as having unsaved changes")
    {
        Editor* editor = makeEditorWithNewUndoSystem();
        editor->setObject(makeObjectWithBitmapLayer());

        REQUIRE(editor->undoRedo()->hasUnsavedChanges() == false);

        recordPixelDraw(editor, qRgba(255, 0, 0, 255));

        REQUIRE(editor->undoRedo()->hasUnsavedChanges() == true);
        delete editor;
    }

    SECTION("multiple records accumulate as separate undo steps")
    {
        Editor* editor = makeEditorWithNewUndoSystem();
        editor->setObject(makeObjectWithBitmapLayer());

        recordPixelDraw(editor, qRgba(255, 0, 0, 255));
        recordPixelDraw(editor, qRgba(0, 0, 255, 255));

        REQUIRE(editor->undoRedo()->hasUnsavedChanges() == true);
        delete editor;
    }
}

TEST_CASE("UndoRedoManager - undo bitmap modification")
{
    Editor* editor = makeEditorWithNewUndoSystem();
    Object* obj = makeObjectWithBitmapLayer();
    editor->setObject(obj);

    LayerBitmap* layer = static_cast<LayerBitmap*>(editor->layers()->currentLayer());
    REQUIRE(layer != nullptr);
    REQUIRE(layer->getBitmapImageAtFrame(1) != nullptr);

    SECTION("undo restores bitmap to pre-modification state")
    {
        const QRgb originalPixel = layer->getBitmapImageAtFrame(1)->pixel(0, 0);
        const QRgb newColor = qRgba(255, 0, 0, 255);

        const UndoSaveState* s = editor->undoRedo()->state(UndoRedoRecordType::KEYFRAME_MODIFY);
        layer->getBitmapImageAtFrame(1)->setPixel(0, 0, newColor);
        editor->undoRedo()->record(s, "draw red");

        REQUIRE(layer->getBitmapImageAtFrame(1)->pixel(0, 0) == newColor);

        QAction* undoAction = editor->undoRedo()->createUndoAction(nullptr, QIcon());
        undoAction->trigger();
        delete undoAction;

        REQUIRE(layer->getBitmapImageAtFrame(1)->pixel(0, 0) == originalPixel);
    }

    SECTION("redo re-applies modification after undo")
    {
        const QRgb newColor = qRgba(0, 255, 0, 255);

        const UndoSaveState* s = editor->undoRedo()->state(UndoRedoRecordType::KEYFRAME_MODIFY);
        layer->getBitmapImageAtFrame(1)->setPixel(0, 0, newColor);
        editor->undoRedo()->record(s, "draw green");

        QAction* undoAction = editor->undoRedo()->createUndoAction(nullptr, QIcon());
        undoAction->trigger();
        delete undoAction;

        REQUIRE(layer->getBitmapImageAtFrame(1)->pixel(0, 0) != newColor);

        QAction* redoAction = editor->undoRedo()->createRedoAction(nullptr, QIcon());
        redoAction->trigger();
        delete redoAction;

        REQUIRE(layer->getBitmapImageAtFrame(1)->pixel(0, 0) == newColor);
    }

    SECTION("multiple undos restore intermediate states in order")
    {
        const QRgb originalPixel = layer->getBitmapImageAtFrame(1)->pixel(0, 0);
        const QRgb color1 = qRgba(255, 0, 0, 255);
        const QRgb color2 = qRgba(0, 0, 255, 255);

        // First modification
        const UndoSaveState* s1 = editor->undoRedo()->state(UndoRedoRecordType::KEYFRAME_MODIFY);
        layer->getBitmapImageAtFrame(1)->setPixel(0, 0, color1);
        editor->undoRedo()->record(s1, "draw red");

        // Second modification (get fresh pointer after first record)
        const UndoSaveState* s2 = editor->undoRedo()->state(UndoRedoRecordType::KEYFRAME_MODIFY);
        layer->getBitmapImageAtFrame(1)->setPixel(0, 0, color2);
        editor->undoRedo()->record(s2, "draw blue");

        REQUIRE(layer->getBitmapImageAtFrame(1)->pixel(0, 0) == color2);

        QAction* undoAction = editor->undoRedo()->createUndoAction(nullptr, QIcon());

        // Undo second modification: should restore color1
        undoAction->trigger();
        REQUIRE(layer->getBitmapImageAtFrame(1)->pixel(0, 0) == color1);

        // Undo first modification: should restore original
        undoAction->trigger();
        REQUIRE(layer->getBitmapImageAtFrame(1)->pixel(0, 0) == originalPixel);

        delete undoAction;
    }

    SECTION("undo then redo preserves full history")
    {
        const QRgb color1 = qRgba(100, 0, 0, 255);
        const QRgb color2 = qRgba(0, 100, 0, 255);

        const UndoSaveState* s1 = editor->undoRedo()->state(UndoRedoRecordType::KEYFRAME_MODIFY);
        layer->getBitmapImageAtFrame(1)->setPixel(0, 0, color1);
        editor->undoRedo()->record(s1, "first draw");

        const UndoSaveState* s2 = editor->undoRedo()->state(UndoRedoRecordType::KEYFRAME_MODIFY);
        layer->getBitmapImageAtFrame(1)->setPixel(0, 0, color2);
        editor->undoRedo()->record(s2, "second draw");

        QAction* undoAction = editor->undoRedo()->createUndoAction(nullptr, QIcon());
        QAction* redoAction = editor->undoRedo()->createRedoAction(nullptr, QIcon());

        undoAction->trigger(); // undo second: color1
        REQUIRE(layer->getBitmapImageAtFrame(1)->pixel(0, 0) == color1);

        redoAction->trigger(); // redo second: color2
        REQUIRE(layer->getBitmapImageAtFrame(1)->pixel(0, 0) == color2);

        undoAction->trigger(); // undo second again: color1
        undoAction->trigger(); // undo first: original
        REQUIRE(layer->getBitmapImageAtFrame(1)->pixel(0, 0) != color1);
        REQUIRE(layer->getBitmapImageAtFrame(1)->pixel(0, 0) != color2);

        delete undoAction;
        delete redoAction;
    }

    delete editor;
}

TEST_CASE("UndoRedoManager - hasUnsavedChanges after save and modification")
{
    SECTION("modifications after save are tracked as unsaved")
    {
        Editor* editor = makeEditorWithNewUndoSystem();
        Object* obj = makeObjectWithBitmapLayer();
        editor->setObject(obj);

        recordPixelDraw(editor, qRgba(255, 0, 0, 255));
        editor->undoRedo()->save(obj);
        REQUIRE(editor->undoRedo()->hasUnsavedChanges() == false);

        recordPixelDraw(editor, qRgba(0, 255, 0, 255));
        REQUIRE(editor->undoRedo()->hasUnsavedChanges() == true);

        delete editor;
    }

    SECTION("undoing back to save point shows no unsaved changes")
    {
        Editor* editor = makeEditorWithNewUndoSystem();
        Object* obj = makeObjectWithBitmapLayer();
        editor->setObject(obj);

        // Mark current state as saved
        editor->undoRedo()->save(obj);

        // Make a modification
        recordPixelDraw(editor, qRgba(255, 0, 0, 255));
        REQUIRE(editor->undoRedo()->hasUnsavedChanges() == true);

        // Undo back to save point
        QAction* undoAction = editor->undoRedo()->createUndoAction(nullptr, QIcon());
        undoAction->trigger();
        delete undoAction;

        REQUIRE(editor->undoRedo()->hasUnsavedChanges() == false);
        delete editor;
    }
}
