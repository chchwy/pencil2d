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

#ifndef PROJECTSTORAGEBACKEND_H
#define PROJECTSTORAGEBACKEND_H

#include <QString>
#include "pencilerror.h"

class Object;

class ProjectStorageBackend
{
public:
    virtual ~ProjectStorageBackend() = default;

    virtual Status open(const QString& projectPath, bool createIfMissing) = 0;
    virtual Status close() = 0;

    virtual Object* loadProject() = 0;
    virtual Status saveProject(const Object* object) = 0;
    virtual Status incrementalSave(const Object* object) = 0;

    virtual Status verify() = 0;
};

#endif // PROJECTSTORAGEBACKEND_H
