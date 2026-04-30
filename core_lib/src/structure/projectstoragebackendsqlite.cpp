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
#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QDomDocument>
#include <QTextStream>

#include "log.h"
#include "object.h"

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
    DebugDetails dd;
    dd << "[SQLite backend saveProject]";

    if (object == nullptr)
    {
        dd << "Object parameter is null.";
        return Status(Status::INVALID_ARGUMENT, dd);
    }

    if (!mDatabase.isOpen())
    {
        dd << "Database is not open.";
        return Status(Status::ERROR_FILE_CANNOT_OPEN, dd);
    }

    Status schemaStatus = ensureSchema();
    if (!schemaStatus.ok())
    {
        return schemaStatus;
    }

    QDomDocument xmlDoc("PencilDocument");
    QDomElement root = xmlDoc.createElement("document");
    QDomProcessingInstruction encoding =
        xmlDoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    xmlDoc.appendChild(encoding);
    xmlDoc.appendChild(root);

    const ObjectData* data = object->data();

    QDomElement projectData = xmlDoc.createElement("projectdata");

    QDomElement currentFrameTag = xmlDoc.createElement("currentFrame");
    currentFrameTag.setAttribute("value", data->getCurrentFrame());
    projectData.appendChild(currentFrameTag);

    QDomElement currentColorTag = xmlDoc.createElement("currentColor");
    QColor currentColor = data->getCurrentColor();
    currentColorTag.setAttribute("r", currentColor.red());
    currentColorTag.setAttribute("g", currentColor.green());
    currentColorTag.setAttribute("b", currentColor.blue());
    currentColorTag.setAttribute("a", currentColor.alpha());
    projectData.appendChild(currentColorTag);

    QDomElement currentLayerTag = xmlDoc.createElement("currentLayer");
    currentLayerTag.setAttribute("value", data->getCurrentLayer());
    projectData.appendChild(currentLayerTag);

    QDomElement currentViewTag = xmlDoc.createElement("currentView");
    QTransform view = data->getCurrentView();
    currentViewTag.setAttribute("m11", view.m11());
    currentViewTag.setAttribute("m12", view.m12());
    currentViewTag.setAttribute("m21", view.m21());
    currentViewTag.setAttribute("m22", view.m22());
    currentViewTag.setAttribute("dx", view.dx());
    currentViewTag.setAttribute("dy", view.dy());
    projectData.appendChild(currentViewTag);

    QDomElement fpsTag = xmlDoc.createElement("fps");
    fpsTag.setAttribute("value", data->getFrameRate());
    projectData.appendChild(fpsTag);

    QDomElement isLoopTag = xmlDoc.createElement("isLoop");
    isLoopTag.setAttribute("value", data->isLooping() ? "true" : "false");
    projectData.appendChild(isLoopTag);

    QDomElement rangedPlaybackTag = xmlDoc.createElement("isRangedPlayback");
    rangedPlaybackTag.setAttribute("value", data->isRangedPlayback() ? "true" : "false");
    projectData.appendChild(rangedPlaybackTag);

    QDomElement markInTag = xmlDoc.createElement("markInFrame");
    markInTag.setAttribute("value", data->getMarkInFrameNumber());
    projectData.appendChild(markInTag);

    QDomElement markOutTag = xmlDoc.createElement("markOutFrame");
    markOutTag.setAttribute("value", data->getMarkOutFrameNumber());
    projectData.appendChild(markOutTag);

    root.appendChild(projectData);

    QDomElement objectElement = object->saveXML(xmlDoc);
    root.appendChild(objectElement);

    QDomElement versionElem = xmlDoc.createElement("version");
    versionElem.appendChild(xmlDoc.createTextNode(QString(APP_VERSION)));
    root.appendChild(versionElem);

    QString xmlContent;
    QTextStream xmlStream(&xmlContent, QIODevice::WriteOnly);
    xmlDoc.save(xmlStream, 2);

    if (!mDatabase.transaction())
    {
        dd << QString("Failed to begin transaction: %1").arg(mDatabase.lastError().text());
        return Status(Status::FAIL, dd);
    }

    Status saveStatus = saveMainDocumentXml(xmlContent);
    if (!saveStatus.ok())
    {
        mDatabase.rollback();
        return saveStatus;
    }

    if (!mDatabase.commit())
    {
        dd << QString("Failed to commit transaction: %1").arg(mDatabase.lastError().text());
        mDatabase.rollback();
        return Status(Status::FAIL, dd);
    }

    return Status::OK;
}

Status ProjectStorageBackendSqlite::incrementalSave(const Object* object)
{
    return saveProject(object);
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

    const QString createProjectDocument =
        "CREATE TABLE IF NOT EXISTS project_document ("
        "id INTEGER PRIMARY KEY CHECK(id = 1),"
        "app_version TEXT NOT NULL,"
        "saved_at_utc TEXT NOT NULL,"
        "main_xml TEXT NOT NULL"
        ");";

    if (!query.exec(createProjectDocument))
    {
        dd << QString("Failed to create project_document: %1").arg(query.lastError().text());
        return Status(Status::FAIL, dd);
    }

    return Status::OK;
}

Status ProjectStorageBackendSqlite::saveMainDocumentXml(const QString& xmlContent)
{
    DebugDetails dd;
    dd << "[SQLite backend saveMainDocumentXml]";

    QSqlQuery query(mDatabase);
    query.prepare(
        "INSERT INTO project_document (id, app_version, saved_at_utc, main_xml) "
        "VALUES (1, :appVersion, :savedAtUtc, :mainXml) "
        "ON CONFLICT(id) DO UPDATE SET "
        "app_version = excluded.app_version, "
        "saved_at_utc = excluded.saved_at_utc, "
        "main_xml = excluded.main_xml;");

    query.bindValue(":appVersion", QString(APP_VERSION));
    query.bindValue(":savedAtUtc", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    query.bindValue(":mainXml", xmlContent);

    if (!query.exec())
    {
        dd << QString("Failed to upsert project_document: %1").arg(query.lastError().text());
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
