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

#include "projectstoragebackendsqlite.h"

#include <QUuid>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "log.h"

ProjectStorageBackendSqlite::ProjectStorageBackendSqlite()
{
}

ProjectStorageBackendSqlite::~ProjectStorageBackendSqlite()
{
    close();
}

Status ProjectStorageBackendSqlite::open(const QString& projectPath, bool createIfMissing)
{
    DebugDetails dd;
    dd << "[SQLite backend open]";

    if (projectPath.isEmpty())
    {
        dd << "Project path is empty.";
        return Status(Status::INVALID_ARGUMENT, dd);
    }

    mProjectPath = projectPath;
    mConnectionName = QString("pencil_sqlite_%1").arg(QUuid::createUuid().toString(QUuid::Id128));
    mDatabase = QSqlDatabase::addDatabase("QSQLITE", mConnectionName);
    mDatabase.setDatabaseName(mProjectPath);

    if (!createIfMissing && !QFileInfo::exists(mProjectPath))
    {
        dd << QString("File does not exist: %1").arg(mProjectPath);
        return Status(Status::FILE_NOT_FOUND, dd);
    }

    if (!mDatabase.open())
    {
        dd << QString("Cannot open SQLite database: %1").arg(mDatabase.lastError().text());
        return Status(Status::ERROR_FILE_CANNOT_OPEN, dd);
    }

    Status pragmaStatus = executePragma("PRAGMA foreign_keys = ON;");
    if (!pragmaStatus.ok())
    {
        return pragmaStatus;
    }

    pragmaStatus = executePragma("PRAGMA journal_mode = WAL;");
    if (!pragmaStatus.ok())
    {
        return pragmaStatus;
    }

    pragmaStatus = executePragma("PRAGMA synchronous = NORMAL;");
    if (!pragmaStatus.ok())
    {
        return pragmaStatus;
    }

    return ensureSchema();
}

Status ProjectStorageBackendSqlite::close()
{
    if (mDatabase.isValid() && mDatabase.isOpen())
    {
        mDatabase.close();
    }

    if (!mConnectionName.isEmpty())
    {
        mDatabase = QSqlDatabase();
        QSqlDatabase::removeDatabase(mConnectionName);
        mConnectionName.clear();
    }

    return Status::OK;
}

Object* ProjectStorageBackendSqlite::loadProject()
{
    FILEMANAGER_LOG("SQLite loadProject is not implemented yet.");
    return nullptr;
}

Status ProjectStorageBackendSqlite::saveProject(const Object* object)
{
    Q_UNUSED(object)

    DebugDetails dd;
    dd << "SQLite saveProject is not implemented yet.";
    return Status(Status::NOT_IMPLEMENTED_YET, dd,
                  QObject::tr("SQLite Project Format"),
                  QObject::tr("Saving to SQLite is not implemented yet."));
}

Status ProjectStorageBackendSqlite::incrementalSave(const Object* object)
{
    Q_UNUSED(object)

    DebugDetails dd;
    dd << "SQLite incrementalSave is not implemented yet.";
    return Status(Status::NOT_IMPLEMENTED_YET, dd,
                  QObject::tr("SQLite Project Format"),
                  QObject::tr("Autosave to SQLite is not implemented yet."));
}

Status ProjectStorageBackendSqlite::verify()
{
    DebugDetails dd;
    dd << "[SQLite backend verify]";

    if (!mDatabase.isOpen())
    {
        dd << "Database is not open.";
        return Status(Status::ERROR_FILE_CANNOT_OPEN, dd);
    }

    QSqlQuery query(mDatabase);
    if (!query.exec("PRAGMA quick_check;"))
    {
        dd << QString("quick_check failed: %1").arg(query.lastError().text());
        return Status(Status::FAIL, dd);
    }

    if (query.next())
    {
        const QString checkResult = query.value(0).toString();
        if (checkResult.compare("ok", Qt::CaseInsensitive) != 0)
        {
            dd << QString("quick_check reported: %1").arg(checkResult);
            return Status(Status::FAIL, dd);
        }
    }

    return Status::OK;
}

Status ProjectStorageBackendSqlite::ensureSchema()
{
    DebugDetails dd;
    dd << "[SQLite backend ensureSchema]";

    QSqlQuery query(mDatabase);

    const QString createSchemaVersion =
        "CREATE TABLE IF NOT EXISTS schema_version ("
        "id INTEGER PRIMARY KEY CHECK(id = 1),"
        "version INTEGER NOT NULL"
        ");";

    if (!query.exec(createSchemaVersion))
    {
        dd << QString("Failed to create schema_version: %1").arg(query.lastError().text());
        return Status(Status::FAIL, dd);
    }

    if (!query.exec("INSERT OR IGNORE INTO schema_version (id, version) VALUES (1, 1);"))
    {
        dd << QString("Failed to initialize schema_version: %1").arg(query.lastError().text());
        return Status(Status::FAIL, dd);
    }

    return Status::OK;
}

Status ProjectStorageBackendSqlite::executePragma(const QString& pragmaSql)
{
    DebugDetails dd;
    dd << QString("[SQLite backend pragma] %1").arg(pragmaSql);

    QSqlQuery query(mDatabase);
    if (!query.exec(pragmaSql))
    {
        dd << QString("Pragma failed: %1").arg(query.lastError().text());
        return Status(Status::FAIL, dd);
    }

    return Status::OK;
}
