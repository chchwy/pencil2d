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

#include "commandregistry.h"
#include "pencildef.h"

#include <QSet>

TEST_CASE("CommandRegistry::instance returns consistent singleton")
{
    const CommandRegistry& a = CommandRegistry::instance();
    const CommandRegistry& b = CommandRegistry::instance();
    REQUIRE(&a == &b);
}

TEST_CASE("CommandRegistry has entries")
{
    const CommandRegistry& reg = CommandRegistry::instance();
    REQUIRE(reg.count() > 0);
}

TEST_CASE("Every command has non-empty id and displayName")
{
    const CommandRegistry& reg = CommandRegistry::instance();
    for (const CommandDefinition* def : reg.all())
    {
        REQUIRE(def->id != nullptr);
        REQUIRE(def->id[0] != '\0');
        REQUIRE(def->displayName != nullptr);
        REQUIRE(def->displayName[0] != '\0');
    }
}

TEST_CASE("No duplicate command IDs")
{
    const CommandRegistry& reg = CommandRegistry::instance();
    QSet<QString> ids;
    for (const CommandDefinition* def : reg.all())
    {
        QString id = QString::fromLatin1(def->id);
        REQUIRE_FALSE(ids.contains(id));
        ids.insert(id);
    }
}

TEST_CASE("find returns correct definition")
{
    const CommandRegistry& reg = CommandRegistry::instance();

    SECTION("Known command")
    {
        const CommandDefinition* def = reg.find("CmdNewFile");
        REQUIRE(def != nullptr);
        REQUIRE(QString::fromLatin1(def->id) == "CmdNewFile");
        REQUIRE(QString::fromLatin1(def->defaultShortcut) == "Ctrl+N");
        REQUIRE(def->category == CommandCategory::File);
    }

    SECTION("Unknown command returns nullptr")
    {
        const CommandDefinition* def = reg.find("CmdDoesNotExist");
        REQUIRE(def == nullptr);
    }
}

TEST_CASE("byCategory returns only commands of that category")
{
    const CommandRegistry& reg = CommandRegistry::instance();
    auto tools = reg.byCategory(CommandCategory::Tool);

    REQUIRE_FALSE(tools.isEmpty());
    for (const CommandDefinition* def : tools)
    {
        REQUIRE(def->category == CommandCategory::Tool);
    }
}

