#ifndef MPBRUSHUTILS_H
#define MPBRUSHUTILS_H

#include <QString>
#include <QMap>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QDirIterator>
#include <QJsonObject>

#include <QPixmap>

#include <QDebug>

#include "pencilerror.h"
#include "brushsetting.h"

static const QString BRUSH_CONTENT_EXT = ".myb";
static const QString BRUSH_PREVIEW_EXT = "_prev.png";
static const QString BRUSH_CONFIG = "brushes.conf";
static const QString BRUSH_QRC = ":brushes";
static const int ICON_SZ = 64;
static const QString BRUSH_COPY_POSTFIX = "_clone";

struct MPBrushInfo {
    QString comment = "";
    qreal version = 0.0;

    void write(QJsonObject& object) const
    {
        QJsonObject::iterator commentObjIt = object.find("comment");

        QString commentId = "comment";
        if (commentObjIt->isUndefined()) {
            object.insert(commentId, comment);
        } else {
            object.remove(commentId);
            object.insert(commentId, comment);
        }

        QString brushVersionKey = "brush-version";
        QJsonObject::iterator versionObjIt = object.find("brush-version");
        if (versionObjIt->isUndefined()) {
            object.insert(brushVersionKey, version);
        } else {
            object.remove(brushVersionKey);
            object.insert(brushVersionKey, version);
        }
    }

    MPBrushInfo read(QJsonObject& object) {
        QJsonObject::iterator commentObjIt = object.find("comment");

        MPBrushInfo info;
        QString commentId = "comment";
        if (!commentObjIt->isUndefined()) {
            info.comment = object.value(commentId).toString();
        }

        QString brushVersionKey = "brush-version";
        QJsonObject::iterator versionObjIt = object.find(brushVersionKey);
        if (!versionObjIt->isUndefined()) {
            info.version = object.value(brushVersionKey).toDouble();
        }

        return info;
    }
};

struct MPBrushParser {

    /// Parses the mypaint brush config ".conf" format and returns a map of the brush groups
    static QMap<QString, QStringList> parseConfig(QFile& file, QString brushesPath)
    {
        QString currentGroup;
        QStringList brushesGroup;
        QMap<QString, QStringList> brushes;

        while (!file.atEnd())
        {
            QString line ( file.readLine().trimmed() );
            if (line.isEmpty() || line.startsWith("#")) continue;
            if (line.startsWith("Group:"))
            {
                // first, we store the last brushesGroup (if any). Note that declaring 2 groups with the same name is wrong (only the last one will be visible)
                if (!currentGroup.isEmpty() && !brushesGroup.isEmpty()) {
                    brushes.insert(currentGroup, brushesGroup);
                }

                currentGroup = line.section(':',1).trimmed(); // Get the name after the first ':' separator
                brushesGroup.clear();
                continue;
            }

            if (QFileInfo(brushesPath + QDir::separator() + line + BRUSH_CONTENT_EXT).isReadable()) {
                brushesGroup << line;
            }

            if (!currentGroup.isEmpty() && !brushesGroup.isEmpty()) brushes.insert(currentGroup, brushesGroup);
        }
        return brushes;
    }

    /// Copy internal brush resources to app data folder
    /// This is where brushes will be loaded from in the future.
    static Status copyResourcesToAppData()
    {
        QString appDataBrushesPath = getBrushesPath();
        QDir dir(appDataBrushesPath);

        // Brush folder exists, no need to copy resources again
        if (dir.exists()) {
            return Status::OK;
        } else {
            dir.mkdir(appDataBrushesPath);
        }

        dir.setPath(BRUSH_QRC);

        QString internalBrushConfigPath = QString(BRUSH_QRC) + QDir::separator() + BRUSH_CONFIG;

        QFile appDataFile(appDataBrushesPath+QDir::separator()+BRUSH_CONFIG);
        if (!appDataFile.exists()) {
            QFile resFile(internalBrushConfigPath);
            resFile.copy(appDataFile.fileName());

            // make sure file has read and write access
            appDataFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        }

        QStringList content = dir.entryList();

        for (QString entry : content) {

            // Ignore config file, we've already copied it
            if (entry.contains(".conf")) {
                continue;
            }

            QDir internalBrushDir(QString(BRUSH_QRC) + QDir::separator() +entry);
            QDir externalBrushDir(appDataBrushesPath + QDir::separator() +entry);
            if (!externalBrushDir.exists()) {
                externalBrushDir.mkpath(appDataBrushesPath + QDir::separator() + entry);
            }

            QStringList dirContent = internalBrushDir.entryList();
            for (QString entryDown : dirContent) {
                QFile internalBrushFile(internalBrushDir.path() + QDir::separator() +entryDown);
                QFile brushFile(externalBrushDir.path() + QDir::separator() + entryDown);

                if (!brushFile.exists()) {
                    internalBrushFile.copy(brushFile.fileName());
                    brushFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
                }

            }
        }

        return Status::OK;
    }

