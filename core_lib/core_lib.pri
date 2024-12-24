#-------------------------------------------------
#
# Pencil2D core library
#
#-------------------------------------------------

RESOURCES += $$PWD/data/core_lib.qrc

INCLUDEPATH += src \
    $$PWD/src/graphics \
    $$PWD/src/graphics/bitmap \
    $$PWD/src/graphics/vector \
    $$PWD/src/interface \
    $$PWD/src/structure \
    $$PWD/src/tool \
    $$PWD/src/util \
    $$PWD/src/managers \
    $$PWD/src/external

PRECOMPILED_HEADER = $$PWD/src/corelib-pch.h

HEADERS +=  \
    $$PWD/src/canvascursorpainter.h \
    $$PWD/src/corelib-pch.h \
    $$PWD/src/graphics/bitmap/bitmapbucket.h \
    $$PWD/src/graphics/bitmap/bitmapimage.h \
    $$PWD/src/graphics/bitmap/tile.h \
    $$PWD/src/graphics/bitmap/tiledbuffer.h \
    $$PWD/src/graphics/vector/bezierarea.h \
    $$PWD/src/graphics/vector/beziercurve.h \
    $$PWD/src/graphics/vector/colorref.h \
    $$PWD/src/graphics/vector/vectorimage.h \
    $$PWD/src/graphics/vector/vectorselection.h \
    $$PWD/src/graphics/vector/vertexref.h \
    $$PWD/src/interface/editor.h \
    $$PWD/src/interface/flowlayout.h \
    $$PWD/src/interface/legacybackupelement.h \
    $$PWD/src/interface/recentfilemenu.h \
    $$PWD/src/interface/scribblearea.h \
    $$PWD/src/interface/backgroundwidget.h \
    $$PWD/src/interface/undoredocommand.h \
    $$PWD/src/managers/basemanager.h \
    $$PWD/src/managers/overlaymanager.h \
    $$PWD/src/managers/clipboardmanager.h \
    $$PWD/src/managers/selectionmanager.h \
    $$PWD/src/managers/colormanager.h \
    $$PWD/src/managers/layermanager.h \
    $$PWD/src/managers/toolmanager.h \
    $$PWD/src/managers/playbackmanager.h \
    $$PWD/src/managers/undoredomanager.h \
    $$PWD/src/managers/viewmanager.h \
    $$PWD/src/managers/preferencemanager.h \
    $$PWD/src/managers/soundmanager.h \
    $$PWD/src/movieimporter.h \
    $$PWD/src/onionskinsubpainter.h \
    $$PWD/src/overlaypainter.h \
    $$PWD/src/camerapainter.h \
    $$PWD/src/structure/camera.h \
    $$PWD/src/structure/keyframe.h \
    $$PWD/src/structure/layer.h \
    $$PWD/src/structure/layerbitmap.h \
    $$PWD/src/structure/layercamera.h \
    $$PWD/src/structure/layersound.h \
    $$PWD/src/structure/layervector.h \
    $$PWD/src/structure/pegbaraligner.h \
    $$PWD/src/structure/soundclip.h \
    $$PWD/src/structure/object.h \
    $$PWD/src/structure/objectdata.h \
    $$PWD/src/structure/filemanager.h \
    $$PWD/src/tool/basetool.h \
    $$PWD/src/tool/brushtool.h \
    $$PWD/src/tool/buckettool.h \
    $$PWD/src/tool/cameratool.h \
    $$PWD/src/tool/erasertool.h \
    $$PWD/src/tool/eyedroppertool.h \
    $$PWD/src/tool/handtool.h \
    $$PWD/src/tool/movetool.h \
    $$PWD/src/tool/penciltool.h \
    $$PWD/src/tool/pentool.h \
    $$PWD/src/tool/polylinetool.h \
    $$PWD/src/tool/selecttool.h \
    $$PWD/src/tool/smudgetool.h \
    $$PWD/src/tool/strokeinterpolator.h \
    $$PWD/src/tool/stroketool.h \
    $$PWD/src/util/blitrect.h \
    $$PWD/src/util/cameraeasingtype.h \
    $$PWD/src/util/camerafieldoption.h \
    $$PWD/src/util/colordictionary.h \
    $$PWD/src/util/fileformat.h \
    $$PWD/src/util/filetype.h \
    $$PWD/src/util/mathutils.h \
    $$PWD/src/util/onionskinpainteroptions.h \
    $$PWD/src/util/onionskinpaintstate.h \
    $$PWD/src/util/painterutils.h \
    $$PWD/src/util/pencildef.h \
    $$PWD/src/util/pencilerror.h \
    $$PWD/src/util/pencilsettings.h \
    $$PWD/src/util/preferencesdef.h \
    $$PWD/src/util/transform.h \
    $$PWD/src/util/util.h \
    $$PWD/src/util/log.h \
    $$PWD/src/util/movemode.h \
    $$PWD/src/util/pointerevent.h \
    $$PWD/src/canvaspainter.h \
    $$PWD/src/soundplayer.h \
    $$PWD/src/movieexporter.h \
    $$PWD/src/miniz.h \
    $$PWD/src/qminiz.h \
    $$PWD/src/activeframepool.h \
    $$PWD/src/external/platformhandler.h \
    $$PWD/src/selectionpainter.h


