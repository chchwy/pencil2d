QT += core gui widgets

TARGET = libmypaint
TEMPLATE = lib
CONFIG += sharedlib

HEADERS += brushmodes.h \
           brushsettings-gen.h \
           fifo.h \
           helpers.h \
           mypaint-mapping.h \
           mypaint.h \
           mypaint-brush.h \
           mypaint-brush-settings.h \
           mypaint-brush-settings-gen.h \
           mypaint-config.h \
           mypaint-fixed-tiled-surface.h \
           mypaint-glib-compat.h \
           mypaint-rectangle.h \
           mypaint-surface.h \
           mypaint-tiled-surface.h \
           operationqueue.h \
           rng-double.h \
           tiled-surface-private.h \
           tilemap.h \
           utils.h

INCLUDEPATH += ../json-c
LIBS += -L../json-c -ljson-c

# for C files, we need to allow C99 mode.
QMAKE_CFLAGS += -std=c99