    static QString getBrushSettingIdentifier(const BrushSettingType& type)
    {
        switch(type)
        {
        case BrushSettingType::BRUSH_SETTING_OPAQUE:                      return "opaque";
        case BrushSettingType::BRUSH_SETTING_OPAQUE_MULTIPLY:             return "opaque_multiply";
        case BrushSettingType::BRUSH_SETTING_OPAQUE_LINEARIZE:            return "opaque_linearize";
        case BrushSettingType::BRUSH_SETTING_RADIUS_LOGARITHMIC:          return "radius_logarithmic";
        case BrushSettingType::BRUSH_SETTING_HARDNESS:                    return "hardness";
        case BrushSettingType::BRUSH_SETTING_SOFTNESS:                    return "softness";
        case BrushSettingType::BRUSH_SETTING_ANTI_ALIASING:               return "anti_aliasing";
        case BrushSettingType::BRUSH_SETTING_DABS_PER_BASIC_RADIUS:       return "dabs_per_basic_radius";
        case BrushSettingType::BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS:      return "dabs_per_actual_radius";
        case BrushSettingType::BRUSH_SETTING_DABS_PER_SECOND:             return "dabs_per_second";
        case BrushSettingType::BRUSH_SETTING_GRIDMAP_SCALE:               return "gridmap_scale";
        case BrushSettingType::BRUSH_SETTING_GRIDMAP_SCALE_X:             return "gridmap_scale_x";
        case BrushSettingType::BRUSH_SETTING_GRIDMAP_SCALE_Y:             return "gridmap_scale_y";
        case BrushSettingType::BRUSH_SETTING_RADIUS_BY_RANDOM:            return "radius_by_random";
        case BrushSettingType::BRUSH_SETTING_SPEED1_SLOWNESS:             return "speed1_slowness";
        case BrushSettingType::BRUSH_SETTING_SPEED2_SLOWNESS:             return "speed2_slowness";
        case BrushSettingType::BRUSH_SETTING_SPEED1_GAMMA:                return "speed1_gamma";
        case BrushSettingType::BRUSH_SETTING_SPEED2_GAMMA:                return "speed2_gamma";
        case BrushSettingType::BRUSH_SETTING_OFFSET_BY_RANDOM:            return "offset_by_random";
        case BrushSettingType::BRUSH_SETTING_OFFSET_Y:                    return "offset_y";
        case BrushSettingType::BRUSH_SETTING_OFFSET_X:                    return "offset_x";
        case BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE:                return "offset_angle";
        case BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_ASC:            return "offset_angle_asc";
        case BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_2:              return "offset_angle_2";
        case BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_2_ASC:          return "offset_angle_2_asc";
        case BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_ADJ:            return "offset_angle_adj";
        case BrushSettingType::BRUSH_SETTING_OFFSET_MULTIPLIER:           return "offset_multiplier";
        case BrushSettingType::BRUSH_SETTING_OFFSET_BY_SPEED:             return "offset_by_speed";
        case BrushSettingType::BRUSH_SETTING_OFFSET_BY_SPEED_SLOWNESS:    return "offset_by_speed_slowness";
        case BrushSettingType::BRUSH_SETTING_SLOW_TRACKING:               return "slow_tracking";
        case BrushSettingType::BRUSH_SETTING_SLOW_TRACKING_PER_DAB:       return "slow_tracking_per_dab";
        case BrushSettingType::BRUSH_SETTING_TRACKING_NOISE:              return "tracking_noise";
        case BrushSettingType::BRUSH_SETTING_COLOR_H:                     return "color_h";
        case BrushSettingType::BRUSH_SETTING_COLOR_S:                     return "color_s";
        case BrushSettingType::BRUSH_SETTING_COLOR_V:                     return "color_v";
        case BrushSettingType::BRUSH_SETTING_RESTORE_COLOR:               return "restore_color";
        case BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_H:              return "change_color_h";
        case BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_L:              return "change_color_l";
        case BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_HSL_S:          return "change_color_hsl_s";
        case BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_V:              return "change_color_v";
        case BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_HSV_S:          return "change_color_hsv_s";
        case BrushSettingType::BRUSH_SETTING_SMUDGE:                      return "smudge";
        case BrushSettingType::BRUSH_SETTING_SMUDGE_LENGTH:               return "smudge_length";
        case BrushSettingType::BRUSH_SETTING_SMUDGE_RADIUS_LOG:           return "smudge_radius_log";
        case BrushSettingType::BRUSH_SETTING_ERASER:                      return "eraser";
        case BrushSettingType::BRUSH_SETTING_STROKE_THRESHOLD:            return "stroke_threshold";
        case BrushSettingType::BRUSH_SETTING_STROKE_DURATION_LOGARITHMIC: return "stroke_duration_logarithmic";
        case BrushSettingType::BRUSH_SETTING_STROKE_HOLDTIME:             return "stroke_holdtime";
        case BrushSettingType::BRUSH_SETTING_CUSTOM_INPUT:                return "custom_input";
        case BrushSettingType::BRUSH_SETTING_CUSTOM_INPUT_SLOWNESS:       return "custom_input_slowness";
        case BrushSettingType::BRUSH_SETTING_ELLIPTICAL_DAB_RATIO:        return "elliptical_dab_ratio";
        case BrushSettingType::BRUSH_SETTING_ELLIPTICAL_DAB_ANGLE:        return "elliptical_dab_angle";
        case BrushSettingType::BRUSH_SETTING_DIRECTION_FILTER:            return "direction_filter";
        case BrushSettingType::BRUSH_SETTING_LOCK_ALPHA:                  return "lock_alpha";
        case BrushSettingType::BRUSH_SETTING_COLORIZE:                    return "colorize";
        case BrushSettingType::BRUSH_SETTING_SNAP_TO_PIXEL:               return "snap_to_pixel";
        case BrushSettingType::BRUSH_SETTING_PRESSURE_GAIN_LOG:           return "pressure_gain_log";
        default: return "";
        }
    }

