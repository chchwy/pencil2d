QT += core

TARGET = paint_lib
TEMPLATE = lib
CONFIG += staticlib
DEFINES += HAVE_JSON_C

win32-g++ {
    QMAKE_CXXFLAGS += -std=c++11
}

win32-msvc* {
    QMAKE_CXXFLAGS += /MP
}

macx {
   QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++
}

unix:!macx {
    QMAKE_CXXFLAGS += -std=c++11
    QMAKE_LINK = $$QMAKE_CXX
    QMAKE_LINK_SHLIB = $$QMAKE_CXX
}

QMAKE_CFLAGS += -std=c99
QMAKE_CFLAGS += -D_XOPEN_SOURCE=600
QMAKE_CC = gcc

# libmypaint
SOURCES += \
            $$PWD/libmypaint/mypaint-brush.c \
            $$PWD/libmypaint/brushmodes.c \
            $$PWD/libmypaint/fifo.c \
            $$PWD/libmypaint/helpers.c \
            $$PWD/libmypaint/libmypaint.c \
            $$PWD/libmypaint/mypaint-brush-settings.c \
            $$PWD/libmypaint/mypaint-brush.c \
            $$PWD/libmypaint/mypaint-fixed-tiled-surface.c \
            $$PWD/libmypaint/mypaint-mapping.c \
            $$PWD/libmypaint/mypaint-rectangle.c \
            $$PWD/libmypaint/mypaint-surface.c \
            $$PWD/libmypaint/mypaint-tiled-surface.c \
            $$PWD/libmypaint/mypaint.c \
            $$PWD/libmypaint/operationqueue.c \
            $$PWD/libmypaint/rng-double.c \
            $$PWD/libmypaint/tilemap.c \
            $$PWD/libmypaint/utils.c

HEADERS += \
            $$PWD/libmypaint/brushmodes.h \
            $$PWD/libmypaint/brushsettings-gen.h \
#            $$PWD/libmypaint/config.h \
            $$PWD/libmypaint/fifo.h \
            $$PWD/libmypaint/helpers.h \
            $$PWD/libmypaint/mypaint-brush-settings-gen.h \
            $$PWD/libmypaint/mypaint-brush-settings.h \
            $$PWD/libmypaint/mypaint-brush.h \
            $$PWD/libmypaint/mypaint-brush.h \
            $$PWD/libmypaint/mypaint-config.h \
            $$PWD/libmypaint/mypaint-fixed-tiled-surface.h \
            $$PWD/libmypaint/mypaint-glib-compat.h \
            $$PWD/libmypaint/mypaint-mapping.h \
            $$PWD/libmypaint/mypaint-rectangle.h \
            $$PWD/libmypaint/mypaint-surface.h \
            $$PWD/libmypaint/mypaint-tiled-surface.h \
            $$PWD/libmypaint/mypaint.h \
            $$PWD/libmypaint/operationqueue.h \
            $$PWD/libmypaint/rng-double.h \
            $$PWD/libmypaint/tiled-surface-private.h \
            $$PWD/libmypaint/tilemap.h \
            $$PWD/libmypaint/utils.h

INCLUDEPATH += $$PWD/libmypaint
DEPENDPATH += $$PWD/libmypaint

# json-c

jsonlibpath = $$PWD/json-c
# HACK: Copy config.h that fits the respective platform, created by configure in another build
exists($$jsonlibpath/config.h) {
    message("config.h moved to correct folder")
} else {
    win32 {
        system(echo "testing win32 ")
        system($$QMAKE_COPY \"$$jsonlibpath\\config\\win32\\config.h\" \"$$jsonlibpath\\config.h\" $$escape_expand(\\n))
        QMAKE_CLEAN += -r $$jsonlibpath\\config.h
    }
    macx|unix {
        system(echo "testing macx and unix ")
        system($$QMAKE_COPY $$jsonlibpath/config/macxunix/config.h $$jsonlibpath $$escape_expand(\\n))
        QMAKE_CLEAN += -r $$jsonlibpath/config.h
    }
}

INCLUDEPATH += $$PWD/json-c
DEPENDPATH += $$PWD/json-c

HEADERS += $$PWD/json-c/arraylist.h \
           $$PWD/json-c/bits.h \
           $$PWD/json-c/debug.h \
           $$PWD/json-c/json.h \
           $$PWD/json-c/json_c_version.h \
           $$PWD/json-c/json_inttypes.h \
           $$PWD/json-c/json_object.h \
           $$PWD/json-c/json_object_iterator.h \
           $$PWD/json-c/json_object_private.h \
           $$PWD/json-c/json_tokener.h \
           $$PWD/json-c/json_util.h \
           $$PWD/json-c/linkhash.h \
           $$PWD/json-c/math_compat.h \
           $$PWD/json-c/printbuf.h \
           $$PWD/json-c/random_seed.h \
           $$PWD/json-c/config.h \
           $$PWD/json-c/json_config.h

SOURCES += $$PWD/json-c/arraylist.c \
           $$PWD/json-c/debug.c \
           $$PWD/json-c/json_c_version.c \
           $$PWD/json-c/json_object.c \
           $$PWD/json-c/json_object_iterator.c \
           $$PWD/json-c/json_tokener.c \
           $$PWD/json-c/json_util.c \
           $$PWD/json-c/libjson.c \
           $$PWD/json-c/linkhash.c \
           $$PWD/json-c/printbuf.c \
           $$PWD/json-c/random_seed.c


# qtmypaint
HEADERS += $$PWD/qtmypaint/mpbrush.h \
           $$PWD/qtmypaint/mphandler.h \
           $$PWD/qtmypaint/mpsurface.h \
           $$PWD/qtmypaint/mptile.h

SOURCES += $$PWD/qtmypaint/mpbrush.cpp \
           $$PWD/qtmypaint/mphandler.cpp \
           $$PWD/qtmypaint/mpsurface.cpp \
           $$PWD/qtmypaint/mptile.cpp

INCLUDEPATH += $$PWD/qtmypaint
DEPENDPATH += $$PWD/qtmypaint
