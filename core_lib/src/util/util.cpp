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
#include "util.h"
#include <QAbstractSpinBox>
#include <QDebug>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include <windows.h>
#include <io.h>
#else
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#endif

static inline bool clipLineToEdge(qreal& t0, qreal& t1, qreal p, qreal q)
{
    if (p < 0) { // Line entering the clipping window
        t0 = qMax(t0, q / p);
        return t0 < t1;
    }
    if (p > 0) { // Line leaving the clipping window
        t1 = qMin(t1, q / p);
        return t0 < t1;
    }
    return q >= 0;
}

QLineF clipLine(const QLineF& line, const QRect& clip, qreal t0, qreal t1)
{
    int left = clip.left(), right = left + clip.width(), top = clip.top(), bottom = top + clip.height();
    qreal x1 = line.x1(), x2 = line.x2(), dx = line.dx(), y1 = line.y1(), y2 = line.y2(), dy = line.dy();

    if ((t0 == 0 && t1 == 1 && ((x1 < left && x2 < left) ||
                                (x1 > right && x2 > right) ||
                                (y1 < top && y2 < top) ||
                                (y1 > bottom && y2 > bottom))) ||
        !clipLineToEdge(t0, t1, -dx, x1 - left) ||
        !clipLineToEdge(t0, t1,  dx, right - x1) ||
        !clipLineToEdge(t0, t1, -dy, y1 - top) ||
        !clipLineToEdge(t0, t1,  dy, bottom - y1)) {
        return {};
    }

    Q_ASSERT(t0 < t1);
    return {line.x1() + line.dx() * t0,
            line.y1() + line.dy() * t0,
            line.x1() + line.dx() * t1,
            line.y1() + line.dy() * t1};
}

void clearFocusOnFinished(QAbstractSpinBox *spinBox)
{
    QObject::connect(spinBox, &QAbstractSpinBox::editingFinished, spinBox, &QAbstractSpinBox::clearFocus);
}

QString ffprobeLocation()
{
#ifdef _WIN32
    return QApplication::applicationDirPath() + "/plugins/ffprobe.exe";
#elif __APPLE__
    return QApplication::applicationDirPath() + "/plugins/ffprobe";
#else
    QString ffprobePath = QStandardPaths::findExecutable(
        "ffprobe",
        QStringList()
        << QApplication::applicationDirPath() + "/plugins"
        << QApplication::applicationDirPath() + "/../plugins" // linuxdeployqt in FHS-like mode
    );
    if (!ffprobePath.isEmpty())
    {
        return ffprobePath;
    }
    return QStandardPaths::findExecutable("ffprobe"); // ffprobe is a standalone project.
#endif
}

QString ffmpegLocation()
{
#ifdef _WIN32
    return QApplication::applicationDirPath() + "/plugins/ffmpeg.exe";
#elif __APPLE__
    return QApplication::applicationDirPath() + "/plugins/ffmpeg";
#else
    QString ffmpegPath = QStandardPaths::findExecutable(
        "ffmpeg",
        QStringList()
        << QApplication::applicationDirPath() + "/plugins"
        << QApplication::applicationDirPath() + "/../plugins" // linuxdeployqt in FHS-like mode
    );
    if (!ffmpegPath.isEmpty())
    {
        return ffmpegPath;
    }
    return QStandardPaths::findExecutable("ffmpeg"); // ffmpeg is a standalone project.
#endif
}

quint64 imageSize(const QImage& img)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    return img.sizeInBytes();
#else
    return img.byteCount();
#endif
}