TEST_CASE("Every CMD_* macro from pencildef.h is in the registry")
{
    const CommandRegistry& reg = CommandRegistry::instance();

    // File
    REQUIRE(reg.find(CMD_NEW_FILE) != nullptr);
    REQUIRE(reg.find(CMD_OPEN_FILE) != nullptr);
    REQUIRE(reg.find(CMD_SAVE_FILE) != nullptr);
    REQUIRE(reg.find(CMD_SAVE_AS) != nullptr);
    REQUIRE(reg.find(CMD_IMPORT_IMAGE) != nullptr);
    REQUIRE(reg.find(CMD_IMPORT_IMAGE_SEQ) != nullptr);
    REQUIRE(reg.find(CMD_IMPORT_IMAGE_PREDEFINED_SET) != nullptr);
    REQUIRE(reg.find(CMD_IMPORT_MOVIE_VIDEO) != nullptr);
    REQUIRE(reg.find(CMD_IMPORT_ANIMATED_IMAGE) != nullptr);
    REQUIRE(reg.find(CMD_IMPORT_LAYERS) != nullptr);
    REQUIRE(reg.find(CMD_IMPORT_SOUND) != nullptr);
    REQUIRE(reg.find(CMD_IMPORT_MOVIE_AUDIO) != nullptr);
    REQUIRE(reg.find(CMD_IMPORT_PALETTE) != nullptr);
    REQUIRE(reg.find(CMD_IMPORT_PALETTE_REPLACE) != nullptr);
    REQUIRE(reg.find(CMD_EXPORT_IMAGE) != nullptr);
    REQUIRE(reg.find(CMD_EXPORT_IMAGE_SEQ) != nullptr);
    REQUIRE(reg.find(CMD_EXPORT_MOVIE) != nullptr);
    REQUIRE(reg.find(CMD_EXPORT_GIF) != nullptr);
    REQUIRE(reg.find(CMD_EXPORT_PALETTE) != nullptr);

    // Edit
    REQUIRE(reg.find(CMD_UNDO) != nullptr);
    REQUIRE(reg.find(CMD_REDO) != nullptr);
    REQUIRE(reg.find(CMD_CUT) != nullptr);
    REQUIRE(reg.find(CMD_COPY) != nullptr);
    REQUIRE(reg.find(CMD_PASTE) != nullptr);
    REQUIRE(reg.find(CMD_PASTE_FROM_PREVIOUS) != nullptr);
    REQUIRE(reg.find(CMD_SELECT_ALL) != nullptr);
    REQUIRE(reg.find(CMD_DESELECT_ALL) != nullptr);
    REQUIRE(reg.find(CMD_CLEAR_FRAME) != nullptr);
    REQUIRE(reg.find(CMD_SELECTION_FLIP_HORIZONTAL) != nullptr);
    REQUIRE(reg.find(CMD_SELECTION_FLIP_VERTICAL) != nullptr);
    REQUIRE(reg.find(CMD_PEGBAR_ALIGNMENT) != nullptr);
    REQUIRE(reg.find(CMD_PREFERENCE) != nullptr);

    // View
    REQUIRE(reg.find(CMD_RESET_WINDOWS) != nullptr);
    REQUIRE(reg.find(CMD_LOCK_WINDOWS) != nullptr);
    REQUIRE(reg.find(CMD_ZOOM_IN) != nullptr);
    REQUIRE(reg.find(CMD_ZOOM_OUT) != nullptr);
    REQUIRE(reg.find(CMD_ZOOM_400) != nullptr);
    REQUIRE(reg.find(CMD_ZOOM_300) != nullptr);
    REQUIRE(reg.find(CMD_ZOOM_200) != nullptr);
    REQUIRE(reg.find(CMD_ZOOM_100) != nullptr);
    REQUIRE(reg.find(CMD_ZOOM_50) != nullptr);
    REQUIRE(reg.find(CMD_ZOOM_33) != nullptr);
    REQUIRE(reg.find(CMD_ZOOM_25) != nullptr);
    REQUIRE(reg.find(CMD_ROTATE_CLOCK) != nullptr);
    REQUIRE(reg.find(CMD_ROTATE_ANTI_CLOCK) != nullptr);
    REQUIRE(reg.find(CMD_RESET_ROTATION) != nullptr);
    REQUIRE(reg.find(CMD_RESET_ZOOM_ROTATE) != nullptr);
    REQUIRE(reg.find(CMD_CENTER_VIEW) != nullptr);
    REQUIRE(reg.find(CMD_FLIP_HORIZONTAL) != nullptr);
    REQUIRE(reg.find(CMD_FLIP_VERTICAL) != nullptr);
    REQUIRE(reg.find(CMD_GRID) != nullptr);
    REQUIRE(reg.find(CMD_OVERLAY_CENTER) != nullptr);
    REQUIRE(reg.find(CMD_OVERLAY_THIRDS) != nullptr);
    REQUIRE(reg.find(CMD_OVERLAY_GOLDEN_RATIO) != nullptr);
    REQUIRE(reg.find(CMD_OVERLAY_SAFE_AREAS) != nullptr);
    REQUIRE(reg.find(CMD_OVERLAY_ONE_POINT_PERSPECTIVE) != nullptr);
    REQUIRE(reg.find(CMD_OVERLAY_TWO_POINT_PERSPECTIVE) != nullptr);
    REQUIRE(reg.find(CMD_OVERLAY_THREE_POINT_PERSPECTIVE) != nullptr);
    REQUIRE(reg.find(CMD_ONIONSKIN_PREV) != nullptr);
    REQUIRE(reg.find(CMD_ONIONSKIN_NEXT) != nullptr);
    REQUIRE(reg.find(CMD_TOGGLE_STATUS_BAR) != nullptr);

    // Animation
    REQUIRE(reg.find(CMD_PLAY) != nullptr);
    REQUIRE(reg.find(CMD_LOOP) != nullptr);
    REQUIRE(reg.find(CMD_LOOP_CONTROL) != nullptr);
    REQUIRE(reg.find(CMD_GOTO_NEXT_FRAME) != nullptr);
    REQUIRE(reg.find(CMD_GOTO_PREV_FRAME) != nullptr);
    REQUIRE(reg.find(CMD_GOTO_NEXT_KEY_FRAME) != nullptr);
    REQUIRE(reg.find(CMD_GOTO_PREV_KEY_FRAME) != nullptr);
    REQUIRE(reg.find(CMD_ADD_FRAME) != nullptr);
    REQUIRE(reg.find(CMD_DUPLICATE_FRAME) != nullptr);
    REQUIRE(reg.find(CMD_REMOVE_FRAME) != nullptr);
    REQUIRE(reg.find(CMD_SELECTION_ADD_FRAME_EXPOSURE) != nullptr);
    REQUIRE(reg.find(CMD_SELECTION_SUBTRACT_FRAME_EXPOSURE) != nullptr);
    REQUIRE(reg.find(CMD_SELECTION_REPOSITION_FRAMES) != nullptr);
    REQUIRE(reg.find(CMD_REVERSE_SELECTED_FRAMES) != nullptr);
    REQUIRE(reg.find(CMD_REMOVE_SELECTED_FRAMES) != nullptr);
    REQUIRE(reg.find(CMD_MOVE_FRAME_FORWARD) != nullptr);
    REQUIRE(reg.find(CMD_MOVE_FRAME_BACKWARD) != nullptr);
    REQUIRE(reg.find(CMD_FLIP_INBETWEEN) != nullptr);
    REQUIRE(reg.find(CMD_FLIP_ROLLING) != nullptr);

    // Tools
    REQUIRE(reg.find(CMD_TOOL_MOVE) != nullptr);
    REQUIRE(reg.find(CMD_TOOL_SELECT) != nullptr);
    REQUIRE(reg.find(CMD_TOOL_BRUSH) != nullptr);
    REQUIRE(reg.find(CMD_TOOL_POLYLINE) != nullptr);
    REQUIRE(reg.find(CMD_TOOL_SMUDGE) != nullptr);
    REQUIRE(reg.find(CMD_TOOL_PEN) != nullptr);
    REQUIRE(reg.find(CMD_TOOL_HAND) != nullptr);
    REQUIRE(reg.find(CMD_TOOL_PENCIL) != nullptr);
    REQUIRE(reg.find(CMD_TOOL_BUCKET) != nullptr);
    REQUIRE(reg.find(CMD_TOOL_EYEDROPPER) != nullptr);
    REQUIRE(reg.find(CMD_TOOL_ERASER) != nullptr);
    REQUIRE(reg.find(CMD_RESET_ALL_TOOLS) != nullptr);

    // Layer
    REQUIRE(reg.find(CMD_NEW_BITMAP_LAYER) != nullptr);
    REQUIRE(reg.find(CMD_NEW_VECTOR_LAYER) != nullptr);
    REQUIRE(reg.find(CMD_NEW_SOUND_LAYER) != nullptr);
    REQUIRE(reg.find(CMD_NEW_CAMERA_LAYER) != nullptr);
    REQUIRE(reg.find(CMD_DELETE_CUR_LAYER) != nullptr);
    REQUIRE(reg.find(CMD_CHANGE_LINE_COLOR_KEYFRAME) != nullptr);
    REQUIRE(reg.find(CMD_CHANGE_LINE_COLOR_LAYER) != nullptr);
    REQUIRE(reg.find(CMD_CHANGE_LAYER_OPACITY) != nullptr);
    REQUIRE(reg.find(CMD_CURRENT_LAYER_VISIBILITY) != nullptr);
    REQUIRE(reg.find(CMD_RELATIVE_LAYER_VISIBILITY) != nullptr);
    REQUIRE(reg.find(CMD_ALL_LAYER_VISIBILITY) != nullptr);

    // Window
    REQUIRE(reg.find(CMD_TOGGLE_TOOLBOX) != nullptr);
    REQUIRE(reg.find(CMD_TOGGLE_TOOL_OPTIONS) != nullptr);
    REQUIRE(reg.find(CMD_TOGGLE_COLOR_WHEEL) != nullptr);
    REQUIRE(reg.find(CMD_TOGGLE_COLOR_LIBRARY) != nullptr);
    REQUIRE(reg.find(CMD_TOGGLE_TIMELINE) != nullptr);
    REQUIRE(reg.find(CMD_TOGGLE_COLOR_INSPECTOR) != nullptr);
    REQUIRE(reg.find(CMD_TOGGLE_ONION_SKIN) != nullptr);

    // Help
    REQUIRE(reg.find(CMD_HELP) != nullptr);
    REQUIRE(reg.find(CMD_EXIT) != nullptr);
}

