/*

Pencil2D - Traditional Animation Software
Copyright (C) 2005-2007 Patrick Corrieri & Pascal Naidon
Copyright (C) 2012-2020 Matthew Chiawen Chang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/

#include "commandregistry.h"
#include "pencildef.h"

// Single source of truth: every command's ID, default shortcut, display name, and category.
// Adding a new command = adding one line here.
const CommandDefinition CommandRegistry::kDefinitions[] =
{
    // --- File ---
    { CMD_NEW_FILE,                  "Ctrl+N",       QT_TR_NOOP("New File"),                    CommandCategory::File },
    { CMD_OPEN_FILE,                 "Ctrl+O",       QT_TR_NOOP("Open File"),                   CommandCategory::File },
    { CMD_SAVE_FILE,                 "Ctrl+S",       QT_TR_NOOP("Save File"),                   CommandCategory::File },
    { CMD_SAVE_AS,                   "Ctrl+Shift+S", QT_TR_NOOP("Save File As"),                CommandCategory::File },
    { CMD_IMPORT_IMAGE,              "",              QT_TR_NOOP("Import Image"),                CommandCategory::File },
    { CMD_IMPORT_IMAGE_SEQ,          "",              QT_TR_NOOP("Import Image Sequence"),       CommandCategory::File },
    { CMD_IMPORT_IMAGE_PREDEFINED_SET, "",            QT_TR_NOOP("Import Image Predefined Set"), CommandCategory::File },
    { CMD_IMPORT_MOVIE_VIDEO,        "",              QT_TR_NOOP("Import Movie Video"),          CommandCategory::File },
    { CMD_IMPORT_ANIMATED_IMAGE,     "",              QT_TR_NOOP("Import Animated Image"),       CommandCategory::File },
    { CMD_IMPORT_LAYERS,             "",              QT_TR_NOOP("Import Layers from project file"), CommandCategory::File },
    { CMD_IMPORT_SOUND,              "",              QT_TR_NOOP("Import Sound"),                CommandCategory::File },
    { CMD_IMPORT_MOVIE_AUDIO,        "",              QT_TR_NOOP("Import Movie Audio"),          CommandCategory::File },
    { CMD_IMPORT_PALETTE,            "",              QT_TR_NOOP("Import Palette (Append)"),     CommandCategory::File },
    { CMD_IMPORT_PALETTE_REPLACE,    "",              QT_TR_NOOP("Import Palette (Replace)"),    CommandCategory::File },
    { CMD_EXPORT_IMAGE,              "Ctrl+Shift+R", QT_TR_NOOP("Export Image"),                CommandCategory::File },
    { CMD_EXPORT_IMAGE_SEQ,          "Ctrl+R",       QT_TR_NOOP("Export Image Sequence"),       CommandCategory::File },
    { CMD_EXPORT_MOVIE,              "",              QT_TR_NOOP("Export Movie"),                CommandCategory::File },
    { CMD_EXPORT_GIF,                "Ctrl+G",       QT_TR_NOOP("Export Animated GIF"),         CommandCategory::File },
    { CMD_EXPORT_PALETTE,            "",              QT_TR_NOOP("Export Palette"),              CommandCategory::File },

    // --- Edit ---
    { CMD_UNDO,                      "Ctrl+Z",       QT_TR_NOOP("Undo"),                       CommandCategory::Edit },
    { CMD_REDO,                      "Ctrl+Shift+Z", QT_TR_NOOP("Redo"),                       CommandCategory::Edit },
    { CMD_CUT,                       "Ctrl+X",       QT_TR_NOOP("Cut"),                        CommandCategory::Edit },
    { CMD_COPY,                      "Ctrl+C",       QT_TR_NOOP("Copy"),                       CommandCategory::Edit },
    { CMD_PASTE,                     "Ctrl+V",       QT_TR_NOOP("Paste"),                      CommandCategory::Edit },
    { CMD_PASTE_FROM_PREVIOUS,       "Ctrl+Left",    QT_TR_NOOP("Paste from Previous Keyframe"), CommandCategory::Edit },
    { CMD_SELECT_ALL,                "Ctrl+A",       QT_TR_NOOP("Select All"),                 CommandCategory::Edit },
    { CMD_DESELECT_ALL,              "Ctrl+D",       QT_TR_NOOP("Deselect All"),               CommandCategory::Edit },
    { CMD_CLEAR_FRAME,               "",              QT_TR_NOOP("Clear Frame"),                CommandCategory::Edit },
    { CMD_SELECTION_FLIP_HORIZONTAL, "",              QT_TR_NOOP("Selection: Horizontal Flip"), CommandCategory::Edit },
    { CMD_SELECTION_FLIP_VERTICAL,   "",              QT_TR_NOOP("Selection: Vertical Flip"),   CommandCategory::Edit },
    { CMD_PEGBAR_ALIGNMENT,          "",              QT_TR_NOOP("Peg bar Alignment"),          CommandCategory::Edit },
    { CMD_PREFERENCE,                "",              QT_TR_NOOP("Preferences"),                CommandCategory::Edit },

    // --- View ---
    { CMD_RESET_WINDOWS,             "",              QT_TR_NOOP("Reset Windows"),              CommandCategory::View },
    { CMD_LOCK_WINDOWS,              "",              QT_TR_NOOP("Lock Windows"),               CommandCategory::View },
    { CMD_ZOOM_IN,                   "Ctrl+Up",      QT_TR_NOOP("Zoom In"),                    CommandCategory::View },
    { CMD_ZOOM_OUT,                  "Ctrl+Down",    QT_TR_NOOP("Zoom Out"),                   CommandCategory::View },
    { CMD_ZOOM_400,                  "4",            QT_TR_NOOP("Set Zoom to 400%"),           CommandCategory::View },
    { CMD_ZOOM_300,                  "3",            QT_TR_NOOP("Set Zoom to 300%"),           CommandCategory::View },
    { CMD_ZOOM_200,                  "2",            QT_TR_NOOP("Set Zoom to 200%"),           CommandCategory::View },
    { CMD_ZOOM_100,                  "1",            QT_TR_NOOP("Set Zoom to 100%"),           CommandCategory::View },
    { CMD_ZOOM_50,                   "Shift+2",      QT_TR_NOOP("Set Zoom to 50%"),            CommandCategory::View },
    { CMD_ZOOM_33,                   "Shift+3",      QT_TR_NOOP("Set Zoom to 33%"),            CommandCategory::View },
    { CMD_ZOOM_25,                   "Shift+4",      QT_TR_NOOP("Set Zoom to 25%"),            CommandCategory::View },
    { CMD_ROTATE_CLOCK,              "R",            QT_TR_NOOP("Rotate Clockwise"),           CommandCategory::View },
    { CMD_ROTATE_ANTI_CLOCK,         "Z",            QT_TR_NOOP("Rotate Anticlockwise"),       CommandCategory::View },
    { CMD_RESET_ROTATION,            "Alt+H",        QT_TR_NOOP("Reset Rotation"),             CommandCategory::View },
    { CMD_RESET_ZOOM_ROTATE,         "Ctrl+H",       QT_TR_NOOP("Reset View"),                 CommandCategory::View },
    { CMD_CENTER_VIEW,               "Ctrl+Shift+H", QT_TR_NOOP("Center View"),                CommandCategory::View },
    { CMD_FLIP_HORIZONTAL,           "Shift+H",      QT_TR_NOOP("View: Horizontal Flip"),      CommandCategory::View },
    { CMD_FLIP_VERTICAL,             "Shift+V",      QT_TR_NOOP("View: Vertical Flip"),        CommandCategory::View },
    { CMD_GRID,                      "G",            QT_TR_NOOP("Toggle Grid"),                CommandCategory::View },
    { CMD_OVERLAY_CENTER,            "",              QT_TR_NOOP("Toggle Center Overlay"),      CommandCategory::View },
    { CMD_OVERLAY_THIRDS,            "",              QT_TR_NOOP("Toggle Thirds Overlay"),      CommandCategory::View },
    { CMD_OVERLAY_GOLDEN_RATIO,      "",              QT_TR_NOOP("Toggle Golden Ratio Overlay"), CommandCategory::View },
    { CMD_OVERLAY_SAFE_AREAS,        "",              QT_TR_NOOP("Toggle Safe Areas Overlay"),  CommandCategory::View },
    { CMD_OVERLAY_ONE_POINT_PERSPECTIVE,  "",         QT_TR_NOOP("Toggle One Point Perspective Overlay"), CommandCategory::View },
    { CMD_OVERLAY_TWO_POINT_PERSPECTIVE,  "",         QT_TR_NOOP("Toggle Two Point Perspective Overlay"), CommandCategory::View },
    { CMD_OVERLAY_THREE_POINT_PERSPECTIVE, "",        QT_TR_NOOP("Toggle Three Point Perspective Overlay"), CommandCategory::View },
    { CMD_ONIONSKIN_PREV,            "O",            QT_TR_NOOP("Toggle Previous Onion Skin"), CommandCategory::View },
    { CMD_ONIONSKIN_NEXT,            "Alt+O",        QT_TR_NOOP("Toggle Next Onion Skin"),     CommandCategory::View },
    { CMD_TOGGLE_STATUS_BAR,         "",              QT_TR_NOOP("Toggle Status Bar Visibility"), CommandCategory::View },

    // --- Animation ---
    { CMD_PLAY,                      "Ctrl+Return",  QT_TR_NOOP("Play/Stop"),                  CommandCategory::Animation },
    { CMD_LOOP,                      "Ctrl+L",       QT_TR_NOOP("Toggle Loop"),                CommandCategory::Animation },
    { CMD_LOOP_CONTROL,              "",              QT_TR_NOOP("Toggle Range Playback"),      CommandCategory::Animation },
    { CMD_GOTO_NEXT_FRAME,           ".",             QT_TR_NOOP("Next Frame"),                 CommandCategory::Animation },
    { CMD_GOTO_PREV_FRAME,           ",",             QT_TR_NOOP("Previous Frame"),             CommandCategory::Animation },
    { CMD_GOTO_NEXT_KEY_FRAME,       "Alt+.",         QT_TR_NOOP("Next Keyframe"),              CommandCategory::Animation },
    { CMD_GOTO_PREV_KEY_FRAME,       "Alt+,",         QT_TR_NOOP("Previous Keyframe"),          CommandCategory::Animation },
    { CMD_ADD_FRAME,                 "F7",            QT_TR_NOOP("Add Frame"),                  CommandCategory::Animation },
    { CMD_DUPLICATE_FRAME,           "F6",            QT_TR_NOOP("Duplicate Frame"),            CommandCategory::Animation },
    { CMD_REMOVE_FRAME,              "Shift+F5",      QT_TR_NOOP("Remove Frame"),               CommandCategory::Animation },
    { CMD_SELECTION_ADD_FRAME_EXPOSURE,      "Ctrl++", QT_TR_NOOP("Selection: Add Frame Exposure"), CommandCategory::Animation },
    { CMD_SELECTION_SUBTRACT_FRAME_EXPOSURE, "Ctrl+-", QT_TR_NOOP("Selection: Subtract Frame Exposure"), CommandCategory::Animation },
    { CMD_SELECTION_REPOSITION_FRAMES,       "",       QT_TR_NOOP("Selection: Reposition Frames"), CommandCategory::Animation },
    { CMD_REVERSE_SELECTED_FRAMES,   "",              QT_TR_NOOP("Selection: Reverse Keyframes"), CommandCategory::Animation },
    { CMD_REMOVE_SELECTED_FRAMES,    "",              QT_TR_NOOP("Selection: Remove Keyframes"), CommandCategory::Animation },
    { CMD_MOVE_FRAME_FORWARD,        "Ctrl+.",        QT_TR_NOOP("Move Frame Forward"),         CommandCategory::Animation },
    { CMD_MOVE_FRAME_BACKWARD,       "Ctrl+,",        QT_TR_NOOP("Move Frame Backward"),        CommandCategory::Animation },
    { CMD_FLIP_INBETWEEN,            "Alt+Z",         QT_TR_NOOP("Flip In-Between"),            CommandCategory::Animation },
    { CMD_FLIP_ROLLING,              "Alt+X",         QT_TR_NOOP("Flip Rolling"),               CommandCategory::Animation },

    // --- Tools ---
    { CMD_TOOL_MOVE,                 "M",            QT_TR_NOOP("Move Tool"),                  CommandCategory::Tool },
    { CMD_TOOL_SELECT,               "V",            QT_TR_NOOP("Select Tool"),                CommandCategory::Tool },
    { CMD_TOOL_BRUSH,                "B",            QT_TR_NOOP("Brush Tool"),                 CommandCategory::Tool },
    { CMD_TOOL_POLYLINE,             "Y",            QT_TR_NOOP("Polyline Tool"),              CommandCategory::Tool },
    { CMD_TOOL_SMUDGE,               "A",            QT_TR_NOOP("Smudge Tool"),                CommandCategory::Tool },
    { CMD_TOOL_PEN,                  "P",            QT_TR_NOOP("Pen Tool"),                   CommandCategory::Tool },
    { CMD_TOOL_HAND,                 "H",            QT_TR_NOOP("Hand Tool"),                  CommandCategory::Tool },
    { CMD_TOOL_PENCIL,               "N",            QT_TR_NOOP("Pencil Tool"),                CommandCategory::Tool },
    { CMD_TOOL_BUCKET,               "K",            QT_TR_NOOP("Bucket Tool"),                CommandCategory::Tool },
    { CMD_TOOL_EYEDROPPER,           "I",            QT_TR_NOOP("Eyedropper Tool"),            CommandCategory::Tool },
    { CMD_TOOL_ERASER,               "E",            QT_TR_NOOP("Eraser Tool"),                CommandCategory::Tool },
    { CMD_RESET_ALL_TOOLS,           "",              QT_TR_NOOP("Reset all tools to default"), CommandCategory::Tool },

    // --- Layer ---
    { CMD_NEW_BITMAP_LAYER,          "Ctrl+Alt+B",   QT_TR_NOOP("New Bitmap Layer"),           CommandCategory::Layer },
    { CMD_NEW_VECTOR_LAYER,          "Ctrl+Alt+V",   QT_TR_NOOP("New Vector Layer"),           CommandCategory::Layer },
    { CMD_NEW_SOUND_LAYER,           "Ctrl+Alt+W",   QT_TR_NOOP("New Sound Layer"),            CommandCategory::Layer },
    { CMD_NEW_CAMERA_LAYER,          "Ctrl+Alt+C",   QT_TR_NOOP("New Camera Layer"),           CommandCategory::Layer },
    { CMD_DELETE_CUR_LAYER,          "",              QT_TR_NOOP("Delete Current Layer"),       CommandCategory::Layer },
    { CMD_CHANGE_LINE_COLOR_KEYFRAME, "",             QT_TR_NOOP("Change Line Color (Current keyframe)"), CommandCategory::Layer },
    { CMD_CHANGE_LINE_COLOR_LAYER,   "",              QT_TR_NOOP("Change Line Color (All keyframes on layer)"), CommandCategory::Layer },
    { CMD_CHANGE_LAYER_OPACITY,      "",              QT_TR_NOOP("Change Layer / Keyframe Opacity"), CommandCategory::Layer },
    { CMD_CURRENT_LAYER_VISIBILITY,  "Alt+1",        QT_TR_NOOP("Show Current Layer Only"),    CommandCategory::Layer },
    { CMD_RELATIVE_LAYER_VISIBILITY, "Alt+2",        QT_TR_NOOP("Show Layers Relative to Current Layer"), CommandCategory::Layer },
    { CMD_ALL_LAYER_VISIBILITY,      "Alt+3",        QT_TR_NOOP("Show All Layers"),            CommandCategory::Layer },

    // --- Window ---
    { CMD_TOGGLE_TOOLBOX,            "Ctrl+1",       QT_TR_NOOP("Toggle Tools Window Visibility"), CommandCategory::Window },
    { CMD_TOGGLE_TOOL_OPTIONS,       "Ctrl+2",       QT_TR_NOOP("Toggle Options Window Visibility"), CommandCategory::Window },
    { CMD_TOGGLE_COLOR_WHEEL,        "Ctrl+3",       QT_TR_NOOP("Toggle Color Box Window Visibility"), CommandCategory::Window },
    { CMD_TOGGLE_COLOR_LIBRARY,      "Ctrl+4",       QT_TR_NOOP("Toggle Color Palette Window Visibility"), CommandCategory::Window },
    { CMD_TOGGLE_TIMELINE,           "Ctrl+6",       QT_TR_NOOP("Toggle Timeline Window Visibility"), CommandCategory::Window },
    { CMD_TOGGLE_COLOR_INSPECTOR,    "Ctrl+7",       QT_TR_NOOP("Toggle Color Inspector Window Visibility"), CommandCategory::Window },
    { CMD_TOGGLE_ONION_SKIN,         "Ctrl+8",       QT_TR_NOOP("Toggle Onion Skins Window Visibility"), CommandCategory::Window },

    // --- Help ---
    { CMD_HELP,                      "",              QT_TR_NOOP("Help"),                       CommandCategory::Help },
    { CMD_EXIT,                      "Ctrl+Q",       QT_TR_NOOP("Exit"),                       CommandCategory::Help },
};

const int CommandRegistry::kDefinitionCount = sizeof(kDefinitions) / sizeof(kDefinitions[0]);

const CommandRegistry& CommandRegistry::instance()
{
    static CommandRegistry reg;
    return reg;
}

CommandRegistry::CommandRegistry()
{
    populate();
}

void CommandRegistry::populate()
{
    mAll.reserve(kDefinitionCount);
    for (int i = 0; i < kDefinitionCount; ++i)
    {
        const CommandDefinition* def = &kDefinitions[i];
        mLookup.insert(QString::fromLatin1(def->id), def);
        mAll.append(def);
    }
}

const CommandDefinition* CommandRegistry::find(const QString& cmdId) const
{
    return mLookup.value(cmdId, nullptr);
}

QVector<const CommandDefinition*> CommandRegistry::byCategory(CommandCategory cat) const
{
    QVector<const CommandDefinition*> result;
    for (const CommandDefinition* def : mAll)
    {
        if (def->category == cat)
        {
            result.append(def);
        }
    }
    return result;
}

QVector<const CommandDefinition*> CommandRegistry::all() const
{
    return mAll;
}

int CommandRegistry::count() const
{
    return mAll.size();
}