SOURCES +=  \
    $$PWD/src/graphics/bitmap/bitmapimage.cpp \
    $$PWD/src/canvascursorpainter.cpp \
    $$PWD/src/graphics/bitmap/bitmapbucket.cpp \
    $$PWD/src/graphics/bitmap/tile.cpp \
    $$PWD/src/graphics/bitmap/tiledbuffer.cpp \
    $$PWD/src/graphics/vector/bezierarea.cpp \
    $$PWD/src/graphics/vector/beziercurve.cpp \
    $$PWD/src/graphics/vector/colorref.cpp \
    $$PWD/src/graphics/vector/vectorimage.cpp \
    $$PWD/src/graphics/vector/vectorselection.cpp \
    $$PWD/src/graphics/vector/vertexref.cpp \
    $$PWD/src/interface/editor.cpp \
    $$PWD/src/interface/flowlayout.cpp \
    $$PWD/src/interface/legacybackupelement.cpp \
    $$PWD/src/interface/recentfilemenu.cpp \
    $$PWD/src/interface/scribblearea.cpp \
    $$PWD/src/interface/backgroundwidget.cpp \
    $$PWD/src/interface/undoredocommand.cpp \
    $$PWD/src/managers/basemanager.cpp \
    $$PWD/src/managers/overlaymanager.cpp \
    $$PWD/src/managers/clipboardmanager.cpp \
    $$PWD/src/managers/selectionmanager.cpp \
    $$PWD/src/managers/colormanager.cpp \
    $$PWD/src/managers/layermanager.cpp \
    $$PWD/src/managers/toolmanager.cpp \
    $$PWD/src/managers/preferencemanager.cpp \
    $$PWD/src/managers/playbackmanager.cpp \
    $$PWD/src/managers/undoredomanager.cpp \
    $$PWD/src/managers/viewmanager.cpp \
    $$PWD/src/managers/soundmanager.cpp \
    $$PWD/src/movieimporter.cpp \
    $$PWD/src/structure/camera.cpp \
    $$PWD/src/structure/keyframe.cpp \
    $$PWD/src/structure/layer.cpp \
    $$PWD/src/structure/layerbitmap.cpp \
    $$PWD/src/structure/layercamera.cpp \
    $$PWD/src/structure/layersound.cpp \
    $$PWD/src/structure/layervector.cpp \
    $$PWD/src/structure/object.cpp \
    $$PWD/src/structure/pegbaraligner.cpp \
    $$PWD/src/structure/soundclip.cpp \
    $$PWD/src/structure/objectdata.cpp \
    $$PWD/src/structure/filemanager.cpp \
    $$PWD/src/tool/basetool.cpp \
    $$PWD/src/tool/brushtool.cpp \
    $$PWD/src/tool/buckettool.cpp \
    $$PWD/src/tool/cameratool.cpp \
    $$PWD/src/tool/erasertool.cpp \
    $$PWD/src/tool/eyedroppertool.cpp \
    $$PWD/src/tool/handtool.cpp \
    $$PWD/src/tool/movetool.cpp \
    $$PWD/src/tool/penciltool.cpp \
    $$PWD/src/tool/pentool.cpp \
    $$PWD/src/tool/polylinetool.cpp \
    $$PWD/src/tool/selecttool.cpp \
    $$PWD/src/tool/smudgetool.cpp \
    $$PWD/src/tool/strokeinterpolator.cpp \
    $$PWD/src/tool/stroketool.cpp \
    $$PWD/src/util/blitrect.cpp \
    $$PWD/src/util/cameraeasingtype.cpp \
    $$PWD/src/util/fileformat.cpp \
    $$PWD/src/util/pencilerror.cpp \
    $$PWD/src/util/pencilsettings.cpp \
    $$PWD/src/util/log.cpp \
    $$PWD/src/util/transform.cpp \
    $$PWD/src/util/util.cpp \
    $$PWD/src/util/pointerevent.cpp \
    $$PWD/src/canvaspainter.cpp \
    $$PWD/src/overlaypainter.cpp \
    $$PWD/src/onionskinsubpainter.cpp \
    $$PWD/src/camerapainter.cpp \
    $$PWD/src/soundplayer.cpp \
    $$PWD/src/movieexporter.cpp \
    $$PWD/src/miniz.cpp \
    $$PWD/src/qminiz.cpp \
    $$PWD/src/activeframepool.cpp \
    $$PWD/src/selectionpainter.cpp

win32 {
    INCLUDEPATH += $$PWD/src/external/win32
    SOURCES += $$PWD/src/external/win32/win32.cpp
}

macx {
    INCLUDEPATH += $$PWD/src/external/macosx
    HEADERS += $$PWD/src/external/macosx/macosxnative.h
    SOURCES += $$PWD/src/external/macosx/macosx.cpp
    OBJECTIVE_SOURCES += $$PWD/src/external/macosx/macosxnative.mm
}

unix:!macx {
    INCLUDEPATH += $$PWD/src/external/linux
    SOURCES += $$PWD/src/external/linux/linux.cpp
}
