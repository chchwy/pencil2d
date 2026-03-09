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

#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#ifdef SENTRY_ENABLED

#include <QString>

/**
 * Initializes and manages the Sentry/GlitchTip crash reporter.
 *
 * Uses the sentry-native SDK with crashpad as the crash backend on Windows.
 * Only compiled when SENTRY_ENABLED is defined (i.e., SENTRY_DSN is passed
 * to qmake at build time). Compatible with both Sentry and GlitchTip.
 */
class CrashHandler
{
public:
    /**
     * Initializes the crash handler. Call once at application startup,
     * after QCoreApplication is constructed.
     *
     * @param handlerPath  Absolute path to the crashpad_handler executable.
     * @param dbPath       Directory where crash databases and reports are stored.
     */
    static void init(const QString& handlerPath, const QString& dbPath);

    /**
     * Flushes pending crash reports and shuts down the crash handler.
     * Must be called before the application exits.
     */
    static void close();
};

#endif // SENTRY_ENABLED

#endif // CRASHHANDLER_H