TEST_CASE("All categories have at least one command")
{
    const CommandRegistry& reg = CommandRegistry::instance();
    REQUIRE_FALSE(reg.byCategory(CommandCategory::File).isEmpty());
    REQUIRE_FALSE(reg.byCategory(CommandCategory::Edit).isEmpty());
    REQUIRE_FALSE(reg.byCategory(CommandCategory::View).isEmpty());
    REQUIRE_FALSE(reg.byCategory(CommandCategory::Animation).isEmpty());
    REQUIRE_FALSE(reg.byCategory(CommandCategory::Tool).isEmpty());
    REQUIRE_FALSE(reg.byCategory(CommandCategory::Layer).isEmpty());
    REQUIRE_FALSE(reg.byCategory(CommandCategory::Window).isEmpty());
    REQUIRE_FALSE(reg.byCategory(CommandCategory::Help).isEmpty());
}

TEST_CASE("byCategory results sum to total count")
{
    const CommandRegistry& reg = CommandRegistry::instance();
    int sum = 0;
    sum += reg.byCategory(CommandCategory::File).size();
    sum += reg.byCategory(CommandCategory::Edit).size();
    sum += reg.byCategory(CommandCategory::View).size();
    sum += reg.byCategory(CommandCategory::Animation).size();
    sum += reg.byCategory(CommandCategory::Tool).size();
    sum += reg.byCategory(CommandCategory::Layer).size();
    sum += reg.byCategory(CommandCategory::Window).size();
    sum += reg.byCategory(CommandCategory::Help).size();
    REQUIRE(sum == reg.count());
}

