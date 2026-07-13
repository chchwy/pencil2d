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
#ifndef WORKINGDIRECTORY_H
#define WORKINGDIRECTORY_H

#include <memory>

#include <QString>

class QLockFile;

/** The temporary folder a project is unpacked into for editing.
 *
 *  Owns a per-process lock file so other instances (and the startup
 *  recovery scan) can tell the directory belongs to a live process.
 *  The directory is deleted when the object is destroyed. */
class WorkingDirectory
{
public:
    WorkingDirectory();
    ~WorkingDirectory();

    WorkingDirectory(const WorkingDirectory&) = delete;
    WorkingDirectory& operator=(const WorkingDirectory&) = delete;

    /** Creates a fresh temp/Pencil2D/<project>_<ext>_<random>/ folder with
     *  a data/ subdirectory, and locks it as owned by this process. */
    void create(const QString& projectName);

    /** Adopts an existing directory (e.g. crash recovery) and locks it. */
    void setPath(const QString& path);

    /** Releases the lock and deletes the directory recursively. */
    void remove();

    QString path() const { return mPath; }

    QString dataPath() const { return mDataPath; }
    void setDataPath(const QString& path) { mDataPath = path; }

private:
    void lockPath(const QString& path);

    QString mPath;
    QString mDataPath;

    // Held for the lifetime of the working dir so other instances (and the
    // startup recovery scan) can tell that the dir belongs to a live process.
    std::unique_ptr<QLockFile> mLock;
};

#endif // WORKINGDIRECTORY_H
