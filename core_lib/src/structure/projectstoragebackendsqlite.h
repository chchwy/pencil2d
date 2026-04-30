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

#ifndef PROJECTSTORAGEBACKENDSQLITE_H
#define PROJECTSTORAGEBACKENDSQLITE_H

#include <QString>
#include <QSqlDatabase>
#include "projectstoragebackend.h"

class ProjectStorageBackendSqlite : public ProjectStorageBackend
{
public:
    ProjectStorageBackendSqlite();
    ~ProjectStorageBackendSqlite() override;

    Status open(const QString& projectPath, bool createIfMissing) override;
    Status close() override;

    Object* loadProject() override;
    Status saveProject(const Object* object) override;
    Status incrementalSave(const Object* object) override;

    Status verify() override;

private:
    Status ensureSchema();
    Status executePragma(const QString& pragmaSql);
    Status saveMainDocumentXml(const QString& xmlContent);

private:
    QString mConnectionName;
    QString mProjectPath;
    QSqlDatabase mDatabase;
};

#endif // PROJECTSTORAGEBACKENDSQLITE_H
