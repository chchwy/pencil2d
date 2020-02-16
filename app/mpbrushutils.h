#ifndef MPBRUSHUTILS_H
#define MPBRUSHUTILS_H

#include <QString>
#include <QMap>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QDirIterator>

#include <QDebug>

#include "pencilerror.h"
#include "brushsetting.h"

static const QString BRUSH_CONTENT_EXT = ".myb";
static const QString BRUSH_PREVIEW_EXT = "_prev.png";
static const QString BRUSH_CONFIG = "brushes.conf";
static const QString BRUSH_QRC = ":brushes";
static const int ICON_SZ = 64;

struct MPBrushParser {

    /// Parses the mypaint brush config ".conf" format and returns a map of the brush groups
    static QMap<QString, QStringList> parseConfig(QFile& file, QString brushesPath)
    {
        QString currentGroup;
        QStringList brushesGroup;
        QMap<QString, QStringList> brushes;

//        copyResourcesToAppData();
//        // end of copy process

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

//    static float getBaseValue(BrushSettingType brushSetting, QString brushFile)
//    {
//        qDebug() << "brush path" << brushFile;
//        QFile file(brushFile+BRUSH_CONTENT_EXT);
//        file.open(QIODevice::ReadOnly | QIODevice::Text);

//        QJsonParseError jsonError;
//        QJsonDocument flowerJson = QJsonDocument::fromJson(file.readAll(),&jsonError);
//        if (jsonError.error != QJsonParseError::NoError){
//        qDebug() << jsonError.errorString();
//        }

////        qDebug() << flowerJson;
//        QMap<QString, QVariant> list = flowerJson.toVariant().toMap();
////        qDebug() << list["settings"].toMap()[""];

//        QMap<QString, QVariant> brushSettingMap = list["settings"].toMap();
//        QMap<QString, QVariant> brushBaseValue = brushSettingMap[getName(brushSetting)].toMap();

////        qDebug() << brushBaseValue;
//        qDebug() << brushBaseValue["base_value"].toFloat();
////        if (getName(brushSetting))
////        {

////        }
////        QMap<QString, QVariant> map = list.first().toMap();
////        qDebug() << map["baseValue"].toString();
////        return list["settings"].toFloat();
//        return 0;
//    }

    /// Will return a string matching the names of a .myb setting
    /// eg.
    /// "change_color_h": {
    ///  ...
    /// }
    ///

    /// Copy internal brush resources to app data folder
    /// This is where brushes will be loaded from in the future.
    static Status copyResourcesToAppData()
    {
        QString appDataBrushesPath = getBrushesPath();
        QDir dir(appDataBrushesPath);
        if (!dir.exists()) {
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

    static Status::StatusData readBrushFromFile(const QString& brushGroup, const QString& brushName)
    {
        const QString& brushPath = getBrushesPath() + QDir::separator() + brushGroup+brushName;

        QFile file(brushPath + BRUSH_CONTENT_EXT);

        // if no local brush file exists, look at internal resources
        // local brush files will overwrite the existence of the internal one
        if (!file.exists()) {
            file.setFileName(BRUSH_QRC + QDir::separator() + brushGroup + brushName + BRUSH_CONTENT_EXT);
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

    static QString getBrushPath(const QString& brushGroup, const QString& brushName, const QString& extension)
    {
        QString brushPath = getBrushesPath() + QDir::separator() + brushGroup+brushName;

        QFile file(brushPath+extension);
        if (!file.exists()) {
            brushPath = QString(BRUSH_QRC) + QDir::separator() + brushGroup+brushName;
        }
        return brushPath + extension;
    }

    static QString getBrushImagePath(const QString& brushGroup, const QString& brushName)
    {
        return getBrushPath(brushGroup, brushName, BRUSH_PREVIEW_EXT);
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

    static void writeBrushToFile(const QString& brushGroup, const QString& brushName, const QByteArray& data)
    {
        const QString& groupPath = getBrushesPath() + QDir::separator() + brushGroup;
        const QString& brushPath = getBrushesPath() + QDir::separator() + brushGroup+brushName;

        QFile file(brushPath + BRUSH_CONTENT_EXT);

        if (!file.exists()) {
            QDir dir;
            dir.mkpath(groupPath);
        }

        file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
        file.write(data);
        file.close();
    }

    static void blackListBrushFile(const QString& brushGroup, const QString& brushName)
    {
        QString brushConfigPath = getBrushesPath() + QDir::separator() + BRUSH_CONFIG;

        QFile file(brushConfigPath);
        file.open(QIODevice::ReadOnly | QIODevice::Text);

        QTextStream stream(&file);

        QStringList blacklistedFiles;
        while(!stream.atEnd()) {
            QString line = stream.readLine();

            if (line.contains(brushGroup+brushName))
            {
                blacklistedFiles << "#" + brushGroup+brushName;
                continue;
            }
            blacklistedFiles << line;
        }
        file.close();

        QFile editConfig(brushConfigPath);

        // TODO: better error handling
        qDebug() << "open: " << editConfig.open(QFile::ReadWrite);

        qDebug() << editConfig.error();
        QTextStream editStream(&editConfig);

        for (QString line : blacklistedFiles) {
            editStream << line + "\n";
        }

        editConfig.close();
    }
};

#endif // MPBRUSHUTILS_H