    static QString getBrushPreviewImagePath(const QString& brushPreset, const QString brushName)
    {
        const QString& brushPath = getBrushesPath() + QDir::separator() + brushPreset + QDir::separator() + brushName;
        QFile file(brushPath+BRUSH_PREVIEW_EXT);

        if (file.exists()) {
            return file.fileName();
        }
        return "";
    }

    static Status::StatusData readBrushFromFile(const QString& brushPreset, const QString& brushName)
    {
        const QString& brushPath = getBrushesPath() + QDir::separator() + brushPreset + QDir::separator() + brushName;

        QFile file(brushPath + BRUSH_CONTENT_EXT);

        // satefy measure:
        // if no local brush file exists, look at internal resources
        // local brush files will overwrite the existence of the internal one
        if (!file.exists()) {
            file.setFileName(BRUSH_QRC + QDir::separator() + brushPreset + QDir::separator() + brushName + BRUSH_CONTENT_EXT);
        }

        auto status = Status::StatusData();
        if (file.open( QIODevice::ReadOnly ))
        {
            QByteArray content = file.readAll();
            status.data = content;
            status.errorcode = Status::OK;
        } else {
            status.errorcode = Status::FAIL;
        }
        return status;
    }

    static QString getBrushesPath()
    {   
        QStringList pathList = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
        return pathList.first() + "/" + "brushes";
    }

