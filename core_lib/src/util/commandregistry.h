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

#ifndef COMMANDREGISTRY_H
#define COMMANDREGISTRY_H

#include <QHash>
#include <QVector>
#include <QString>
#include <QCoreApplication>

enum class CommandCategory : int
{
    File,
    Edit,
    View,
    Animation,
    Tool,
    Layer,
    Window,
    Help
};

struct CommandDefinition
{
    const char* id;              // e.g. "CmdNewFile"
    const char* defaultShortcut; // e.g. "Ctrl+N"
    const char* displayName;     // QT_TR_NOOP string for translation
    CommandCategory category;
};

class CommandRegistry
{
    Q_DECLARE_TR_FUNCTIONS(CommandRegistry)

public:
    static const CommandRegistry& instance();

    const CommandDefinition* find(const QString& cmdId) const;
    QVector<const CommandDefinition*> byCategory(CommandCategory cat) const;
    QVector<const CommandDefinition*> all() const;
    int count() const;

private:
    CommandRegistry();
    void populate();

    QHash<QString, const CommandDefinition*> mLookup;
    QVector<const CommandDefinition*> mAll;

    static const CommandDefinition kDefinitions[];
    static const int kDefinitionCount;
};

// Command ID constants — the single source of truth for command string identifiers.
// Use these instead of raw string literals for compile-time typo detection.
namespace CmdId
{
    // File
    inline constexpr const char* NewFile              = "CmdNewFile";
    inline constexpr const char* OpenFile             = "CmdOpenFile";
    inline constexpr const char* SaveFile             = "CmdSaveFile";
    inline constexpr const char* SaveAs               = "CmdSaveAs";
    inline constexpr const char* ImportImage          = "CmdImportImage";
    inline constexpr const char* ImportImageSequence  = "CmdImportImageSequence";
    inline constexpr const char* ImportImagePredefinedSet = "CmdImportImagePredefinedSet";
    inline constexpr const char* ImportMovieVideo     = "CmdImportMovieVideo";
    inline constexpr const char* ImportAnimatedImage  = "CmdImportAnimatedImage";
    inline constexpr const char* ImportLayers         = "CmdImportLayers";
    inline constexpr const char* ImportSound          = "CmdImportSound";
    inline constexpr const char* ImportMovieAudio     = "CmdImportMovieAudio";
    inline constexpr const char* ImportPalette        = "CmdImportPalette";
    inline constexpr const char* ImportPaletteReplace = "CmdImportPaletteReplace";
    inline constexpr const char* ExportImage          = "CmdExportImage";
    inline constexpr const char* ExportImageSequence  = "CmdExportImageSequence";
    inline constexpr const char* ExportMovie          = "CmdExportMovie";
    inline constexpr const char* ExportGIF            = "CmdExportGIF";
    inline constexpr const char* ExportPalette        = "CmdExportPalette";

    // Edit
    inline constexpr const char* Undo                 = "CmdUndo";
    inline constexpr const char* Redo                 = "CmdRedo";
    inline constexpr const char* Cut                  = "CmdCut";
    inline constexpr const char* Copy                 = "CmdCopy";
    inline constexpr const char* Paste                = "CmdPaste";
    inline constexpr const char* PasteFromPrevious    = "CmdPasteFromPrevious";
    inline constexpr const char* SelectAll            = "CmdSelectAll";
    inline constexpr const char* DeselectAll          = "CmdDeselectAll";
    inline constexpr const char* ClearFrame           = "CmdClearFrame";
    inline constexpr const char* SelectionFlipH       = "CmdSelectionFlipHorizontal";
    inline constexpr const char* SelectionFlipV       = "CmdSelectionFlipVertical";
    inline constexpr const char* PegBarAlignment      = "CmdPegBarAlignment";
    inline constexpr const char* Preferences          = "CmdPreferences";

    // View
    inline constexpr const char* ResetWindows         = "CmdResetWindows";
    inline constexpr const char* LockWindows          = "CmdLockWindows";
    inline constexpr const char* ZoomIn               = "CmdZoomIn";
    inline constexpr const char* ZoomOut              = "CmdZoomOut";
    inline constexpr const char* Zoom400              = "CmdZoom400";
    inline constexpr const char* Zoom300              = "CmdZoom300";
    inline constexpr const char* Zoom200              = "CmdZoom200";
    inline constexpr const char* Zoom100              = "CmdZoom100";
    inline constexpr const char* Zoom50               = "CmdZoom50";
    inline constexpr const char* Zoom33               = "CmdZoom33";
    inline constexpr const char* Zoom25               = "CmdZoom25";
    inline constexpr const char* RotateClockwise      = "CmdRotateClockwise";
    inline constexpr const char* RotateAntiClockwise  = "CmdRotateAntiClockwise";
    inline constexpr const char* ResetRotation        = "CmdResetRotation";
    inline constexpr const char* ResetZoomRotate      = "CmdResetZoomRotate";
    inline constexpr const char* CenterView           = "CmdCenterView";
    inline constexpr const char* FlipHorizontal       = "CmdFlipHorizontal";
    inline constexpr const char* FlipVertical         = "CmdFlipVertical";
    inline constexpr const char* Grid                 = "CmdGrid";
    inline constexpr const char* OverlayCenter        = "CmdOverlayCenter";
    inline constexpr const char* OverlayThirds        = "CmdOverlayThirds";
    inline constexpr const char* OverlayGoldenRatio   = "CmdOverlayGoldenRatio";
    inline constexpr const char* OverlaySafeAreas     = "CmdOverlaySafeAreas";
    inline constexpr const char* OverlayPerspective1  = "CmdOverlayOnePointPerspective";
    inline constexpr const char* OverlayPerspective2  = "CmdOverlayTwoPointPerspective";
    inline constexpr const char* OverlayPerspective3  = "CmdOverlayThreePointPerspective";
    inline constexpr const char* OnionSkinPrev        = "CmdOnionSkinPrevious";
    inline constexpr const char* OnionSkinNext        = "CmdOnionSkinNext";
    inline constexpr const char* ToggleStatusBar      = "CmdToggleStatusBar";

