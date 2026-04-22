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

#ifndef PENCILDEF_H
#define PENCILDEF_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define S__GIT_TIMESTAMP TOSTRING(GIT_TIMESTAMP)
#define S__GIT_COMMIT_HASH TOSTRING(GIT_CURRENT_SHA1)

enum ToolCategory: int
{
    BASETOOL        = 0,
    STROKETOOL      = 1,
    TRANSFORMTOOL   = 2
};

enum ToolType : int
{
    INVALID_TOOL = -1,
    PENCIL = 0,
    ERASER,
    SELECT,
    MOVE,
    HAND,
    SMUDGE,
    CAMERA,
    PEN,
    POLYLINE,
    BUCKET,
    EYEDROPPER,
    BRUSH,
    TOOL_TYPE_COUNT
};

enum class DotColorType {
    RED,
    BLUE,
    GREEN,
    BLACK,
    WHITE
};

enum BackgroundStyle
{

};

enum StabilizationLevel
{
    NONE,
    SIMPLE,
    STRONG
};

enum TimecodeTextLevel
{
    NOTEXT,
    FRAMES, // FF
    SMPTE,  // HH:MM:SS:FF
    SFF     // S:FF
};

enum class LayerVisibility
{
    CURRENTONLY = 0,
    RELATED = 1,
    ALL = 2,
    // If you are adding new enum values here, be sure to update the ++/-- operators below
};

inline LayerVisibility& operator++(LayerVisibility& vis)
{
    return vis = (vis == LayerVisibility::ALL) ? LayerVisibility::CURRENTONLY : static_cast<LayerVisibility>(static_cast<int>(vis)+1);
}

inline LayerVisibility& operator--(LayerVisibility& vis)
{
    return vis = (vis == LayerVisibility::CURRENTONLY) ? LayerVisibility::ALL : static_cast<LayerVisibility>(static_cast<int>(vis)-1);
}

// Max frames that can be imported and loaded onto the timeline
const static int MaxFramesBound = 9999;

// Save / Export
#define LAST_PCLX_PATH          "LastFilePath"

// Import
#define IMPORT_REPOSITION_TYPE      "ImportRepositionType"

// Settings Group/Key Name
#define PENCIL2D "Pencil"
#define SHORTCUTS_GROUP             "Shortcuts"
#define SETTING_AUTO_SAVE           "AutoSave"
#define SETTING_AUTO_SAVE_NUMBER    "AutosaveNumber"
#define SETTING_AUTO_SAVE_BY_TIME       "AutoSaveByTime"
#define SETTING_AUTO_SAVE_BY_TIME_TIMER "AutoSaveByTimeTimer"
#define SETTING_TOOL_CURSOR         "ToolCursors"
#define SETTING_CANVAS_CURSOR       "DottedCursors"
#define SETTING_HIGH_RESOLUTION     "HighResPosition"
#define SETTING_BACKGROUND_STYLE    "Background"
#define SETTING_WINDOW_OPACITY      "WindowOpacity"
#define SETTING_WINDOW_GEOMETRY     "WindowGeometry"
#define SETTING_WINDOW_STATE        "WindowState"
#define SETTING_SHOW_STATUS_BAR     "ShowStatusBar"
#define SETTING_CURVE_SMOOTHING     "CurveSmoothing"
#define SETTING_DISPLAY_EFFECT      "RenderEffect"
#define SETTING_SHORT_SCRUB         "ShortScrub"
#define SETTING_FPS                 "Fps"
#define SETTING_FIELD_W             "FieldW"
#define SETTING_FIELD_H             "FieldH"
#define SETTING_FRAME_SIZE          "FrameSize"
#define SETTING_TIMELINE_SIZE       "TimelineSize"
#define SETTING_LABEL_FONT_SIZE     "LabelFontSize"
#define SETTING_DRAW_LABEL          "DrawLabel"
#define SETTING_QUICK_SIZING        "QuickSizing"
#define SETTING_LAYOUT_LOCK         "LayoutLock"
#define SETTING_ROTATION_INCREMENT  "RotationIncrement"
#define SETTING_SHOW_SELECTION_INFO "ShowSelectionInfo"
#define SETTING_ASK_FOR_PRESET      "AskForPreset"
#define SETTING_LOAD_MOST_RECENT    "LoadMostRecent"
#define SETTING_LOAD_DEFAULT_PRESET "LoadDefaultPreset"
#define SETTING_DEFAULT_PRESET      "DefaultPreset"

