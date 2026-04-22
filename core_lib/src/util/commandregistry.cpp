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

// Single source of truth: every command's ID, default shortcut, display name, and category.
// Adding a new command = adding one line here + one constexpr in CmdId namespace.
const CommandDefinition CommandRegistry::kDefinitions[] =
{
    // --- File ---
    { CmdId::NewFile,                "Ctrl+N",       QT_TR_NOOP("New File"),                    CommandCategory::File },
    { CmdId::OpenFile,               "Ctrl+O",       QT_TR_NOOP("Open File"),                   CommandCategory::File },
    { CmdId::SaveFile,               "Ctrl+S",       QT_TR_NOOP("Save File"),                   CommandCategory::File },
    { CmdId::SaveAs,                 "Ctrl+Shift+S", QT_TR_NOOP("Save File As"),                CommandCategory::File },
    { CmdId::ImportImage,            "",              QT_TR_NOOP("Import Image"),                CommandCategory::File },
    { CmdId::ImportImageSequence,    "",              QT_TR_NOOP("Import Image Sequence"),       CommandCategory::File },
    { CmdId::ImportImagePredefinedSet, "",            QT_TR_NOOP("Import Image Predefined Set"), CommandCategory::File },
    { CmdId::ImportMovieVideo,       "",              QT_TR_NOOP("Import Movie Video"),          CommandCategory::File },
    { CmdId::ImportAnimatedImage,    "",              QT_TR_NOOP("Import Animated Image"),       CommandCategory::File },
    { CmdId::ImportLayers,           "",              QT_TR_NOOP("Import Layers from project file"), CommandCategory::File },
    { CmdId::ImportSound,            "",              QT_TR_NOOP("Import Sound"),                CommandCategory::File },
    { CmdId::ImportMovieAudio,       "",              QT_TR_NOOP("Import Movie Audio"),          CommandCategory::File },
    { CmdId::ImportPalette,          "",              QT_TR_NOOP("Import Palette (Append)"),     CommandCategory::File },
    { CmdId::ImportPaletteReplace,   "",              QT_TR_NOOP("Import Palette (Replace)"),    CommandCategory::File },
    { CmdId::ExportImage,            "Ctrl+Shift+R", QT_TR_NOOP("Export Image"),                CommandCategory::File },
    { CmdId::ExportImageSequence,    "Ctrl+R",       QT_TR_NOOP("Export Image Sequence"),       CommandCategory::File },
    { CmdId::ExportMovie,            "",              QT_TR_NOOP("Export Movie"),                CommandCategory::File },
    { CmdId::ExportGIF,              "Ctrl+G",       QT_TR_NOOP("Export Animated GIF"),         CommandCategory::File },
    { CmdId::ExportPalette,          "",              QT_TR_NOOP("Export Palette"),              CommandCategory::File },

    // --- Edit ---
    { CmdId::Undo,                   "Ctrl+Z",       QT_TR_NOOP("Undo"),                       CommandCategory::Edit },
    { CmdId::Redo,                   "Ctrl+Shift+Z", QT_TR_NOOP("Redo"),                       CommandCategory::Edit },
    { CmdId::Cut,                    "Ctrl+X",       QT_TR_NOOP("Cut"),                        CommandCategory::Edit },
    { CmdId::Copy,                   "Ctrl+C",       QT_TR_NOOP("Copy"),                       CommandCategory::Edit },
    { CmdId::Paste,                  "Ctrl+V",       QT_TR_NOOP("Paste"),                      CommandCategory::Edit },
    { CmdId::PasteFromPrevious,      "Ctrl+Left",    QT_TR_NOOP("Paste from Previous Keyframe"), CommandCategory::Edit },
    { CmdId::SelectAll,              "Ctrl+A",       QT_TR_NOOP("Select All"),                 CommandCategory::Edit },
    { CmdId::DeselectAll,            "Ctrl+D",       QT_TR_NOOP("Deselect All"),               CommandCategory::Edit },
    { CmdId::ClearFrame,             "",              QT_TR_NOOP("Clear Frame"),                CommandCategory::Edit },
    { CmdId::SelectionFlipH,         "",              QT_TR_NOOP("Selection: Horizontal Flip"), CommandCategory::Edit },
    { CmdId::SelectionFlipV,         "",              QT_TR_NOOP("Selection: Vertical Flip"),   CommandCategory::Edit },
    { CmdId::PegBarAlignment,        "",              QT_TR_NOOP("Peg bar Alignment"),          CommandCategory::Edit },
    { CmdId::Preferences,            "",              QT_TR_NOOP("Preferences"),                CommandCategory::Edit },

    // --- View ---
    { CmdId::ResetWindows,           "",              QT_TR_NOOP("Reset Windows"),              CommandCategory::View },
    { CmdId::LockWindows,            "",              QT_TR_NOOP("Lock Windows"),               CommandCategory::View },
    { CmdId::ZoomIn,                 "Ctrl+Up",      QT_TR_NOOP("Zoom In"),                    CommandCategory::View },
    { CmdId::ZoomOut,                "Ctrl+Down",    QT_TR_NOOP("Zoom Out"),                   CommandCategory::View },
    { CmdId::Zoom400,                "4",            QT_TR_NOOP("Set Zoom to 400%"),           CommandCategory::View },
    { CmdId::Zoom300,                "3",            QT_TR_NOOP("Set Zoom to 300%"),           CommandCategory::View },
    { CmdId::Zoom200,                "2",            QT_TR_NOOP("Set Zoom to 200%"),           CommandCategory::View },
    { CmdId::Zoom100,                "1",            QT_TR_NOOP("Set Zoom to 100%"),           CommandCategory::View },
    { CmdId::Zoom50,                 "Shift+2",      QT_TR_NOOP("Set Zoom to 50%"),            CommandCategory::View },
    { CmdId::Zoom33,                 "Shift+3",      QT_TR_NOOP("Set Zoom to 33%"),            CommandCategory::View },
    { CmdId::Zoom25,                 "Shift+4",      QT_TR_NOOP("Set Zoom to 25%"),            CommandCategory::View },
    { CmdId::RotateClockwise,        "R",            QT_TR_NOOP("Rotate Clockwise"),           CommandCategory::View },
    { CmdId::RotateAntiClockwise,    "Z",            QT_TR_NOOP("Rotate Anticlockwise"),       CommandCategory::View },
    { CmdId::ResetRotation,          "Alt+H",        QT_TR_NOOP("Reset Rotation"),             CommandCategory::View },
    { CmdId::ResetZoomRotate,        "Ctrl+H",       QT_TR_NOOP("Reset View"),                 CommandCategory::View },
    { CmdId::CenterView,             "Ctrl+Shift+H", QT_TR_NOOP("Center View"),                CommandCategory::View },
    { CmdId::FlipHorizontal,         "Shift+H",      QT_TR_NOOP("View: Horizontal Flip"),      CommandCategory::View },
    { CmdId::FlipVertical,           "Shift+V",      QT_TR_NOOP("View: Vertical Flip"),        CommandCategory::View },
    { CmdId::Grid,                   "G",            QT_TR_NOOP("Toggle Grid"),                CommandCategory::View },
    { CmdId::OverlayCenter,          "",              QT_TR_NOOP("Toggle Center Overlay"),      CommandCategory::View },
    { CmdId::OverlayThirds,          "",              QT_TR_NOOP("Toggle Thirds Overlay"),      CommandCategory::View },
    { CmdId::OverlayGoldenRatio,     "",              QT_TR_NOOP("Toggle Golden Ratio Overlay"), CommandCategory::View },
    { CmdId::OverlaySafeAreas,       "",              QT_TR_NOOP("Toggle Safe Areas Overlay"),  CommandCategory::View },
    { CmdId::OverlayPerspective1,    "",              QT_TR_NOOP("Toggle One Point Perspective Overlay"), CommandCategory::View },
    { CmdId::OverlayPerspective2,    "",              QT_TR_NOOP("Toggle Two Point Perspective Overlay"), CommandCategory::View },
    { CmdId::OverlayPerspective3,    "",              QT_TR_NOOP("Toggle Three Point Perspective Overlay"), CommandCategory::View },
    { CmdId::OnionSkinPrev,          "O",            QT_TR_NOOP("Toggle Previous Onion Skin"), CommandCategory::View },
    { CmdId::OnionSkinNext,          "Alt+O",        QT_TR_NOOP("Toggle Next Onion Skin"),     CommandCategory::View },
    { CmdId::ToggleStatusBar,        "",              QT_TR_NOOP("Toggle Status Bar Visibility"), CommandCategory::View },

    // --- Animation ---
    { CmdId::Play,                   "Ctrl+Return",  QT_TR_NOOP("Play/Stop"),                  CommandCategory::Animation },
    { CmdId::Loop,                   "Ctrl+L",       QT_TR_NOOP("Toggle Loop"),                CommandCategory::Animation },
    { CmdId::LoopControl,            "",              QT_TR_NOOP("Toggle Range Playback"),      CommandCategory::Animation },
    { CmdId::GotoNextFrame,          ".",             QT_TR_NOOP("Next Frame"),                 CommandCategory::Animation },
    { CmdId::GotoPrevFrame,          ",",             QT_TR_NOOP("Previous Frame"),             CommandCategory::Animation },
    { CmdId::GotoNextKeyFrame,       "Alt+.",         QT_TR_NOOP("Next Keyframe"),              CommandCategory::Animation },
    { CmdId::GotoPrevKeyFrame,       "Alt+,",         QT_TR_NOOP("Previous Keyframe"),          CommandCategory::Animation },
    { CmdId::AddFrame,               "F7",            QT_TR_NOOP("Add Frame"),                  CommandCategory::Animation },
    { CmdId::DuplicateFrame,         "F6",            QT_TR_NOOP("Duplicate Frame"),            CommandCategory::Animation },
    { CmdId::RemoveFrame,            "Shift+F5",      QT_TR_NOOP("Remove Frame"),               CommandCategory::Animation },
    { CmdId::AddFrameExposure,       "Ctrl++",        QT_TR_NOOP("Selection: Add Frame Exposure"), CommandCategory::Animation },
    { CmdId::SubtractFrameExposure,  "Ctrl+-",        QT_TR_NOOP("Selection: Subtract Frame Exposure"), CommandCategory::Animation },
    { CmdId::RepositionFrames,       "",              QT_TR_NOOP("Selection: Reposition Frames"), CommandCategory::Animation },
    { CmdId::ReverseSelectedFrames,  "",              QT_TR_NOOP("Selection: Reverse Keyframes"), CommandCategory::Animation },
    { CmdId::RemoveSelectedFrames,   "",              QT_TR_NOOP("Selection: Remove Keyframes"), CommandCategory::Animation },
    { CmdId::MoveFrameForward,       "Ctrl+.",        QT_TR_NOOP("Move Frame Forward"),         CommandCategory::Animation },
    { CmdId::MoveFrameBackward,      "Ctrl+,",        QT_TR_NOOP("Move Frame Backward"),        CommandCategory::Animation },
    { CmdId::FlipInBetween,          "Alt+Z",         QT_TR_NOOP("Flip In-Between"),            CommandCategory::Animation },
    { CmdId::FlipRolling,            "Alt+X",         QT_TR_NOOP("Flip Rolling"),               CommandCategory::Animation },

    // --- Tools ---
    { CmdId::ToolMove,               "M",            QT_TR_NOOP("Move Tool"),                  CommandCategory::Tool },
    { CmdId::ToolSelect,             "V",            QT_TR_NOOP("Select Tool"),                CommandCategory::Tool },
    { CmdId::ToolBrush,              "B",            QT_TR_NOOP("Brush Tool"),                 CommandCategory::Tool },
    { CmdId::ToolPolyline,           "Y",            QT_TR_NOOP("Polyline Tool"),              CommandCategory::Tool },
    { CmdId::ToolSmudge,             "A",            QT_TR_NOOP("Smudge Tool"),                CommandCategory::Tool },
    { CmdId::ToolPen,                "P",            QT_TR_NOOP("Pen Tool"),                   CommandCategory::Tool },
    { CmdId::ToolHand,               "H",            QT_TR_NOOP("Hand Tool"),                  CommandCategory::Tool },
    { CmdId::ToolPencil,             "N",            QT_TR_NOOP("Pencil Tool"),                CommandCategory::Tool },
    { CmdId::ToolBucket,             "K",            QT_TR_NOOP("Bucket Tool"),                CommandCategory::Tool },
    { CmdId::ToolEyedropper,         "I",            QT_TR_NOOP("Eyedropper Tool"),            CommandCategory::Tool },
    { CmdId::ToolEraser,             "E",            QT_TR_NOOP("Eraser Tool"),                CommandCategory::Tool },
    { CmdId::ResetAllTools,          "",              QT_TR_NOOP("Reset all tools to default"), CommandCategory::Tool },

    // --- Layer ---
    { CmdId::NewBitmapLayer,         "Ctrl+Alt+B",   QT_TR_NOOP("New Bitmap Layer"),           CommandCategory::Layer },
    { CmdId::NewVectorLayer,         "Ctrl+Alt+V",   QT_TR_NOOP("New Vector Layer"),           CommandCategory::Layer },
    { CmdId::NewSoundLayer,          "Ctrl+Alt+W",   QT_TR_NOOP("New Sound Layer"),            CommandCategory::Layer },
    { CmdId::NewCameraLayer,         "Ctrl+Alt+C",   QT_TR_NOOP("New Camera Layer"),           CommandCategory::Layer },
    { CmdId::DeleteCurrentLayer,     "",              QT_TR_NOOP("Delete Current Layer"),       CommandCategory::Layer },
    { CmdId::ChangeLineColorKeyframe, "",             QT_TR_NOOP("Change Line Color (Current keyframe)"), CommandCategory::Layer },
    { CmdId::ChangeLineColorLayer,   "",              QT_TR_NOOP("Change Line Color (All keyframes on layer)"), CommandCategory::Layer },
    { CmdId::ChangeLayerOpacity,     "",              QT_TR_NOOP("Change Layer / Keyframe Opacity"), CommandCategory::Layer },
    { CmdId::VisibilityCurrentOnly,  "Alt+1",        QT_TR_NOOP("Show Current Layer Only"),    CommandCategory::Layer },
    { CmdId::VisibilityRelative,     "Alt+2",        QT_TR_NOOP("Show Layers Relative to Current Layer"), CommandCategory::Layer },
    { CmdId::VisibilityAll,          "Alt+3",        QT_TR_NOOP("Show All Layers"),            CommandCategory::Layer },

    // --- Window ---
    { CmdId::ToggleToolBox,          "Ctrl+1",       QT_TR_NOOP("Toggle Tools Window Visibility"), CommandCategory::Window },
    { CmdId::ToggleToolOptions,      "Ctrl+2",       QT_TR_NOOP("Toggle Options Window Visibility"), CommandCategory::Window },
    { CmdId::ToggleColorWheel,       "Ctrl+3",       QT_TR_NOOP("Toggle Color Box Window Visibility"), CommandCategory::Window },
    { CmdId::ToggleColorLibrary,     "Ctrl+4",       QT_TR_NOOP("Toggle Color Palette Window Visibility"), CommandCategory::Window },
    { CmdId::ToggleTimeline,         "Ctrl+6",       QT_TR_NOOP("Toggle Timeline Window Visibility"), CommandCategory::Window },
    { CmdId::ToggleColorInspector,   "Ctrl+7",       QT_TR_NOOP("Toggle Color Inspector Window Visibility"), CommandCategory::Window },
    { CmdId::ToggleOnionSkin,        "Ctrl+8",       QT_TR_NOOP("Toggle Onion Skins Window Visibility"), CommandCategory::Window },

    // --- Help ---
    { CmdId::Help,                   "",              QT_TR_NOOP("Help"),                       CommandCategory::Help },
    { CmdId::Exit,                   "Ctrl+Q",       QT_TR_NOOP("Exit"),                       CommandCategory::Help },
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
