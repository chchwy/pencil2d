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

#include "log.h"
#include "pencil2d.h"
#include "pencilerror.h"
#include "platformhandler.h"

#ifdef SENTRY_ENABLED
#include "crashhandler.h"
#include <QCoreApplication>
#include <QStandardPaths>
#endif

/**
 * This is the entrypoint of the program. It performs basic initialization, then
 * boots the actual application (@ref Pencil2D).
 */
int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(core_lib);
    PlatformHandler::initialise();
    initCategoryLogging();

    Pencil2D app(argc, argv);

#ifdef SENTRY_ENABLED
    {
        QString handlerPath = QCoreApplication::applicationDirPath() + "/crashpad_handler.exe";
        QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/crashes";
        CrashHandler::init(handlerPath, dbPath);
    }
#endif

    int result;
    switch (app.handleCommandLineOptions().code())
    {
        case Status::OK:
            result = Pencil2D::exec();
            break;
        case Status::SAFE:
            result = EXIT_SUCCESS;
            break;
        default:
            result = EXIT_FAILURE;
            break;
    }

#ifdef SENTRY_ENABLED
    CrashHandler::close();
#endif

    return result;
}