#define SETTING_INVERT_DRAG_ZOOM_DIRECTION   "InvertDragZoomDirection"
#define SETTING_INVERT_SCROLL_ZOOM_DIRECTION "InvertScrollZoomDirection"

#define SETTING_ANTIALIAS        "Antialiasing"
#define SETTING_SHOW_GRID        "ShowGrid"
#define SETTING_COUNT            "Count"
#define SETTING_SHADOW           "Shadow"
#define SETTING_PREV_ONION       "PrevOnion"
#define SETTING_NEXT_ONION       "NextOnion"
#define SETTING_MULTILAYER_ONION "MultilayerOnion"
#define SETTING_AXIS             "Axis"
#define SETTING_CAMERABORDER     "CameraBorder"
#define SETTING_INVISIBLE_LINES  "InvisibleLines"
#define SETTING_OUTLINES         "Outlines"
#define SETTING_ONION_BLUE       "OnionBlue"
#define SETTING_ONION_RED        "OnionRed"

#define SETTING_FRAME_POOL_SIZE  "FramePoolSizeInMB"
#define SETTING_GRID_SIZE_W      "GridSizeW"
#define SETTING_GRID_SIZE_H      "GridSizeH"
#define SETTING_OVERLAY_CENTER   "OverlayCenter"
#define SETTING_OVERLAY_THIRDS   "OverlayThirds"
#define SETTING_OVERLAY_GOLDEN   "OverlayGolden"
#define SETTING_OVERLAY_SAFE     "OverlaySafe"
#define SETTING_OVERLAY_PERSPECTIVE1 "OverlayPerspective1"
#define SETTING_OVERLAY_PERSPECTIVE2 "OverlayPerspective2"
#define SETTING_OVERLAY_PERSPECTIVE3 "OverlayPerspective3"
#define SETTING_OVERLAY_ANGLE    "OverlayAngle"
#define SETTING_TITLE_SAFE_ON    "TitleSafeOn"
#define SETTING_TITLE_SAFE       "TitleSafe"
#define SETTING_ACTION_SAFE_ON   "ActionSafeOn"
#define SETTING_ACTION_SAFE      "ActionSafe"
#define SETTING_OVERLAY_SAFE_HELPER_TEXT_ON "OverlaySafeHelperTextOn"
#define SETTING_TIMECODE_TEXT    "TimecodeText"

#define SETTING_ONION_MAX_OPACITY       "OnionMaxOpacity"
#define SETTING_ONION_MIN_OPACITY       "OnionMinOpacity"
#define SETTING_ONION_PREV_FRAMES_NUM   "OnionPrevFramesNum"
#define SETTING_ONION_NEXT_FRAMES_NUM   "OnionNextFramesNum"
#define SETTING_ONION_WHILE_PLAYBACK    "OnionWhilePlayback"
#define SETTING_ONION_TYPE              "OnionType"
#define SETTING_FLIP_ROLL_MSEC          "FlipRoll"
#define SETTING_FLIP_ROLL_DRAWINGS      "FlipRollDrawings"
#define SETTING_FLIP_INBETWEEN_MSEC     "FlipInbetween"
#define SETTING_SOUND_SCRUB_ACTIVE      "SoundScrubActive"
#define SETTING_SOUND_SCRUB_MSEC        "SoundScrubMsec"
#define SETTING_NEW_UNDO_REDO_ON        "NewUndoRedoOn"
#define SETTING_UNDO_REDO_MAX_STEPS     "UndoRedoMaxSteps"

#define SETTING_LAYER_VISIBILITY "LayerVisibility"
#define SETTING_LAYER_VISIBILITY_THRESHOLD "LayerVisibilityThreshold"

#define SETTING_DRAW_ON_EMPTY_FRAME_ACTION  "DrawOnEmptyFrameAction"

#define SETTING_LANGUAGE        "Language"

#endif // PENCILDEF_H
