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
#include "colormanager.h"
#include "toolmanager.h"
#include "layermanager.h"
#include "playbackmanager.h"
#include "viewmanager.h"
#include "preferencemanager.h"
#include "soundmanager.h"
#include "selectionmanager.h"
#include "overlaymanager.h"
#include "clipboardmanager.h"
#include "undoredomanager.h"

TEST_CASE("Editor::init()")
{
    Editor* editor = new Editor;

    SECTION("init returns true")
    {
        REQUIRE(editor->init() == true);
    }

    SECTION("all managers are created after init")
    {
        editor->init();

        REQUIRE(editor->color() != nullptr);
        REQUIRE(editor->tools() != nullptr);
        REQUIRE(editor->layers() != nullptr);
        REQUIRE(editor->playback() != nullptr);
        REQUIRE(editor->view() != nullptr);
        REQUIRE(editor->preference() != nullptr);
        REQUIRE(editor->sound() != nullptr);
        REQUIRE(editor->select() != nullptr);
        REQUIRE(editor->overlays() != nullptr);
        REQUIRE(editor->clipboards() != nullptr);
        REQUIRE(editor->undoRedo() != nullptr);
    }

    SECTION("managers are null before init")
    {
        REQUIRE(editor->color() == nullptr);
        REQUIRE(editor->tools() == nullptr);
        REQUIRE(editor->layers() == nullptr);
        REQUIRE(editor->playback() == nullptr);
        REQUIRE(editor->view() == nullptr);
        REQUIRE(editor->preference() == nullptr);
        REQUIRE(editor->sound() == nullptr);
        REQUIRE(editor->select() == nullptr);
        REQUIRE(editor->overlays() == nullptr);
        REQUIRE(editor->clipboards() == nullptr);
        REQUIRE(editor->undoRedo() == nullptr);
    }

    delete editor;
}

static Object* makeObject()
{
    Object* obj = new Object;
    obj->init();
    obj->addNewBitmapLayer();
    return obj;
}

TEST_CASE("Editor::setObject()")
{
    Editor* editor = new Editor;
    editor->init();

    SECTION("object is null before setObject")
    {
        REQUIRE(editor->object() == nullptr);
    }

    SECTION("object is set after setObject")
    {
        Object* object = makeObject();
        editor->setObject(object);
        REQUIRE(editor->object() == object);
    }

    SECTION("setObject returns OK")
    {
        REQUIRE(editor->setObject(makeObject()) == Status::OK);
    }

    SECTION("setObject replaces existing object")
    {
        Object* object1 = makeObject();
        Object* object2 = makeObject();
        editor->setObject(object1);
        REQUIRE(editor->object() == object1);

        editor->setObject(object2);
        REQUIRE(editor->object() == object2);
    }

    delete editor;
}

TEST_CASE("Editor default state")
{
    Editor* editor = new Editor;
    editor->init();
    editor->setObject(makeObject());

    SECTION("default frame is 1")
    {
        REQUIRE(editor->currentFrame() == 1);
    }

    SECTION("current layer index is 0")
    {
        REQUIRE(editor->currentLayerIndex() == 0);
    }

    delete editor;
}
