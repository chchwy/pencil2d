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
#include "workingdirectory.h"

#include <QDebug>
#include <QDir>
#include <QLockFile>

#include "fileformat.h"
#include "pencildef.h"
#include "util.h"

WorkingDirectory::WorkingDirectory() = default;

WorkingDirectory::~WorkingDirectory()
{
    remove();
}

void WorkingDirectory::create(const QString& projectName)
{
    QDir dir(QDir::tempPath());

    QString strWorkingDir;
    do
    {
        strWorkingDir = QString("%1/Pencil2D/%2_%3_%4/").arg(QDir::tempPath(),
                                                             projectName,
                                                             PFF_TMP_DECOMPRESS_EXT,
                                                             uniqueString(8));
    }
    while(dir.exists(strWorkingDir));

    dir.mkpath(strWorkingDir);
    mPath = strWorkingDir;

    lockPath(strWorkingDir);

    QDir dataDir(strWorkingDir + PFF_DATA_DIR);
    dataDir.mkpath(".");

    mDataPath = dataDir.absolutePath();
}

void WorkingDirectory::setPath(const QString& path)
{
    if (!QDir(path).exists())
    {
        qWarning() << "WorkingDirectory::setPath: directory does not exist:" << path;
    }
    mPath = path;

    // Take ownership of an adopted working dir (e.g. a recovered project).
    lockPath(path);
}

void WorkingDirectory::lockPath(const QString& path)
{
    // Mark the working dir as owned by this process, so that the startup
    // recovery scan of other instances leaves it alone.
    mLock.reset(new QLockFile(QDir(path).filePath(PFF_WORKING_DIR_LOCK_FILE)));
    mLock->setStaleLockTime(0); // stale = owning process is gone, never by age
    if (!mLock->tryLock(0))
    {
        qWarning() << "Could not lock the working directory:" << path;
    }
}

void WorkingDirectory::remove()
{
    // Release the lock before deleting the dir, otherwise the open lock
    // file keeps the directory from being removed on Windows.
    mLock.reset();

    if (!mPath.isEmpty())
    {
        QDir dir(mPath);
        if (!dir.removeRecursively())
        {
            // Not fatal: the temp dir is leaked and will be cleaned up
            // by a later startup scan.
            qWarning() << "Could not remove the working directory:" << mPath;
        }
        mPath.clear();
        mDataPath.clear();
    }
}
