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

#include "crashhandler.h"

#ifdef SENTRY_ENABLED

#include <sentry.h>

#include <QDir>

void CrashHandler::init(const QString& handlerPath, const QString& dbPath)
{
    QDir().mkpath(dbPath);

    sentry_options_t* options = sentry_options_new();

    sentry_options_set_dsn(options, SENTRY_DSN);
    sentry_options_set_release(options, "pencil2d@" APP_VERSION);
    sentry_options_set_environment(options, SENTRY_ENVIRONMENT);

    // crashpad_handler.exe must be alongside pencil2d.exe
    sentry_options_set_handler_path(options, handlerPath.toUtf8().constData());

    // Directory where crash databases and pending uploads are stored
    sentry_options_set_database_path(options, dbPath.toUtf8().constData());

    sentry_init(options);
}

void CrashHandler::close()
{
    sentry_close();
}

#endif // SENTRY_ENABLED
