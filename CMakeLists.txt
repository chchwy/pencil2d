cmake_minimum_required(VERSION 3.21)
project(pencil2d VERSION 0.7.1 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 REQUIRED COMPONENTS Gui Multimedia Network Svg Xml)

qt_standard_project_setup()

qt_add_executable(pencil2d WIN32 MACOSX_BUNDLE
    app/src/app-pch.h
    app/src/aboutdialog.cpp app/src/aboutdialog.h
    app/src/actioncommands.cpp app/src/actioncommands.h
    app/src/app_util.cpp app/src/app_util.h
    app/src/basedockwidget.cpp app/src/basedockwidget.h
    app/src/bucketoptionswidget.cpp app/src/bucketoptionswidget.h
    app/src/cameracontextmenu.cpp app/src/cameracontextmenu.h
    app/src/cameraoptionswidget.cpp app/src/cameraoptionswidget.h
    app/src/camerapropertiesdialog.cpp app/src/camerapropertiesdialog.h
    app/src/checkupdatesdialog.cpp app/src/checkupdatesdialog.h
    app/src/colorbox.cpp app/src/colorbox.h
    app/src/colorinspector.cpp app/src/colorinspector.h
    app/src/colorpalettewidget.cpp app/src/colorpalettewidget.h
    app/src/colorslider.cpp app/src/colorslider.h
    app/src/colorwheel.cpp app/src/colorwheel.h
    app/src/commandlineexporter.cpp app/src/commandlineexporter.h
    app/src/commandlineparser.cpp app/src/commandlineparser.h
    app/src/doubleprogressdialog.cpp app/src/doubleprogressdialog.h
    app/src/elidedlabel.cpp app/src/elidedlabel.h
    app/src/errordialog.cpp app/src/errordialog.h
    app/src/exportimagedialog.cpp app/src/exportimagedialog.h
    app/src/exportmoviedialog.cpp app/src/exportmoviedialog.h
    app/src/filedialog.cpp app/src/filedialog.h
    app/src/filespage.cpp app/src/filespage.h
    app/src/generalpage.cpp app/src/generalpage.h
    app/src/importexportdialog.cpp app/src/importexportdialog.h
    app/src/importimageseqdialog.cpp app/src/importimageseqdialog.h
    app/src/importlayersdialog.cpp app/src/importlayersdialog.h
    app/src/importpositiondialog.cpp app/src/importpositiondialog.h
    app/src/layeropacitydialog.cpp app/src/layeropacitydialog.h
    app/src/main.cpp
    app/src/mainwindow2.cpp app/src/mainwindow2.h
    app/src/onionskinwidget.cpp app/src/onionskinwidget.h
    app/src/pegbaralignmentdialog.cpp app/src/pegbaralignmentdialog.h
    app/src/pencil2d.cpp app/src/pencil2d.h
    app/src/predefinedsetmodel.cpp app/src/predefinedsetmodel.h
    app/src/preferencesdialog.cpp app/src/preferencesdialog.h
    app/src/presetdialog.cpp app/src/presetdialog.h
    app/src/repositionframesdialog.cpp app/src/repositionframesdialog.h
    app/src/shortcutfilter.cpp app/src/shortcutfilter.h
    app/src/shortcutspage.cpp app/src/shortcutspage.h
    app/src/spinslider.cpp app/src/spinslider.h
    app/src/statusbar.cpp app/src/statusbar.h
    app/src/timecontrols.cpp app/src/timecontrols.h
    app/src/timeline.cpp app/src/timeline.h
    app/src/timelinecells.cpp app/src/timelinecells.h
    app/src/timelinepage.cpp app/src/timelinepage.h
    app/src/toolbox.cpp app/src/toolbox.h
    app/src/tooloptionwidget.cpp app/src/tooloptionwidget.h
    app/src/toolspage.cpp app/src/toolspage.h
    
   
    core_lib/src/activeframepool.cpp core_lib/src/activeframepool.h
    core_lib/src/camerapainter.cpp core_lib/src/camerapainter.h
    core_lib/src/canvascursorpainter.cpp core_lib/src/canvascursorpainter.h
    core_lib/src/canvaspainter.cpp core_lib/src/canvaspainter.h
    core_lib/src/corelib-pch.h
    core_lib/src/external/platformhandler.h
    core_lib/src/graphics/bitmap/bitmapbucket.cpp core_lib/src/graphics/bitmap/bitmapbucket.h
    core_lib/src/graphics/bitmap/bitmapimage.cpp core_lib/src/graphics/bitmap/bitmapimage.h
    core_lib/src/graphics/bitmap/tile.cpp core_lib/src/graphics/bitmap/tile.h
    core_lib/src/graphics/bitmap/tiledbuffer.cpp core_lib/src/graphics/bitmap/tiledbuffer.h
    core_lib/src/graphics/vector/bezierarea.cpp core_lib/src/graphics/vector/bezierarea.h
    core_lib/src/graphics/vector/beziercurve.cpp core_lib/src/graphics/vector/beziercurve.h
    core_lib/src/graphics/vector/colorref.cpp core_lib/src/graphics/vector/colorref.h
    core_lib/src/graphics/vector/vectorimage.cpp core_lib/src/graphics/vector/vectorimage.h
    core_lib/src/graphics/vector/vectorselection.cpp core_lib/src/graphics/vector/vectorselection.h
    core_lib/src/graphics/vector/vertexref.cpp core_lib/src/graphics/vector/vertexref.h
    core_lib/src/interface/backgroundwidget.cpp core_lib/src/interface/backgroundwidget.h
    core_lib/src/interface/editor.cpp core_lib/src/interface/editor.h
    core_lib/src/interface/flowlayout.cpp core_lib/src/interface/flowlayout.h
    core_lib/src/interface/legacybackupelement.cpp core_lib/src/interface/legacybackupelement.h
    core_lib/src/interface/recentfilemenu.cpp core_lib/src/interface/recentfilemenu.h
    core_lib/src/interface/scribblearea.cpp core_lib/src/interface/scribblearea.h
    core_lib/src/interface/undoredocommand.cpp core_lib/src/interface/undoredocommand.h
    core_lib/src/managers/basemanager.cpp core_lib/src/managers/basemanager.h
    core_lib/src/managers/clipboardmanager.cpp core_lib/src/managers/clipboardmanager.h
    core_lib/src/managers/colormanager.cpp core_lib/src/managers/colormanager.h
    core_lib/src/managers/layermanager.cpp core_lib/src/managers/layermanager.h
    core_lib/src/managers/overlaymanager.cpp core_lib/src/managers/overlaymanager.h
    core_lib/src/managers/playbackmanager.cpp core_lib/src/managers/playbackmanager.h
    core_lib/src/managers/preferencemanager.cpp core_lib/src/managers/preferencemanager.h
    core_lib/src/managers/selectionmanager.cpp core_lib/src/managers/selectionmanager.h
    core_lib/src/managers/soundmanager.cpp core_lib/src/managers/soundmanager.h
    core_lib/src/managers/toolmanager.cpp core_lib/src/managers/toolmanager.h
    core_lib/src/managers/undoredomanager.cpp core_lib/src/managers/undoredomanager.h
    core_lib/src/managers/viewmanager.cpp core_lib/src/managers/viewmanager.h
    core_lib/src/miniz.cpp core_lib/src/miniz.h
    core_lib/src/movieexporter.cpp core_lib/src/movieexporter.h
    core_lib/src/movieimporter.cpp core_lib/src/movieimporter.h
    core_lib/src/onionskinsubpainter.cpp core_lib/src/onionskinsubpainter.h
    core_lib/src/overlaypainter.cpp core_lib/src/overlaypainter.h
    core_lib/src/qminiz.cpp core_lib/src/qminiz.h
    core_lib/src/selectionpainter.cpp core_lib/src/selectionpainter.h
    core_lib/src/soundplayer.cpp core_lib/src/soundplayer.h
    core_lib/src/structure/camera.cpp core_lib/src/structure/camera.h
    core_lib/src/structure/filemanager.cpp core_lib/src/structure/filemanager.h
    core_lib/src/structure/keyframe.cpp core_lib/src/structure/keyframe.h
    core_lib/src/structure/layer.cpp core_lib/src/structure/layer.h
    core_lib/src/structure/layerbitmap.cpp core_lib/src/structure/layerbitmap.h
    core_lib/src/structure/layercamera.cpp core_lib/src/structure/layercamera.h
    core_lib/src/structure/layersound.cpp core_lib/src/structure/layersound.h
    core_lib/src/structure/layervector.cpp core_lib/src/structure/layervector.h
    core_lib/src/structure/object.cpp core_lib/src/structure/object.h
    core_lib/src/structure/objectdata.cpp core_lib/src/structure/objectdata.h
    core_lib/src/structure/pegbaraligner.cpp core_lib/src/structure/pegbaraligner.h
    core_lib/src/structure/soundclip.cpp core_lib/src/structure/soundclip.h
    core_lib/src/tool/basetool.cpp core_lib/src/tool/basetool.h
    core_lib/src/tool/brushtool.cpp core_lib/src/tool/brushtool.h
    core_lib/src/tool/buckettool.cpp core_lib/src/tool/buckettool.h
    core_lib/src/tool/cameratool.cpp core_lib/src/tool/cameratool.h
    core_lib/src/tool/erasertool.cpp core_lib/src/tool/erasertool.h
    core_lib/src/tool/eyedroppertool.cpp core_lib/src/tool/eyedroppertool.h
    core_lib/src/tool/handtool.cpp core_lib/src/tool/handtool.h
    core_lib/src/tool/movetool.cpp core_lib/src/tool/movetool.h
    core_lib/src/tool/penciltool.cpp core_lib/src/tool/penciltool.h
    core_lib/src/tool/pentool.cpp core_lib/src/tool/pentool.h
    core_lib/src/tool/polylinetool.cpp core_lib/src/tool/polylinetool.h
    core_lib/src/tool/selecttool.cpp core_lib/src/tool/selecttool.h
    core_lib/src/tool/smudgetool.cpp core_lib/src/tool/smudgetool.h
    core_lib/src/tool/strokeinterpolator.cpp core_lib/src/tool/strokeinterpolator.h
    core_lib/src/tool/stroketool.cpp core_lib/src/tool/stroketool.h
    core_lib/src/util/blitrect.cpp core_lib/src/util/blitrect.h
    core_lib/src/util/cameraeasingtype.cpp core_lib/src/util/cameraeasingtype.h
    core_lib/src/util/camerafieldoption.h
    core_lib/src/util/colordictionary.h
    core_lib/src/util/fileformat.cpp core_lib/src/util/fileformat.h
    core_lib/src/util/filetype.h
    core_lib/src/util/log.cpp core_lib/src/util/log.h
    core_lib/src/util/mathutils.h
    core_lib/src/util/movemode.h
    core_lib/src/util/onionskinpainteroptions.h
    core_lib/src/util/onionskinpaintstate.h
    core_lib/src/util/painterutils.h
    core_lib/src/util/pencildef.h
    core_lib/src/util/pencilerror.cpp core_lib/src/util/pencilerror.h
    core_lib/src/util/pencilsettings.cpp core_lib/src/util/pencilsettings.h
    core_lib/src/util/pointerevent.cpp core_lib/src/util/pointerevent.h
    core_lib/src/util/preferencesdef.h
    core_lib/src/util/transform.cpp core_lib/src/util/transform.h
    core_lib/src/util/util.cpp core_lib/src/util/util.h
)

