QT += core gui widgets

TARGET = QTMyPaint
TEMPLATE = lib
CONFIG += shared

HEADERS += mpbrush.h \
           mphandler.h \
           mpsurface.h \
           mptile.h

SOURCES += mpbrush.cpp \
           mphandler.cpp \
           mpsurface.cpp \
           mptile.cpp

# --- json-c ---
win32:CONFIG(release, debug|release): LIBS += -L../json-c/release/ -ljson-c
else:win32:CONFIG(debug, debug|release): LIBS += -L../json-c/debug/ -ljson-c
else:unix: LIBS += -L../json-c -ljson-c
macx: LIBS += -L../json-c -ljson-c

INCLUDEPATH += ../json-c
DEPENDPATH += ../json-c


win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += ../json-c/release/libjson-c.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += ../json-c/debug/libjson-c.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += ../json-c/release/json-c.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += ../json-c/debug/json-c.lib
else:unix: PRE_TARGETDEPS += ../json-c/libjson-c.a

# --- libmypaint ---

# We use a precompiled version of mypaint to avoid various compile problem with dependencies
win32:CONFIG(release, debug|release): LIBS += -L../libmypaint/release/ -l libmypaint
else:win32:CONFIG(debug, debug|release): LIBS += -L../libmypaint/debug/ -l libmypaint
else:!macx:unix: LIBS += -L$$PWD/../libmypaint/ -l libmypaint \
                   $$PWD/../libmypaint/libmypaint.so
macx: LIBS += $$PWD/../libmypaint/libmypaint.dylib

INCLUDEPATH += $$PWD/../libmypaint
DEPENDPATH += $$PWD/../libmypaint