TEST_CASE("Default shortcuts from registry match expected values")
{
    const CommandRegistry& reg = CommandRegistry::instance();

    SECTION("Standard shortcuts")
    {
        REQUIRE(QString::fromLatin1(reg.find(CMD_UNDO)->defaultShortcut) == "Ctrl+Z");
        REQUIRE(QString::fromLatin1(reg.find(CMD_REDO)->defaultShortcut) == "Ctrl+Shift+Z");
        REQUIRE(QString::fromLatin1(reg.find(CMD_CUT)->defaultShortcut) == "Ctrl+X");
        REQUIRE(QString::fromLatin1(reg.find(CMD_COPY)->defaultShortcut) == "Ctrl+C");
        REQUIRE(QString::fromLatin1(reg.find(CMD_PASTE)->defaultShortcut) == "Ctrl+V");
    }

    SECTION("Tool shortcuts")
    {
        REQUIRE(QString::fromLatin1(reg.find(CMD_TOOL_BRUSH)->defaultShortcut) == "B");
        REQUIRE(QString::fromLatin1(reg.find(CMD_TOOL_ERASER)->defaultShortcut) == "E");
        REQUIRE(QString::fromLatin1(reg.find(CMD_TOOL_PEN)->defaultShortcut) == "P");
    }

    SECTION("Commands with no default shortcut")
    {
        REQUIRE(QString::fromLatin1(reg.find(CMD_CLEAR_FRAME)->defaultShortcut) == "");
        REQUIRE(QString::fromLatin1(reg.find(CMD_IMPORT_IMAGE)->defaultShortcut) == "");
    }
}
