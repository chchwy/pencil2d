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
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QDomDocument>
#include <QTextStream>

#include "log.h"
#include "object.h"
#include "layer.h"

namespace {

constexpr int kCurrentSqliteSchemaVersion = 1;

void extractProjectData(const QDomElement& element, ObjectData& data)
{
    const QString tagName = element.tagName();
    if (tagName == "currentFrame")
    {
        data.setCurrentFrame(element.attribute("value").toInt());
    }
    else if (tagName == "currentColor")
    {
        const int r = element.attribute("r", "255").toInt();
        const int g = element.attribute("g", "255").toInt();
        const int b = element.attribute("b", "255").toInt();
        const int a = element.attribute("a", "255").toInt();
        data.setCurrentColor(QColor(r, g, b, a));
    }
    else if (tagName == "currentLayer")
    {
        data.setCurrentLayer(element.attribute("value", "0").toInt());
    }
    else if (tagName == "currentView")
    {
        const double m11 = element.attribute("m11", "1").toDouble();
        const double m12 = element.attribute("m12", "0").toDouble();
        const double m21 = element.attribute("m21", "0").toDouble();
        const double m22 = element.attribute("m22", "1").toDouble();
        const double dx = element.attribute("dx", "0").toDouble();
        const double dy = element.attribute("dy", "0").toDouble();
        data.setCurrentView(QTransform(m11, m12, m21, m22, dx, dy));
    }
    else if (tagName == "fps" || tagName == "currentFps")
    {
        data.setFrameRate(element.attribute("value", "12").toInt());
    }
    else if (tagName == "isLoop")
    {
        data.setLooping(element.attribute("value", "false") == "true");
    }
    else if (tagName == "isRangedPlayback")
    {
        data.setRangedPlayback(element.attribute("value", "false") == "true");
    }
    else if (tagName == "markInFrame")
    {
        data.setMarkInFrameNumber(element.attribute("value", "0").toInt());
    }
    else if (tagName == "markOutFrame")
    {
        data.setMarkOutFrameNumber(element.attribute("value", "15").toInt());
    }
}

ObjectData loadProjectData(const QDomElement& projectDataElem)
{
    ObjectData data;
    for (QDomNode node = projectDataElem.firstChild(); !node.isNull(); node = node.nextSibling())
    {
        const QDomElement element = node.toElement();
        if (element.isNull())
        {
            continue;
        }
        extractProjectData(element, data);
    }
    return data;
}

}

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
    DebugDetails dd;
    dd << "[SQLite backend loadProject]";

    if (!mDatabase.isOpen())
    {
        FILEMANAGER_LOG("SQLite loadProject failed: database is not open.");
        return nullptr;
    }

    QSqlQuery query(mDatabase);
    query.prepare("SELECT main_xml FROM project_document WHERE id = 1;");
    if (!query.exec())
    {
        FILEMANAGER_LOG("SQLite loadProject failed: cannot query project_document.");
        return nullptr;
    }

    if (!query.next())
    {
        FILEMANAGER_LOG("SQLite loadProject failed: no project document found.");
        return nullptr;
    }

    const QString xmlContent = query.value(0).toString();
    if (xmlContent.isEmpty())
    {
        FILEMANAGER_LOG("SQLite loadProject failed: empty XML content.");
        return nullptr;
    }

    QDomDocument xmlDoc;
    if (!xmlDoc.setContent(xmlContent))
    {
        FILEMANAGER_LOG("SQLite loadProject failed: invalid XML content.");
        return nullptr;
    }

    const QDomDocumentType type = xmlDoc.doctype();
    if (!(type.name() == "PencilDocument" || type.name() == "MyObject"))
    {
        FILEMANAGER_LOG("SQLite loadProject failed: invalid XML doctype.");
        return nullptr;
    }

    const QDomElement root = xmlDoc.documentElement();
    if (root.isNull() || root.tagName() != "document")
    {
        FILEMANAGER_LOG("SQLite loadProject failed: invalid XML root.");
        return nullptr;
    }

    std::unique_ptr<Object> object(new Object);
    object->setFilePath(mProjectPath);
    object->createWorkingDir();
    object->setDataDir(QDir(object->workingDir()).filePath("data"));
    object->loadDefaultPalette();

    const Status restoreStatus = restoreAssetFilesToDirectory(object->dataDir());
    if (!restoreStatus.ok())
    {
        FILEMANAGER_LOG("SQLite loadProject failed: unable to restore asset files.");
        return nullptr;
    }

    for (QDomNode node = root.firstChild(); !node.isNull(); node = node.nextSibling())
    {
        const QDomElement element = node.toElement();
        if (element.isNull())
        {
            continue;
        }

        if (element.tagName() == "object")
        {
            if (!object->loadXML(element, [] {}))
            {
                FILEMANAGER_LOG("SQLite loadProject failed: unable to load object payload.");
                return nullptr;
            }
        }
        else if (element.tagName() == "projectdata" || element.tagName() == "editor")
        {
            object->setData(loadProjectData(element));
        }
    }

    return object.release();
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

    Status assetStatus = flushObjectAssets(object);
    if (!assetStatus.ok())
    {
        return assetStatus;
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

    assetStatus = saveAssetFilesFromDirectory(object->dataDir());
    if (!assetStatus.ok())
    {
        mDatabase.rollback();
        return assetStatus;
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

    Status initStatus = initializeSchemaVersionTable();
    if (!initStatus.ok())
    {
        return initStatus;
    }

    QSqlQuery query(mDatabase);
    if (!query.exec("SELECT version FROM schema_version WHERE id = 1;"))
    {
        dd << QString("Failed to read schema version during ensureSchema: %1").arg(query.lastError().text());
        return Status(Status::FAIL, dd);
    }

    if (!query.next())
    {
        dd << "schema_version row is missing during ensureSchema.";
        return Status(Status::FAIL, dd);
    }

    const int schemaVersion = query.value(0).toInt();
    if (schemaVersion != kCurrentSqliteSchemaVersion)
    {
        Status migrationStatus = migrateSchema(schemaVersion);
        if (!migrationStatus.ok())
        {
            return migrationStatus;
        }
    }

    Status tableStatus = ensureProjectTables();
    if (!tableStatus.ok())
    {
        return tableStatus;
    }

    return validateSchemaVersion();
}

Status ProjectStorageBackendSqlite::initializeSchemaVersionTable()
{
    DebugDetails dd;
    dd << "[SQLite backend initializeSchemaVersionTable]";

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

    if (!query.exec(QString("INSERT OR IGNORE INTO schema_version (id, version) VALUES (1, %1);")
                    .arg(kCurrentSqliteSchemaVersion)))
    {
        dd << QString("Failed to initialize schema_version: %1").arg(query.lastError().text());
        return Status(Status::FAIL, dd);
    }

    return Status::OK;
}

Status ProjectStorageBackendSqlite::ensureProjectTables()
{
    DebugDetails dd;
    dd << "[SQLite backend ensureProjectTables]";

    QSqlQuery query(mDatabase);

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

    const QString createAssetFiles =
        "CREATE TABLE IF NOT EXISTS asset_files ("
        "path TEXT PRIMARY KEY,"
        "content BLOB NOT NULL"
        ");";

    if (!query.exec(createAssetFiles))
    {
        dd << QString("Failed to create asset_files: %1").arg(query.lastError().text());
        return Status(Status::FAIL, dd);
    }

    return Status::OK;
}

Status ProjectStorageBackendSqlite::migrateSchema(int fromVersion)
{
    DebugDetails dd;
    dd << QString("[SQLite backend migrateSchema] from=%1 to=%2")
              .arg(fromVersion)
              .arg(kCurrentSqliteSchemaVersion);

    if (fromVersion == kCurrentSqliteSchemaVersion)
    {
        return Status::OK;
    }

    if (fromVersion > kCurrentSqliteSchemaVersion)
    {
        dd << QString("Unsupported future schema version: %1").arg(fromVersion);
        return Status(Status::NOT_SUPPORTED, dd,
                      QObject::tr("SQLite Project Format"),
                      QObject::tr("This SQLite project uses schema version %1, but this build supports version %2.")
                          .arg(fromVersion)
                          .arg(kCurrentSqliteSchemaVersion));
    }

    if (fromVersion == 0)
    {
        Status tableStatus = ensureProjectTables();
        if (!tableStatus.ok())
        {
            return tableStatus;
        }

        QSqlQuery query(mDatabase);
        if (!query.exec(QString("UPDATE schema_version SET version = %1 WHERE id = 1;")
                        .arg(kCurrentSqliteSchemaVersion)))
        {
            dd << QString("Failed to upgrade schema version to %1: %2")
                      .arg(kCurrentSqliteSchemaVersion)
                      .arg(query.lastError().text());
            return Status(Status::FAIL, dd);
        }

        return Status::OK;
    }

    dd << QString("No migration path from schema version %1 to %2.")
              .arg(fromVersion)
              .arg(kCurrentSqliteSchemaVersion);
    return Status(Status::NOT_SUPPORTED, dd,
                  QObject::tr("SQLite Project Format"),
                  QObject::tr("This SQLite project uses older schema version %1, and migration is not implemented yet.")
                      .arg(fromVersion));
}

Status ProjectStorageBackendSqlite::validateSchemaVersion()
{
    DebugDetails dd;
    dd << "[SQLite backend validateSchemaVersion]";

    QSqlQuery query(mDatabase);
    if (!query.exec("SELECT version FROM schema_version WHERE id = 1;"))
    {
        dd << QString("Failed to read schema version: %1").arg(query.lastError().text());
        return Status(Status::FAIL, dd);
    }

    if (!query.next())
    {
        dd << "schema_version row is missing.";
        return Status(Status::FAIL, dd,
                      QObject::tr("SQLite Project Format"),
                      QObject::tr("This SQLite project is missing schema version information."));
    }

    const int schemaVersion = query.value(0).toInt();
    if (schemaVersion == kCurrentSqliteSchemaVersion)
    {
        return Status::OK;
    }

    return migrateSchema(schemaVersion);
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

Status ProjectStorageBackendSqlite::flushObjectAssets(const Object* object)
{
    DebugDetails dd;
    dd << "[SQLite backend flushObjectAssets]";

    const int layerCount = object->getLayerCount();
    QStringList attachedFiles;
    for (int i = 0; i < layerCount; ++i)
    {
        Layer* layer = object->getLayer(i);
        if (layer == nullptr)
        {
            continue;
        }

        Status presaveStatus = layer->presave(object->dataDir());
        if (!presaveStatus.ok())
        {
            dd.collect(presaveStatus.details());
            return Status(Status::FAIL, dd);
        }

        Status saveStatus = layer->save(object->dataDir(), attachedFiles, [] {});
        if (!saveStatus.ok())
        {
            dd.collect(saveStatus.details());
            return Status(Status::FAIL, dd);
        }
    }

    return Status::OK;
}

Status ProjectStorageBackendSqlite::saveAssetFilesFromDirectory(const QString& dataDirPath)
{
    DebugDetails dd;
    dd << "[SQLite backend saveAssetFilesFromDirectory]";

    QSqlQuery clearQuery(mDatabase);
    if (!clearQuery.exec("DELETE FROM asset_files;"))
    {
        dd << QString("Failed to clear asset_files: %1").arg(clearQuery.lastError().text());
        return Status(Status::FAIL, dd);
    }

    QDir baseDir(dataDirPath);
    if (!baseDir.exists())
    {
        return Status::OK;
    }

    QSqlQuery insertQuery(mDatabase);
    insertQuery.prepare(
        "INSERT INTO asset_files (path, content) VALUES (:path, :content) "
        "ON CONFLICT(path) DO UPDATE SET content = excluded.content;");

    QDirIterator it(dataDirPath, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        const QString absolutePath = it.next();
        const QString relativePath = baseDir.relativeFilePath(absolutePath);

        QFile file(absolutePath);
        if (!file.open(QIODevice::ReadOnly))
        {
            dd << QString("Cannot read asset file: %1").arg(absolutePath);
            return Status(Status::ERROR_FILE_CANNOT_OPEN, dd);
        }

        const QByteArray content = file.readAll();
        file.close();

        insertQuery.bindValue(":path", relativePath);
        insertQuery.bindValue(":content", content);
        if (!insertQuery.exec())
        {
            dd << QString("Failed to store asset '%1': %2").arg(relativePath, insertQuery.lastError().text());
            return Status(Status::FAIL, dd);
        }
    }

    return Status::OK;
}

Status ProjectStorageBackendSqlite::restoreAssetFilesToDirectory(const QString& dataDirPath)
{
    DebugDetails dd;
    dd << "[SQLite backend restoreAssetFilesToDirectory]";

    if (!QDir().mkpath(dataDirPath))
    {
        dd << QString("Cannot create data directory: %1").arg(dataDirPath);
        return Status(Status::FAIL, dd);
    }

    QSqlQuery query(mDatabase);
    if (!query.exec("SELECT path, content FROM asset_files;"))
    {
        dd << QString("Failed to load asset files: %1").arg(query.lastError().text());
        return Status(Status::FAIL, dd);
    }

    while (query.next())
    {
        const QString relativePath = query.value(0).toString();
        const QByteArray content = query.value(1).toByteArray();

        const QString fullPath = QDir(dataDirPath).filePath(relativePath);
        QFileInfo fileInfo(fullPath);
        if (!QDir().mkpath(fileInfo.path()))
        {
            dd << QString("Cannot create asset subdirectory: %1").arg(fileInfo.path());
            return Status(Status::FAIL, dd);
        }

        QFile file(fullPath);
        if (!file.open(QIODevice::WriteOnly))
        {
            dd << QString("Cannot write asset file: %1").arg(fullPath);
            return Status(Status::ERROR_FILE_CANNOT_OPEN, dd);
        }

        if (file.write(content) != content.size())
        {
            file.close();
            dd << QString("Failed to write full content to: %1").arg(fullPath);
            return Status(Status::FAIL, dd);
        }

        file.close();
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
