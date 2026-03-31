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
#include "preferencemanager.h"

TEST_CASE("PreferenceManager - initialization")
{
    Editor* editor = new Editor;
    editor->init();
    PreferenceManager* prefs = editor->preference();

    SECTION("init returns true")
    {
        PreferenceManager standalone(editor);
        REQUIRE(standalone.init() == true);
    }

    SECTION("load returns Status::OK")
    {
        Object obj;
        REQUIRE(prefs->load(&obj) == Status::OK);
    }

    SECTION("save returns Status::OK")
    {
        Object obj;
        REQUIRE(prefs->save(&obj) == Status::OK);
    }

    delete editor;
}

TEST_CASE("PreferenceManager - bool getter/setter")
{
    Editor* editor = new Editor;
    editor->init();
    PreferenceManager* prefs = editor->preference();

    SECTION("set and get bool - GRID")
    {
        prefs->set(SETTING::GRID, true);
        REQUIRE(prefs->isOn(SETTING::GRID) == true);

        prefs->set(SETTING::GRID, false);
        REQUIRE(prefs->isOn(SETTING::GRID) == false);
    }

    SECTION("set and get bool - ANTIALIAS")
    {
        prefs->set(SETTING::ANTIALIAS, false);
        REQUIRE(prefs->isOn(SETTING::ANTIALIAS) == false);

        prefs->set(SETTING::ANTIALIAS, true);
        REQUIRE(prefs->isOn(SETTING::ANTIALIAS) == true);
    }

    SECTION("turnOn sets bool to true")
    {
        prefs->set(SETTING::SHADOW, false);
        prefs->turnOn(SETTING::SHADOW);
        REQUIRE(prefs->isOn(SETTING::SHADOW) == true);
    }

    SECTION("turnOff sets bool to false")
    {
        prefs->set(SETTING::SHADOW, true);
        prefs->turnOff(SETTING::SHADOW);
        REQUIRE(prefs->isOn(SETTING::SHADOW) == false);
    }

    delete editor;
}

TEST_CASE("PreferenceManager - int getter/setter")
{
    Editor* editor = new Editor;
    editor->init();
    PreferenceManager* prefs = editor->preference();

    SECTION("set and get int - FPS")
    {
        prefs->set(SETTING::FPS, 24);
        REQUIRE(prefs->getInt(SETTING::FPS) == 24);
    }

    SECTION("set and get int - GRID_SIZE_W")
    {
        prefs->set(SETTING::GRID_SIZE_W, 50);
        REQUIRE(prefs->getInt(SETTING::GRID_SIZE_W) == 50);
    }

    SECTION("FRAME_SIZE is clamped to minimum 4")
    {
        prefs->set(SETTING::FRAME_SIZE, 1);
        REQUIRE(prefs->getInt(SETTING::FRAME_SIZE) == 4);
    }

    SECTION("FRAME_SIZE is clamped to maximum 40")
    {
        prefs->set(SETTING::FRAME_SIZE, 100);
        REQUIRE(prefs->getInt(SETTING::FRAME_SIZE) == 40);
    }

    SECTION("FRAME_SIZE within range is stored as-is")
    {
        prefs->set(SETTING::FRAME_SIZE, 20);
        REQUIRE(prefs->getInt(SETTING::FRAME_SIZE) == 20);
    }

    SECTION("TIMELINE_SIZE is clamped to minimum 2")
    {
        prefs->set(SETTING::TIMELINE_SIZE, 1);
        REQUIRE(prefs->getInt(SETTING::TIMELINE_SIZE) == 2);
    }

    SECTION("TIMELINE_SIZE above minimum is stored as-is")
    {
        prefs->set(SETTING::TIMELINE_SIZE, 300);
        REQUIRE(prefs->getInt(SETTING::TIMELINE_SIZE) == 300);
    }

    SECTION("LABEL_FONT_SIZE is clamped to minimum 12")
    {
        prefs->set(SETTING::LABEL_FONT_SIZE, 5);
        REQUIRE(prefs->getInt(SETTING::LABEL_FONT_SIZE) == 12);
    }

    SECTION("LABEL_FONT_SIZE above minimum is stored as-is")
    {
        prefs->set(SETTING::LABEL_FONT_SIZE, 16);
        REQUIRE(prefs->getInt(SETTING::LABEL_FONT_SIZE) == 16);
    }

    delete editor;
}

TEST_CASE("PreferenceManager - float getter/setter")
{
    Editor* editor = new Editor;
    editor->init();
    PreferenceManager* prefs = editor->preference();

    SECTION("set and get float - LAYER_VISIBILITY_THRESHOLD")
    {
        prefs->set(SETTING::LAYER_VISIBILITY_THRESHOLD, 0.75f);
        REQUIRE(qFuzzyCompare(prefs->getFloat(SETTING::LAYER_VISIBILITY_THRESHOLD), 0.75f));
    }

    SECTION("set float to zero")
    {
        prefs->set(SETTING::LAYER_VISIBILITY_THRESHOLD, 0.0f);
        REQUIRE(qFuzzyCompare(prefs->getFloat(SETTING::LAYER_VISIBILITY_THRESHOLD) + 1.0f, 1.0f));
    }

    delete editor;
}

TEST_CASE("PreferenceManager - string getter/setter")
{
    Editor* editor = new Editor;
    editor->init();
    PreferenceManager* prefs = editor->preference();

    SECTION("set and get string - BACKGROUND_STYLE")
    {
        prefs->set(SETTING::BACKGROUND_STYLE, QString("checkerboard"));
        REQUIRE(prefs->getString(SETTING::BACKGROUND_STYLE) == "checkerboard");
    }

    SECTION("set and get string - ONION_TYPE")
    {
        prefs->set(SETTING::ONION_TYPE, QString("absolute"));
        REQUIRE(prefs->getString(SETTING::ONION_TYPE) == "absolute");

        prefs->set(SETTING::ONION_TYPE, QString("relative"));
        REQUIRE(prefs->getString(SETTING::ONION_TYPE) == "relative");
    }

    SECTION("getString on an int setting returns the integer as a string")
    {
        prefs->set(SETTING::FPS, 15);
        REQUIRE(prefs->getString(SETTING::FPS) == "15");
    }

    SECTION("getString on a true bool setting returns 'true'")
    {
        prefs->set(SETTING::GRID, true);
        REQUIRE(prefs->getString(SETTING::GRID) == "true");
    }

    SECTION("getString on a false bool setting returns 'false'")
    {
        prefs->set(SETTING::GRID, false);
        REQUIRE(prefs->getString(SETTING::GRID) == "false");
    }

    delete editor;
}
