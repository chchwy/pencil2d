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

TEST_CASE("Every CmdId constant is in the registry")
{
    const CommandRegistry& reg = CommandRegistry::instance();

    // File
    REQUIRE(reg.find(CmdId::NewFile) != nullptr);
    REQUIRE(reg.find(CmdId::OpenFile) != nullptr);
    REQUIRE(reg.find(CmdId::SaveFile) != nullptr);
    REQUIRE(reg.find(CmdId::SaveAs) != nullptr);
    REQUIRE(reg.find(CmdId::ImportImage) != nullptr);
    REQUIRE(reg.find(CmdId::ImportImageSequence) != nullptr);
    REQUIRE(reg.find(CmdId::ImportImagePredefinedSet) != nullptr);
    REQUIRE(reg.find(CmdId::ImportMovieVideo) != nullptr);
    REQUIRE(reg.find(CmdId::ImportAnimatedImage) != nullptr);
    REQUIRE(reg.find(CmdId::ImportLayers) != nullptr);
    REQUIRE(reg.find(CmdId::ImportSound) != nullptr);
    REQUIRE(reg.find(CmdId::ImportMovieAudio) != nullptr);
    REQUIRE(reg.find(CmdId::ImportPalette) != nullptr);
    REQUIRE(reg.find(CmdId::ImportPaletteReplace) != nullptr);
    REQUIRE(reg.find(CmdId::ExportImage) != nullptr);
    REQUIRE(reg.find(CmdId::ExportImageSequence) != nullptr);
    REQUIRE(reg.find(CmdId::ExportMovie) != nullptr);
    REQUIRE(reg.find(CmdId::ExportGIF) != nullptr);
    REQUIRE(reg.find(CmdId::ExportPalette) != nullptr);

    // Edit
    REQUIRE(reg.find(CmdId::Undo) != nullptr);
    REQUIRE(reg.find(CmdId::Redo) != nullptr);
    REQUIRE(reg.find(CmdId::Cut) != nullptr);
    REQUIRE(reg.find(CmdId::Copy) != nullptr);
    REQUIRE(reg.find(CmdId::Paste) != nullptr);
    REQUIRE(reg.find(CmdId::PasteFromPrevious) != nullptr);
    REQUIRE(reg.find(CmdId::SelectAll) != nullptr);
    REQUIRE(reg.find(CmdId::DeselectAll) != nullptr);
    REQUIRE(reg.find(CmdId::ClearFrame) != nullptr);
    REQUIRE(reg.find(CmdId::SelectionFlipH) != nullptr);
    REQUIRE(reg.find(CmdId::SelectionFlipV) != nullptr);
    REQUIRE(reg.find(CmdId::PegBarAlignment) != nullptr);
    REQUIRE(reg.find(CmdId::Preferences) != nullptr);

    // View
    REQUIRE(reg.find(CmdId::ResetWindows) != nullptr);
    REQUIRE(reg.find(CmdId::LockWindows) != nullptr);
    REQUIRE(reg.find(CmdId::ZoomIn) != nullptr);
    REQUIRE(reg.find(CmdId::ZoomOut) != nullptr);
    REQUIRE(reg.find(CmdId::Zoom400) != nullptr);
    REQUIRE(reg.find(CmdId::Zoom300) != nullptr);
    REQUIRE(reg.find(CmdId::Zoom200) != nullptr);
    REQUIRE(reg.find(CmdId::Zoom100) != nullptr);
    REQUIRE(reg.find(CmdId::Zoom50) != nullptr);
    REQUIRE(reg.find(CmdId::Zoom33) != nullptr);
    REQUIRE(reg.find(CmdId::Zoom25) != nullptr);
    REQUIRE(reg.find(CmdId::RotateClockwise) != nullptr);
    REQUIRE(reg.find(CmdId::RotateAntiClockwise) != nullptr);
    REQUIRE(reg.find(CmdId::ResetRotation) != nullptr);
    REQUIRE(reg.find(CmdId::ResetZoomRotate) != nullptr);
    REQUIRE(reg.find(CmdId::CenterView) != nullptr);
    REQUIRE(reg.find(CmdId::FlipHorizontal) != nullptr);
    REQUIRE(reg.find(CmdId::FlipVertical) != nullptr);
    REQUIRE(reg.find(CmdId::Grid) != nullptr);
    REQUIRE(reg.find(CmdId::OverlayCenter) != nullptr);
    REQUIRE(reg.find(CmdId::OverlayThirds) != nullptr);
    REQUIRE(reg.find(CmdId::OverlayGoldenRatio) != nullptr);
    REQUIRE(reg.find(CmdId::OverlaySafeAreas) != nullptr);
    REQUIRE(reg.find(CmdId::OverlayPerspective1) != nullptr);
    REQUIRE(reg.find(CmdId::OverlayPerspective2) != nullptr);
    REQUIRE(reg.find(CmdId::OverlayPerspective3) != nullptr);
    REQUIRE(reg.find(CmdId::OnionSkinPrev) != nullptr);
    REQUIRE(reg.find(CmdId::OnionSkinNext) != nullptr);
    REQUIRE(reg.find(CmdId::ToggleStatusBar) != nullptr);

    // Animation
    REQUIRE(reg.find(CmdId::Play) != nullptr);
    REQUIRE(reg.find(CmdId::Loop) != nullptr);
    REQUIRE(reg.find(CmdId::LoopControl) != nullptr);
    REQUIRE(reg.find(CmdId::GotoNextFrame) != nullptr);
    REQUIRE(reg.find(CmdId::GotoPrevFrame) != nullptr);
    REQUIRE(reg.find(CmdId::GotoNextKeyFrame) != nullptr);
    REQUIRE(reg.find(CmdId::GotoPrevKeyFrame) != nullptr);
    REQUIRE(reg.find(CmdId::AddFrame) != nullptr);
    REQUIRE(reg.find(CmdId::DuplicateFrame) != nullptr);
    REQUIRE(reg.find(CmdId::RemoveFrame) != nullptr);
    REQUIRE(reg.find(CmdId::AddFrameExposure) != nullptr);
    REQUIRE(reg.find(CmdId::SubtractFrameExposure) != nullptr);
    REQUIRE(reg.find(CmdId::RepositionFrames) != nullptr);
    REQUIRE(reg.find(CmdId::ReverseSelectedFrames) != nullptr);
    REQUIRE(reg.find(CmdId::RemoveSelectedFrames) != nullptr);
    REQUIRE(reg.find(CmdId::MoveFrameForward) != nullptr);
    REQUIRE(reg.find(CmdId::MoveFrameBackward) != nullptr);
    REQUIRE(reg.find(CmdId::FlipInBetween) != nullptr);
    REQUIRE(reg.find(CmdId::FlipRolling) != nullptr);

    // Tools
    REQUIRE(reg.find(CmdId::ToolMove) != nullptr);
    REQUIRE(reg.find(CmdId::ToolSelect) != nullptr);
    REQUIRE(reg.find(CmdId::ToolBrush) != nullptr);
    REQUIRE(reg.find(CmdId::ToolPolyline) != nullptr);
    REQUIRE(reg.find(CmdId::ToolSmudge) != nullptr);
    REQUIRE(reg.find(CmdId::ToolPen) != nullptr);
    REQUIRE(reg.find(CmdId::ToolHand) != nullptr);
    REQUIRE(reg.find(CmdId::ToolPencil) != nullptr);
    REQUIRE(reg.find(CmdId::ToolBucket) != nullptr);
    REQUIRE(reg.find(CmdId::ToolEyedropper) != nullptr);
    REQUIRE(reg.find(CmdId::ToolEraser) != nullptr);
    REQUIRE(reg.find(CmdId::ResetAllTools) != nullptr);

    // Layer
    REQUIRE(reg.find(CmdId::NewBitmapLayer) != nullptr);
    REQUIRE(reg.find(CmdId::NewVectorLayer) != nullptr);
    REQUIRE(reg.find(CmdId::NewSoundLayer) != nullptr);
    REQUIRE(reg.find(CmdId::NewCameraLayer) != nullptr);
    REQUIRE(reg.find(CmdId::DeleteCurrentLayer) != nullptr);
    REQUIRE(reg.find(CmdId::ChangeLineColorKeyframe) != nullptr);
    REQUIRE(reg.find(CmdId::ChangeLineColorLayer) != nullptr);
    REQUIRE(reg.find(CmdId::ChangeLayerOpacity) != nullptr);
    REQUIRE(reg.find(CmdId::VisibilityCurrentOnly) != nullptr);
    REQUIRE(reg.find(CmdId::VisibilityRelative) != nullptr);
    REQUIRE(reg.find(CmdId::VisibilityAll) != nullptr);

    // Window
    REQUIRE(reg.find(CmdId::ToggleToolBox) != nullptr);
    REQUIRE(reg.find(CmdId::ToggleToolOptions) != nullptr);
    REQUIRE(reg.find(CmdId::ToggleColorWheel) != nullptr);
    REQUIRE(reg.find(CmdId::ToggleColorLibrary) != nullptr);
    REQUIRE(reg.find(CmdId::ToggleTimeline) != nullptr);
    REQUIRE(reg.find(CmdId::ToggleColorInspector) != nullptr);
    REQUIRE(reg.find(CmdId::ToggleOnionSkin) != nullptr);

    // Help
    REQUIRE(reg.find(CmdId::Help) != nullptr);
    REQUIRE(reg.find(CmdId::Exit) != nullptr);
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
        REQUIRE(QString::fromLatin1(reg.find(CmdId::Undo)->defaultShortcut) == "Ctrl+Z");
        REQUIRE(QString::fromLatin1(reg.find(CmdId::Redo)->defaultShortcut) == "Ctrl+Shift+Z");
        REQUIRE(QString::fromLatin1(reg.find(CmdId::Cut)->defaultShortcut) == "Ctrl+X");
        REQUIRE(QString::fromLatin1(reg.find(CmdId::Copy)->defaultShortcut) == "Ctrl+C");
        REQUIRE(QString::fromLatin1(reg.find(CmdId::Paste)->defaultShortcut) == "Ctrl+V");
    }

    SECTION("Tool shortcuts")
    {
        REQUIRE(QString::fromLatin1(reg.find(CmdId::ToolBrush)->defaultShortcut) == "B");
        REQUIRE(QString::fromLatin1(reg.find(CmdId::ToolEraser)->defaultShortcut) == "E");
        REQUIRE(QString::fromLatin1(reg.find(CmdId::ToolPen)->defaultShortcut) == "P");
    }

    SECTION("Commands with no default shortcut")
    {
        REQUIRE(QString::fromLatin1(reg.find(CmdId::ClearFrame)->defaultShortcut) == "");
        REQUIRE(QString::fromLatin1(reg.find(CmdId::ImportImage)->defaultShortcut) == "");
    }
}
