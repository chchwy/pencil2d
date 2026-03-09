# Sentry / GlitchTip crash reporting integration (Windows x64, MSVC)
#
# Pass SENTRY_DSN=<your-dsn> to qmake to enable crash reporting, e.g.:
#   qmake SENTRY_DSN="https://key@glitchtip.example.com/1"
#
# Optionally override the SDK location:
#   qmake SENTRY_DSN=... SENTRY_SDK_PATH=/path/to/sentry-native/install
#
# The sentry-native SDK must be built first:
#   git clone --depth=1 --recurse-submodules \
#       https://github.com/getsentry/sentry-native.git third_party/sentry-native
#   cmake -S third_party/sentry-native -B third_party/sentry-native/build \
#       -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSENTRY_BACKEND=crashpad \
#       -DCMAKE_INSTALL_PREFIX=third_party/sentry-native/install
#   cmake --build third_party/sentry-native/build --config RelWithDebInfo
#   cmake --install third_party/sentry-native/build --config RelWithDebInfo

win32-msvc*:!isEmpty(SENTRY_DSN) {
    isEmpty(SENTRY_SDK_PATH): SENTRY_SDK_PATH = $$PWD/../third_party/sentry-native/install

    !exists($$SENTRY_SDK_PATH/include/sentry.h) {
        error("sentry-native SDK not found at $$SENTRY_SDK_PATH. Build it first or set SENTRY_SDK_PATH=<path>.")
    }

    DEFINES += SENTRY_ENABLED
    DEFINES += "SENTRY_DSN=\\\"$$SENTRY_DSN\\\""

    # Map build type to a Sentry environment string
    contains(VERSION, ^99\\.0\\.0\\..*) {
        DEFINES += "SENTRY_ENVIRONMENT=\\\"nightly\\\""
    } else:equals(VERSION, "0.0.0.0") {
        DEFINES += "SENTRY_ENVIRONMENT=\\\"development\\\""
    } else {
        DEFINES += "SENTRY_ENVIRONMENT=\\\"production\\\""
    }

    INCLUDEPATH += $$SENTRY_SDK_PATH/include
    LIBS += -L$$SENTRY_SDK_PATH/lib -lsentry

    # Install sentry.dll and crashpad_handler.exe alongside pencil2d.exe
    sentry_bins.path = /
    sentry_bins.files = \
        $$SENTRY_SDK_PATH/bin/sentry.dll \
        $$SENTRY_SDK_PATH/bin/crashpad_handler.exe
    INSTALLS += sentry_bins
}