set_target_properties(pencil2d PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

qt_add_ui(pencil2d
    SOURCES app/ui/aboutdialog.ui
    app/ui/bucketoptionswidget.ui
    app/ui/cameraoptionswidget.ui
    app/ui/camerapropertiesdialog.ui
    app/ui/colorinspector.ui
    app/ui/colorpalette.ui
    app/ui/doubleprogressdialog.ui
    app/ui/errordialog.ui
    app/ui/exportimageoptions.ui
    app/ui/exportmovieoptions.ui
    app/ui/filespage.ui
    app/ui/generalpage.ui
    app/ui/importexportdialog.ui
    app/ui/importimageseqoptions.ui
    app/ui/importimageseqpreview.ui
    app/ui/importlayersdialog.ui
    app/ui/importpositiondialog.ui
    app/ui/layeropacitydialog.ui
    app/ui/mainwindow2.ui
    app/ui/onionskin.ui
    app/ui/pegbaralignmentdialog.ui
    app/ui/preferencesdialog.ui
    app/ui/presetdialog.ui
    app/ui/repositionframesdialog.ui
    app/ui/shortcutspage.ui
    app/ui/timelinepage.ui
    app/ui/toolboxwidget.ui
    app/ui/tooloptions.ui
    app/ui/toolspage.ui    
)

target_include_directories(pencil2d PRIVATE
    ../../core_lib/src
    app/../core_lib/src
    app/../core_lib/src/external
    app/../core_lib/src/graphics
    app/../core_lib/src/graphics/bitmap
    app/../core_lib/src/graphics/vector
    app/../core_lib/src/interface
    app/../core_lib/src/managers
    app/../core_lib/src/structure
    app/../core_lib/src/tool
    app/../core_lib/src/util
    app/../core_lib/ui
    app/src
    core_lib/src/external
    core_lib/src/graphics/vector
    core_lib/src/interface
    core_lib/src/managers
    core_lib/src/src
    core_lib/src/src/graphics
    core_lib/src/src/graphics/bitmap
    core_lib/src/structure
    core_lib/src/tool
    core_lib/src/util
)

target_compile_definitions(pencil2d PRIVATE
    APP_VERSION=""
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x050600
)

target_link_libraries(pencil2d PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Multimedia
    Qt::Network
    Qt::Svg
    Qt::Widgets
    Qt::Xml
)


# Resources:
set(core_lib_resource_files
    "core_lib/data/resources/kb.ini"
)

qt_add_resources(pencil2d "core_lib"
    PREFIX
        "/"
    BASE
        "core_lib/data"
    FILES
        ${core_lib_resource_files}
)
set(app_resource_files
    "app/data/background/checkerboard.png"
    "app/data/background/dots.png"
    "app/data/background/grid.jpg"
    "app/data/background/weave.jpg"
    "app/data/icons/blue.png"
    "app/data/icons/general/checkerboard_smaller.png"
    "app/data/icons/general/cross.png"
    "app/data/icons/general/cursor-brush.svg"
    "app/data/icons/general/cursor-bucket.svg"
    "app/data/icons/general/cursor-diagonal-left.svg"
    "app/data/icons/general/cursor-diagonal-right.svg"
    "app/data/icons/general/cursor-eyedropper.svg"
    "app/data/icons/general/cursor-horizontal.svg"
    "app/data/icons/general/cursor-move.svg"
    "app/data/icons/general/cursor-pen.svg"
    "app/data/icons/general/cursor-pencil.svg"
    "app/data/icons/general/cursor-rotate.svg"
    "app/data/icons/general/cursor-smudge-liquify.svg"
    "app/data/icons/general/cursor-smudge.svg"
    "app/data/icons/general/cursor-vertical.svg"
    "app/data/icons/green.png"
    "app/data/icons/icon.png"
    "app/data/icons/logo.png"
    "app/data/icons/red.png"
    "app/data/icons/themes/playful/controls/control-loop.svg"
    "app/data/icons/themes/playful/controls/control-play-end.svg"
    "app/data/icons/themes/playful/controls/control-play-start.svg"
    "app/data/icons/themes/playful/controls/control-play.svg"
    "app/data/icons/themes/playful/controls/control-sound-enable.svg"
    "app/data/icons/themes/playful/controls/control-sound-scrub.svg"
    "app/data/icons/themes/playful/controls/control-stop.svg"
    "app/data/icons/themes/playful/dialog-error.svg"
    "app/data/icons/themes/playful/display/lines-invisible.svg"
    "app/data/icons/themes/playful/display/lines-outline.svg"
    "app/data/icons/themes/playful/display/mirror-horizontal.svg"
    "app/data/icons/themes/playful/display/mirror-vertical.svg"
    "app/data/icons/themes/playful/display/overlay-center.svg"
    "app/data/icons/themes/playful/display/overlay-golden-ratio.svg"
    "app/data/icons/themes/playful/display/overlay-grid.svg"
    "app/data/icons/themes/playful/display/overlay-safe.svg"
    "app/data/icons/themes/playful/display/overlay-thirds.svg"
    "app/data/icons/themes/playful/display/perspective-angle.svg"
    "app/data/icons/themes/playful/display/perspective-onepoint.svg"
    "app/data/icons/themes/playful/display/perspective-threepoints.svg"
    "app/data/icons/themes/playful/display/perspective-twopoints.svg"
    "app/data/icons/themes/playful/menubar/clear-canvas.svg"
    "app/data/icons/themes/playful/menubar/copy.svg"
    "app/data/icons/themes/playful/menubar/cut.svg"
    "app/data/icons/themes/playful/menubar/new.svg"
    "app/data/icons/themes/playful/menubar/open.svg"
    "app/data/icons/themes/playful/menubar/paste.svg"
    "app/data/icons/themes/playful/menubar/redo.svg"
    "app/data/icons/themes/playful/menubar/save.svg"
    "app/data/icons/themes/playful/menubar/undo.svg"
    "app/data/icons/themes/playful/menubar/view-reset.svg"
    "app/data/icons/themes/playful/menubar/zoom-in.svg"
    "app/data/icons/themes/playful/menubar/zoom-out.svg"
    "app/data/icons/themes/playful/menubar/zoom-select.svg"
    "app/data/icons/themes/playful/misc/add-color.svg"
    "app/data/icons/themes/playful/misc/color-dialog.svg"
    "app/data/icons/themes/playful/misc/more-options.svg"
    "app/data/icons/themes/playful/misc/remove-color.svg"
    "app/data/icons/themes/playful/onion/onionskin-blue.svg"
    "app/data/icons/themes/playful/onion/onionskin-enable.svg"
    "app/data/icons/themes/playful/onion/onionskin-red.svg"
    "app/data/icons/themes/playful/preferences/preferences-files.svg"
    "app/data/icons/themes/playful/preferences/preferences-general.svg"
    "app/data/icons/themes/playful/preferences/preferences-shortcuts.svg"
    "app/data/icons/themes/playful/preferences/preferences-timeline.svg"
    "app/data/icons/themes/playful/preferences/preferences-tools.svg"
    "app/data/icons/themes/playful/timeline/cell-bitmap.svg"
    "app/data/icons/themes/playful/timeline/cell-camera.svg"
    "app/data/icons/themes/playful/timeline/cell-sound.svg"
    "app/data/icons/themes/playful/timeline/cell-vector.svg"
    "app/data/icons/themes/playful/timeline/frame-add.svg"
    "app/data/icons/themes/playful/timeline/frame-duplicate.svg"
    "app/data/icons/themes/playful/timeline/frame-remove.svg"
    "app/data/icons/themes/playful/timeline/layer-add.svg"
    "app/data/icons/themes/playful/timeline/layer-duplicate.svg"
    "app/data/icons/themes/playful/timeline/layer-remove.svg"
    "app/data/icons/themes/playful/tools/tool-brush.svg"
    "app/data/icons/themes/playful/tools/tool-bucket.svg"
    "app/data/icons/themes/playful/tools/tool-camera-move.svg"
    "app/data/icons/themes/playful/tools/tool-camera-rotate.svg"
    "app/data/icons/themes/playful/tools/tool-camera-scale.svg"
    "app/data/icons/themes/playful/tools/tool-eraser.svg"
    "app/data/icons/themes/playful/tools/tool-eyedropper.svg"
    "app/data/icons/themes/playful/tools/tool-hand.svg"
    "app/data/icons/themes/playful/tools/tool-move.svg"
    "app/data/icons/themes/playful/tools/tool-pen.svg"
    "app/data/icons/themes/playful/tools/tool-pencil.svg"
    "app/data/icons/themes/playful/tools/tool-polyline.svg"
    "app/data/icons/themes/playful/tools/tool-select.svg"
    "app/data/icons/themes/playful/tools/tool-smudge.svg"
)

qt_add_resources(pencil2d "app"
    PREFIX
        "/"
    BASE
        "app/data"
    FILES
        ${app_resource_files}
)
set(app1_resource_files
    "app/data/pencil2d_quick_guide.pdf"
)

qt_add_resources(pencil2d "app1"
    PREFIX
        "/app"
    BASE
        "app/data"
    FILES
        ${app1_resource_files}
)

if(PENCIL2D_NIGHTLY)
    target_compile_definitions(pencil2d PRIVATE
        PENCIL2D_NIGHTLY_BUILD
    )
endif()

if(PENCIL2D_RELEASE)
    target_compile_definitions(pencil2d PRIVATE
        PENCIL2D_RELEASE_BUILD
        QT_NO_DEBUG_OUTPUT
    )
endif()

if(WIN32 AND NOT WIN_LEGACY)
    target_compile_definitions(pencil2d PRIVATE
        _WIN32_WINNT=0x0601
    )
endif()

if(WIN_LEGACY)
    target_compile_definitions(pencil2d PRIVATE
        _WIN32_WINNT=0x0501
    )
endif()

if(MACOS)
    target_sources(pencil2d PUBLIC
        core_lib/src/external/macosx/macosx.cpp
        core_lib/src/external/macosx/macosxnative.h core_lib/src/external/macosx/macosxnative.mm
    )

    target_include_directories(pencil2d PRIVATE
        core_lib/src/external/macosx
    )

    target_link_libraries(pencil2d PRIVATE
        "-framework AppKit"
        "-framework Carbon"
        objc
    )
endif()

if(WIN32)
    target_sources(pencil2d PUBLIC
        core_lib/src/external/win32/win32.cpp
    )

    target_include_directories(pencil2d PRIVATE
        core_lib/src/external/win32
    )
endif()

if(UNIX AND NOT MACOS)
    target_sources(pencil2d PUBLIC
        core_lib/src/external/linux/linux.cpp
    )

    target_include_directories(pencil2d PRIVATE
        core_lib/src/external/linux
    )
endif()

install(TARGETS pencil2d
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET pencil2d
    FILENAME_VARIABLE deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