    static QString getBrushPath(const QString& brushPreset, const QString& brushName, const QString& extension)
    {
        QString brushPath = getBrushesPath() + QDir::separator() + brushPreset + QDir::separator() + brushName;

        QFile file(brushPath+extension);
        if (!file.exists()) {
            brushPath = QString(BRUSH_QRC) + QDir::separator() + brushPreset + QDir::separator() + brushName;
        }
        return brushPath + extension;
    }

    static QString getBrushImagePath(const QString& brushPreset, const QString& brushName)
    {
        return getBrushPath(brushPreset, brushName, BRUSH_PREVIEW_EXT);
    }

    static QString getBrushConfigPath(const QString extension = "")
    {
        QString brushPath = getBrushesPath() + QDir::separator() + extension;

        QFile file(brushPath);
        if (!file.exists()) {
            brushPath = QString(BRUSH_QRC) + QDir::separator() + extension;
        }
        return brushPath;
    }

    static Status writeBrushToFile(const QString& brushPreset, const QString& brushName, const QByteArray& data)
    {
        Status status = Status::OK;
        const QString& groupPath = getBrushesPath() + QDir::separator() + brushPreset + QDir::separator();
        const QString& brushPath = getBrushesPath() + QDir::separator() + brushPreset + QDir::separator() + brushName;

        QFile file(brushPath + BRUSH_CONTENT_EXT);

        QDir dir(groupPath);
        if (!dir.exists()) {
            dir.mkpath(groupPath);
        }

        file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);