    // Animation
    inline constexpr const char* Play                 = "CmdPlay";
    inline constexpr const char* Loop                 = "CmdLoop";
    inline constexpr const char* LoopControl          = "CmdLoopControl";
    inline constexpr const char* FlipInBetween        = "CmdFlipInBetween";
    inline constexpr const char* FlipRolling          = "CmdFlipRolling";
    inline constexpr const char* GotoNextFrame        = "CmdGotoNextFrame";
    inline constexpr const char* GotoPrevFrame        = "CmdGotoPreviousFrame";
    inline constexpr const char* GotoNextKeyFrame     = "CmdGotoNextKeyFrame";
    inline constexpr const char* GotoPrevKeyFrame     = "CmdGotoPreviousKeyFrame";
    inline constexpr const char* AddFrame             = "CmdAddFrame";
    inline constexpr const char* DuplicateFrame       = "CmdDuplicateFrame";
    inline constexpr const char* RemoveFrame          = "CmdRemoveFrame";
    inline constexpr const char* ReverseSelectedFrames = "CmdReverseSelectedFrames";
    inline constexpr const char* RemoveSelectedFrames = "CmdRemoveSelectedFrames";
    inline constexpr const char* RepositionFrames     = "CmdSelectionRepositionFrames";
    inline constexpr const char* AddFrameExposure     = "CmdSelectionAddFrameExposure";
    inline constexpr const char* SubtractFrameExposure = "CmdSelectionSubtractFrameExposure";
    inline constexpr const char* MoveFrameBackward    = "CmdMoveFrameBackward";
    inline constexpr const char* MoveFrameForward     = "CmdMoveFrameForward";

    // Tools
    inline constexpr const char* ToolMove             = "CmdToolMove";
    inline constexpr const char* ToolSelect           = "CmdToolSelect";
    inline constexpr const char* ToolBrush            = "CmdToolBrush";
    inline constexpr const char* ToolPolyline         = "CmdToolPolyline";
    inline constexpr const char* ToolSmudge           = "CmdToolSmudge";
    inline constexpr const char* ToolPen              = "CmdToolPen";
    inline constexpr const char* ToolHand             = "CmdToolHand";
    inline constexpr const char* ToolPencil           = "CmdToolPencil";
    inline constexpr const char* ToolBucket           = "CmdToolBucket";
    inline constexpr const char* ToolEyedropper       = "CmdToolEyedropper";
    inline constexpr const char* ToolEraser           = "CmdToolEraser";
    inline constexpr const char* ResetAllTools        = "CmdResetAllTools";

    // Layer
    inline constexpr const char* NewBitmapLayer       = "CmdNewBitmapLayer";
    inline constexpr const char* NewVectorLayer       = "CmdNewVectorLayer";
    inline constexpr const char* NewSoundLayer        = "CmdNewSoundLayer";
    inline constexpr const char* NewCameraLayer       = "CmdNewCameraLayer";
    inline constexpr const char* DeleteCurrentLayer   = "CmdDeleteCurrentLayer";
    inline constexpr const char* ChangeLineColorKeyframe = "CmdChangeLineColorKeyframe";
    inline constexpr const char* ChangeLineColorLayer = "CmdChangeLineColorLayer";
    inline constexpr const char* ChangeLayerOpacity   = "CmdChangeLayerOpacity";
    inline constexpr const char* VisibilityCurrentOnly = "CmdLayerVisibilityCurrentOnly";
    inline constexpr const char* VisibilityRelative   = "CmdLayerVisibilityRelative";
    inline constexpr const char* VisibilityAll        = "CmdLayerVisibilityAll";

    // Window
    inline constexpr const char* ToggleToolBox        = "CmdToggleToolBox";
    inline constexpr const char* ToggleToolOptions    = "CmdToggleToolOptions";
    inline constexpr const char* ToggleColorWheel     = "CmdToggleColorWheel";
    inline constexpr const char* ToggleColorInspector = "CmdToggleColorInspector";
    inline constexpr const char* ToggleColorLibrary   = "CmdToggleColorLibrary";
    inline constexpr const char* ToggleOnionSkin      = "CmdToggleOnionSkin";
    inline constexpr const char* ToggleTimeline       = "CmdToggleTimeline";

    // Help
    inline constexpr const char* Help                 = "CmdHelp";
    inline constexpr const char* Exit                 = "CmdExit";
}

#endif // COMMANDREGISTRY_H