QString uniqueString(int len)
{
    static const char alphanum[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    const int alphanumLen = sizeof(alphanum);

    if (len > 128) len = 128;

    char s[128 + 1];
    for (int i = 0; i < len; ++i)
    {
        s[i] = alphanum[rand() % (alphanumLen - 1)];
    }
    s[len] = 0;
    return QString::fromUtf8(s);
}

QString closestCanonicalPath(const QString& path)
{
    QString origPath = QDir(path).absolutePath();

    // Iterate up the path until an existing file/directory is found
    QFileInfo existingSubpath(origPath);
    // Symlinks must be checked for separately because exists checks if the target of the symlink exists, not the symlink itself
    while (!existingSubpath.isRoot() && !existingSubpath.exists() && !existingSubpath.isSymbolicLink())
    {
        // Move up one directory logically
        existingSubpath.setFile(existingSubpath.dir().absolutePath());
    }

    // Resolve symlinks for all existing parts of the path
    QString canonicalPath = existingSubpath.canonicalFilePath();
    if (canonicalPath.isEmpty())
    {
        // This can happen if there is a dangling symlink in the path
        return QString();
    }

    // Combine existing canonical path with non-existing path segment
    QString finalPath = QDir(canonicalPath).filePath(QDir(existingSubpath.absoluteFilePath()).relativeFilePath(origPath));

    return QDir(finalPath).absolutePath();
}

QString validateDataPath(const QString& filePath, const QString& dataDirPath)
{
    // Make sure src path is relative
    if (!QFileInfo(filePath).isRelative()) return QString();

    // Get canonical path of data dir and file for comparison
    QString canonicalDataDirPath = closestCanonicalPath(dataDirPath);
    QString canonicalFilePath = closestCanonicalPath(QDir(dataDirPath).filePath(filePath));

    // Bail out if either canonical path could not be resolved (e.g. dangling symlinks)
    if (canonicalDataDirPath.isEmpty() || canonicalFilePath.isEmpty())
    {
        qWarning() << "validateDataPath: failed to resolve canonical path for:" << filePath;
        return QString();
    }

    // Ensure the data dir path ends with a separator so that a prefix match
    // cannot falsely succeed against a sibling directory with a similar name
    // (e.g. /tmp/data matching /tmp/dataevil/...)
    if (!canonicalDataDirPath.endsWith('/'))
        canonicalDataDirPath.append('/');

    // Use case-insensitive comparison on filesystems that are case-insensitive
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    const Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
    const Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif
    if (canonicalFilePath.startsWith(canonicalDataDirPath, cs))
    {
        return canonicalFilePath;
    }

    // If canonicalFilePath does not start with the canonicalDataDirPath, then symlinks or '..' have made
    // the file resolve outside of the data directory and the file should not be loaded.
    qWarning() << "validateDataPath: rejected path outside data directory:" << filePath;
    return QString();
}

/**
 * Flushes the file at the given path to stable storage.
 *
 * Opening for read-write is required: POSIX allows fsync to fail on
 * read-only descriptors and FlushFileBuffers needs write access.
 */
static bool syncFileToDisk(const QString& path, QString& errorOut)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadWrite))
    {
        errorOut = QString("Cannot open file for syncing: %1").arg(file.errorString());
        return false;
    }

#ifdef Q_OS_WIN
    HANDLE handle = reinterpret_cast<HANDLE>(_get_osfhandle(file.handle()));
    if (handle == INVALID_HANDLE_VALUE || !FlushFileBuffers(handle))
    {
        errorOut = QString("FlushFileBuffers failed with error code %1").arg(GetLastError());
        return false;
    }
#else
#ifdef Q_OS_MAC
    // On macOS, fsync only flushes to the drive, not through the drive's
    // cache; F_FULLFSYNC is the real durability barrier.
    if (fcntl(file.handle(), F_FULLFSYNC) != 0 && fsync(file.handle()) != 0)
#else
    if (fsync(file.handle()) != 0)
#endif
    {
        errorOut = QString("fsync failed: %1").arg(QString::fromLocal8Bit(strerror(errno)));
        return false;
    }
#endif
    return true;
}

Status atomicReplace(const QString& tmpPath, const QString& finalPath)
{
    DebugDetails dd;
    dd << QString("Atomic replace: %1 -> %2").arg(tmpPath, finalPath);

    if (!QFile::exists(tmpPath))
    {
        dd << "Error: temporary file does not exist";
        return Status(Status::FILE_NOT_FOUND, dd);
    }

    QString syncError;
    if (!syncFileToDisk(tmpPath, syncError))
    {
        dd << QString("Error: %1").arg(syncError);
        return Status(Status::FAIL, dd);
    }

#ifdef Q_OS_WIN
    const std::wstring tmpNative = QDir::toNativeSeparators(tmpPath).toStdWString();
    const std::wstring finalNative = QDir::toNativeSeparators(finalPath).toStdWString();
    if (!MoveFileExW(tmpNative.c_str(), finalNative.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        dd << QString("Error: MoveFileExW failed with error code %1").arg(GetLastError());
        return Status(Status::FAIL, dd);
    }
#else
    if (::rename(QFile::encodeName(tmpPath).constData(),
                 QFile::encodeName(finalPath).constData()) != 0)
    {
        dd << QString("Error: rename failed: %1").arg(QString::fromLocal8Bit(strerror(errno)));
        return Status(Status::FAIL, dd);
    }

    // Sync the parent directory so the rename itself is durable.
    const QByteArray dirPath = QFile::encodeName(QFileInfo(finalPath).absolutePath());
    int dirFd = ::open(dirPath.constData(), O_RDONLY);
    if (dirFd >= 0)
    {
        fsync(dirFd);
        ::close(dirFd);
    }
#endif

    dd << "Atomic replace succeeded";
    return Status(Status::OK, dd);
}