        if (file.error() == QFile::NoError) {
            file.write(data);
            file.close();

            status = Status::OK;
        } else {
            status.setTitle(QObject::tr("Write error:"));
            status.setDescription(file.errorString());
            status = Status::FAIL;

        }
        return status;
    }

    static Status renameMoveBrushFileIfNeeded(QString originalPreset, QString originalName, QString newPreset, QString newName)
    {

        Status status = Status::OK;
        const QString groupPath = getBrushesPath() + QDir::separator() + newPreset;
        const QString brushPath = getBrushesPath() + QDir::separator() + newPreset + QDir::separator() + newName;

        const QString oldGroupPath = getBrushesPath() + QDir::separator() + originalPreset;
        const QString oldBrushPath = getBrushesPath() + QDir::separator() + originalPreset + QDir::separator() + originalName;

        QDir presetDir(groupPath);

        QString absoluteOldBrushPath = oldBrushPath + BRUSH_CONTENT_EXT;
        QString absoluteOldBrushPreviewPath = oldBrushPath + BRUSH_PREVIEW_EXT;

        QString absoluteBrushPath = brushPath + BRUSH_CONTENT_EXT;
        QString absoluteBrushPreviewPath = brushPath+ BRUSH_PREVIEW_EXT;
        if (!presetDir.exists()) {
            bool pathCreated = presetDir.mkpath(groupPath);

            if (!pathCreated) {
                status = Status::FAIL;

                status.setTitle(QObject::tr("Something went wrong"));
                status.setDescription(QObject::tr("Couldn't create preset dir, verify that the folder is writable"));
                return status;
            }
        }

        QFile brushFile(brushPath);
        if (!brushFile.exists()) {

            QFile moveFile(oldBrushPath);
            moveFile.rename(absoluteOldBrushPath, absoluteBrushPath);

            if (moveFile.error() != QFile::NoError) {
                status = Status::FAIL;

                status.setTitle(QObject::tr("Something went wrong"));
                status.setDescription(QObject::tr("Failed to rename or move: ") + moveFile.fileName() + QObject::tr(" verify that the folder is writable")
                                      + QObject::tr("The following error was given: ") + moveFile.errorString());
                return status;
            }
        }

        QFile brushImageFile(absoluteBrushPreviewPath);
        if (!brushImageFile.exists()) {
            QFile moveImageFile(absoluteOldBrushPreviewPath);
            moveImageFile.rename(absoluteOldBrushPreviewPath, absoluteBrushPreviewPath);

            if (moveImageFile.error() != QFile::NoError) {
                status = Status::FAIL;

                status.setTitle(QObject::tr("Something went wrong"));
                status.setDescription(QObject::tr("Failed to rename or move: ") + moveImageFile.fileName() + QObject::tr(" verify that the folder is writable")
                                      + QObject::tr("The following error was given: ") + moveImageFile.errorString());
                return status;
            }

        }

        return status;
    }

    static Status writeBrushIcon(const QPixmap& iconPix, const QString brushPreset, const QString brushName) {
        Status status = Status::OK;

        const QString brushPath = getBrushesPath() + QDir::separator() + brushPreset + QDir::separator() + brushName;

        const QString brushFileName = brushPath+BRUSH_PREVIEW_EXT;
        if (iconPix.save(brushPath+BRUSH_PREVIEW_EXT) == false) {
            status = Status::FAIL;
            status.setTitle(QObject::tr("Error saving brushImage"));
            status.setDescription(QObject::tr("Failed to save: ") + brushFileName);
        }
        return status;
    }

    static Status copyRenameBrushFileIfNeeded(const QString& originalPreset, const QString& originalName, const QString& newPreset, QString& newName)
    {
        QString groupPath = getBrushesPath() + QDir::separator() + newPreset;
        QString brushPath = getBrushesPath() + QDir::separator() + newPreset + QDir::separator() + newName;

        QString oldGroupPath = getBrushesPath() + QDir::separator() + originalPreset;
        QString oldBrushPath = getBrushesPath() + QDir::separator() + originalPreset + QDir::separator() + originalName;

        Status status = Status::OK;
        QFile file(brushPath + BRUSH_CONTENT_EXT);
        QFile fileImage(brushPath + BRUSH_PREVIEW_EXT);

        QDir dir(groupPath);
        if (!dir.exists()) {
            bool pathCreated = dir.mkpath(groupPath);

            if (!pathCreated) {
                status = Status::FAIL;

                status.setTitle(QObject::tr("Something went wrong"));
                status.setDescription(QObject::tr("Couldn't create dir: ") + dir.path() + QObject::tr("verify that the folder is writable"));
                return status;
            }
        }

        if (file.error() != QFile::NoError || fileImage.error() != QFile::NoError) {
            status = Status::FAIL;
            status.setTitle(QObject::tr("File error"));
            status.setDescription(QObject::tr("Failed to read files: ") + file.errorString() + " " + fileImage.errorString());
        } else {

            if (!file.exists() && !fileImage.exists()) {
                QFile fileToCopy(oldBrushPath+BRUSH_CONTENT_EXT);
                fileToCopy.copy(oldBrushPath+BRUSH_CONTENT_EXT, brushPath+BRUSH_CONTENT_EXT);

                if (fileToCopy.error() != QFile::NoError) {
                    status = Status::FAIL;
                    status.setTitle("Error: Copy file");
                    status.setDescription(QObject::tr("Failed to copy: ") + fileToCopy.fileName() + ", the folder error was given: "
                                          + fileToCopy.errorString());

                    return status;
                }

                fileToCopy.setFileName(oldBrushPath+BRUSH_PREVIEW_EXT);
                fileToCopy.copy(oldBrushPath+BRUSH_PREVIEW_EXT, brushPath+BRUSH_PREVIEW_EXT);

                if (fileToCopy.error() != QFile::NoError) {
                    status = Status::FAIL;
                    status.setTitle(QObject::tr("Error: Copy file"));
                    status.setDescription(QObject::tr("Failed to copy: ") + fileToCopy.fileName() + QObject::tr(", the folder error was given: ")
                                          + fileToCopy.errorString());
                    return status;
                }

            } else {

                QString clonePostFix = BRUSH_COPY_POSTFIX;

                int countClones = 0;
                for (int i = 0; i < dir.entryList().count(); i++) {
                    QString x = dir.entryList()[i];

                    if (x == newName+BRUSH_CONTENT_EXT) {
                        countClones++;
                    }
                }

                QString clonedName = newName;
                clonedName = clonedName.append(clonePostFix+QString::number(countClones));

                QString clonedPath = oldGroupPath + QDir::separator() + clonedName;
                QString newFileName = clonedPath+BRUSH_CONTENT_EXT;
                QString newImageName = clonedPath+BRUSH_PREVIEW_EXT;
                file.copy(newFileName);
                fileImage.copy(newImageName);

                if (file.error() != QFile::NoError || fileImage.error() != QFile::NoError) {
                    status = Status::FAIL;
                    status.setTitle(QObject::tr("Error: Copy file(s)"));
                    status.setDescription(QObject::tr("Failed to copy: ") +
                                          file.fileName() + "\n " + fileImage.fileName() +
                                          QObject::tr(", the folder error was given: ") + file.errorString() + "\n " + fileImage.errorString());
                    return status;
                }

                newName = clonedName;
            }
        }

        return status;
    }

    static Status addBrushFileToList(const QString toolName, const QString& brushPreset, const QString& brushName)
    {
        Status st = Status::OK;
        QString brushConfigPath = getBrushesPath() + QDir::separator() + BRUSH_CONFIG;

        QFile file(brushConfigPath);
        file.open(QIODevice::ReadOnly | QIODevice::Text);

        if (file.error() != QFile::NoError) {
            st = Status::FAIL;

            st.setTitle(QObject::tr("Failed to open file"));
            st.setDescription(QObject::tr("The following error was given: \n") + file.errorString());
            return st;
        }

        QTextStream stream(&file);

        bool brushGroupFound = false;

        QStringList cleanupFilesList;
        while (!stream.atEnd()) {
            QString line = stream.readLine();

            if (line == QString(brushPreset+QDir::separator()+brushName)) {
                continue;
            }
            cleanupFilesList << line;
        }

        QString groupKey = "Group:" + toolName;
        QStringList newFilesList;
        for (QString line : cleanupFilesList) {

            // Assume we have found the beginning of a group/preset
            if (line.contains(groupKey, Qt::CaseSensitivity::CaseInsensitive) && !toolName.isEmpty())
            {
                brushGroupFound = true;
                newFilesList << line;
                continue;
            }

            if (brushGroupFound) {
                newFilesList << brushPreset + QDir::separator() + brushName;
                brushGroupFound = false;
            }
            newFilesList << line;
        }
        file.close();

        QFile editConfigFile(brushConfigPath);
        editConfigFile.open(QFile::ReadWrite);

        if (editConfigFile.error() != QFile::NoError) {
            st = Status::FAIL;

            st.setTitle(QObject::tr("Failed to open file"));
            st.setDescription(QObject::tr("The following error was given: \n") + file.errorString());
            return st;
        }

        QTextStream editStream(&editConfigFile);

        for (QString line : newFilesList) {
            editStream << line + "\n";
        }

        editConfigFile.close();

        return st;
    }

    // MAYBE: There's no need to blacklist files anymore since it's all been moved to disk
    // simply deleting the brush now and removing it from the config file should be enough.
    // might be better for performance if the user owns a lot of brushes...
    static Status blackListBrushFile(const QString& brushPreset, const QString& brushName)
    {
        Status st = Status::OK;
        QString brushConfigPath = getBrushesPath() + QDir::separator() + BRUSH_CONFIG;

        QFile file(brushConfigPath);

        file.open(QIODevice::ReadOnly | QIODevice::Text);

        if (file.error() != QFile::NoError) {
            st = Status::FAIL;

            st.setTitle(QObject::tr("Failed to open file"));
            st.setDescription(QObject::tr("The following error was given: \n") + file.errorString());
            return st;
        }

        QTextStream stream(&file);

        QStringList blacklistedFiles;
        while(!stream.atEnd()) {
            QString line = stream.readLine();

            if (line.contains(brushPreset+ QDir::separator() + brushName))
            {
                blacklistedFiles << "#" + brushPreset + QDir::separator() + brushName;
                continue;
            }
            blacklistedFiles << line;
        }
        file.close();

        QFile editConfigFile(brushConfigPath);

        editConfigFile.open(QFile::ReadWrite);
        if (editConfigFile.error() != QFile::NoError) {
            st = Status::FAIL;

            st.setTitle(QObject::tr("Failed to open file"));
            st.setDescription(QObject::tr("The following error was given: \n") + file.errorString());
            return st;
        }

        QTextStream editStream(&editConfigFile);

        for (QString line : blacklistedFiles) {
            editStream << line + "\n";
        }

        editConfigFile.close();

        return Status::OK;
    }
};

#endif // MPBRUSHUTILS_H
