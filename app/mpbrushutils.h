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
static const QString DefaultPreset = "deevad";

static const QString CommentToken= "#";
static const QString PresetToken = "Preset:";
static const QString BrushToken = "-";
static const QString ToolToken = "Tool:";
struct MPCONF {

    static Status renamePreset(const QString& oldName, const QString& newName)
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

        QStringList newFilesList;
        bool presetRenamed = false;

        while (!stream.atEnd()) {
            QString line = stream.readLine();

            if (!presetRenamed) {
                if (MPCONF::getValue(line).compare(oldName, Qt::CaseInsensitive) == 0) {
                    line = PresetToken + " " + newName;
                    presetRenamed = true;
                }
            }

            newFilesList << line;
        }
        file.close();

        if (newFilesList.isEmpty()) { return st; }

        QFile editConfigFile(brushConfigPath);
        editConfigFile.resize(0);
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

    static Status addPreset(const QString& presetName)
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

        QStringList readLineList;
        while (!stream.atEnd()) {
            readLineList << stream.readLine();
        }

        QStringList newFilesList;
        bool presetTokenFound = false;
        bool presetAdded = false;
        for (const QString& line : readLineList) {

            // Find preset first
            if (!presetAdded) {
                if (isPresetToken(line)) {

                    presetTokenFound = true;
                }

                if (presetTokenFound) {
                    newFilesList.append(PresetToken + " " + presetName);
                    newFilesList.append("\tTool: pencil");
                    newFilesList.append("\tTool: eraser");
                    newFilesList.append("\tTool: pen");
                    newFilesList.append("\tTool: smudge");
                    newFilesList.append("\tTool: brush");
                    presetAdded = true;
                }
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

    static Status removePreset(const QString presetName)
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
        bool searchingPreset = false;
        QStringList newFilesList;
        while (!stream.atEnd()) {

            const QString& line = stream.readLine();
                if (isPresetToken(line)) {

                    // find preset of interest otherwise skip
                    if (MPCONF::getValue(line).compare(presetName, Qt::CaseInsensitive) == 0) {
                        searchingPreset = true;
                    } else {
                        searchingPreset = false;
                    }
                }

            if (searchingPreset) {
                continue;
            }
            newFilesList << line;
        }
        file.close();

        if (newFilesList.isEmpty()) { return Status::FAIL; }

        QFile editConfigFile(brushConfigPath);
        editConfigFile.resize(0);
        editConfigFile.open(QFile::ReadWrite);

        if (editConfigFile.error() != QFile::NoError) {
            st = Status::FAIL;

            st.setTitle(QObject::tr("Failed to open file"));
            st.setDescription(QObject::tr("The following error was given: \n") + file.errorString());
            return st;
        }

        QTextStream editStream(&editConfigFile);
        editStream.reset();

        for (QString line : newFilesList) {
            editStream << line + "\n";
            qDebug() << line;
        }

        editConfigFile.close();

        return st;
    }

    static Status addBrush(const QString toolName, const QString& brushPreset, const QString& brushName)
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

        bool searchingPreset = false;
        bool searchingTool = false;
        bool brushAdded = false;
        QStringList newFilesList;
        while (!stream.atEnd()) {

            QString line = stream.readLine();
            // Find preset first
            if (!brushAdded) {

                qDebug() << MPCONF::getValue(line);
                if (isPresetToken(line)) {

                    // find preset of interest otherwise skip
                    if (MPCONF::getValue(line).compare(brushPreset, Qt::CaseInsensitive) == 0) {
                        searchingPreset = true;
                    } else {
                        searchingPreset = false;
                    }
                }

                if (isToolToken(line)) {
                    if (MPCONF::getValue(line).compare(toolName, Qt::CaseInsensitive) == 0) {
                        searchingTool = true;
                    } else {
                        searchingTool = false;
                    }
                }
            }

            if ((searchingTool && searchingPreset) /*|| isBrushToken(line)*/) {
                if (!brushAdded) {

                    if (isToolToken(line)) {
                        newFilesList << line;
                        newFilesList << "\t\t- " + brushName;
                    } else {
                        newFilesList << "\t\t- " + brushName;
                    }
                    brushAdded = true;
                    continue;
                }
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

    static Status removeBrush(const QString& brushPreset, const QString& toolName, const QString& brushName)
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

        QStringList newList;

        bool searchingPreset = false;
        bool searchingTool = false;
        while(!stream.atEnd()) {
            QString line = stream.readLine();

            if (isPresetToken(line)) {

                if (MPCONF::getValue(line).compare(brushPreset, Qt::CaseInsensitive) == 0) {
                    searchingPreset = true;
                } else {
                    searchingPreset = false;
                }
            }

            if (isToolToken(line)) {

                if (MPCONF::getValue(line).compare(toolName, Qt::CaseInsensitive) == 0) {
                    searchingTool = true;
                } else {
                    searchingTool = false;
                }
            }

            if ((searchingPreset && searchingTool)) {
                if (MPCONF::getValue(line).compare(brushName, Qt::CaseInsensitive) == 0)
                {
                    continue;
                }
            }
            newList << line;
        }
        file.close();

        QFile editConfigFile(brushConfigPath);

        editConfigFile.resize(0);
        editConfigFile.open(QFile::ReadWrite);
        if (editConfigFile.error() != QFile::NoError) {
            st = Status::FAIL;

            st.setTitle(QObject::tr("Failed to open file"));
            st.setDescription(QObject::tr("The following error was given: \n") + file.errorString());
            return st;
        }

        QTextStream editStream(&editConfigFile);

        for (QString line : newList) {
            editStream << line + "\n";
        }

        editConfigFile.close();

        return Status::OK;
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

        QStringList newList;

        bool searchingPreset = false;
        while(!stream.atEnd()) {
            QString line = stream.readLine();

            if (isPresetToken(line)) {

                if (MPCONF::getValue(line).compare(brushPreset, Qt::CaseInsensitive) == 0) {
                    searchingPreset = true;
                } else {
                    searchingPreset = false;
                }
            }

            if (searchingPreset) {
                if (MPCONF::getValue(line).compare(brushName, Qt::CaseInsensitive) == 0)
                {
                    line = line.prepend("#");
                }
            }
            newList << line;
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

        for (QString line : newList) {
            editStream << line + "\n";
        }

        editConfigFile.close();

        return Status::OK;
    }

    static QString getBrushesPath()
    {
        QStringList pathList = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
        return pathList.first() + QDir::separator() + "brushes";
    }

    static QString getValue(const QString& text) {
        if (isCommentToken(text)) {
            return text.section('#',1).trimmed();
        } else if (isPresetToken(text) || isToolToken(text)) {
            return text.section(':',1).trimmed();
        } else if (isBrushToken(text)) {
            return text.section('-',1).trimmed();
        }
        return "";
    }

    static bool isCommentToken(const QString& text) {
        if (text.isEmpty() || text.startsWith(CommentToken)) { return true; }
        return false;
    }

    static bool isPresetToken(const QString& text) {
        if (text.trimmed().startsWith(PresetToken, Qt::CaseInsensitive)) { return true; }
        return false;
    }

    static bool isToolToken(const QString& text) {
        if (text.trimmed().startsWith(ToolToken, Qt::CaseInsensitive)) { return true; }
        return false;
    }

    static bool isBrushToken(const QString& text) {
        if (text.trimmed().startsWith(BrushToken, Qt::CaseInsensitive)) { return true; }
        return false;
    }
};

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

struct MPBrushPreset {
    QString name;

    void clear() {
        brushForToolMap.clear();
    }

    QList<QStringList> allBrushes() const {
        return brushForToolMap.values();
    }

    QStringList brushesForTool(QString toolKey) const {
        return brushForToolMap.value(toolKey);
    }

    void insert(QString toolName, QStringList brushNames)
    {
        brushForToolMap.insert(toolName, brushNames);
    }

    bool isEmpty() const {
        return brushForToolMap.isEmpty();
    }

    QList<QString> toolNames() {
        return brushForToolMap.keys();
    }

private:
    QMap<QString, QStringList> brushForToolMap;

};

struct MPBrushParser {

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

    /// Parses the mypaint brush config ".conf" format and returns a map of the brush groups
    static QVector<MPBrushPreset> parseConfig(QFile& file, QString brushesPath)
    {

        MPBrushPreset brushesForPreset;
        QString currentTool;
        QString currentPreset;
        QStringList brushList;

        QVector<MPBrushPreset> brushPresets;

        int presetIndex = 0;
        while (!file.atEnd())
        {
            QString line ( file.readLine().trimmed() );
            if (line.isEmpty() || line.startsWith("#")) continue;

            if (MPCONF::isPresetToken(line))
            {
                if (!brushesForPreset.isEmpty() && !currentTool.isEmpty()) {

                    brushesForPreset.insert(currentTool, brushList);

                    brushPresets[presetIndex] = brushesForPreset;
                    presetIndex++;
                }
                currentPreset = MPCONF::getValue(line);
                brushesForPreset.name = currentPreset;

                brushesForPreset.clear();
                brushList.clear();

                brushPresets.append(brushesForPreset);
                continue;
            }

            if (MPCONF::isToolToken(line))
            {

                if (!currentTool.isEmpty()) {
                    brushesForPreset.insert(currentTool, brushList);
                }

                currentTool = MPCONF::getValue(line);
                brushList.clear();
                continue;
            }

            if (MPCONF::isBrushToken(line)) {

                QString brush = MPCONF::getValue(line);
                QString relativePath = currentPreset + QDir::separator() + brush;
                if (QFileInfo(brushesPath + QDir::separator() + relativePath + BRUSH_CONTENT_EXT).isReadable()) {
                    brushList << brush;
                }
                continue;
            }

            if (!currentTool.isEmpty() && !brushPresets.isEmpty()) {
                brushPresets.append(brushesForPreset);
            }

            if (brushPresets.isEmpty()) {
                // TODO: Handle error case
            }
        }

        if (!brushesForPreset.isEmpty() && !currentTool.isEmpty()) {

            brushesForPreset.insert(currentTool, brushList);

            Q_ASSUME(presetIndex <= brushPresets.size());

            brushPresets[presetIndex] = brushesForPreset;
            presetIndex++;
        }

        return brushPresets;
    }

    /// Copy internal brush resources to app data folder
    /// This is where brushes will be loaded from in the future.
    static Status copyResourcesToAppData()
    {
        QString appDataBrushesPath = MPCONF::getBrushesPath();
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

    static QString getBrushPreviewImagePath(const QString& brushPreset, const QString brushName)
    {
        const QString& brushPath = MPCONF::getBrushesPath() + QDir::separator() + brushPreset + QDir::separator() + brushName;
        QFile file(brushPath+BRUSH_PREVIEW_EXT);

        if (file.exists()) {
            return file.fileName();
        }
        return "";
    }

    static Status::StatusData readBrushFromFile(const QString& brushPreset, const QString& brushName)
    {
        const QString& brushPath = MPCONF::getBrushesPath() + QDir::separator() + brushPreset + QDir::separator() + brushName;

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

    static QString getBrushPath(const QString& brushPreset, const QString& brushName, const QString& extension)
    {
        QString brushPath = MPCONF::getBrushesPath() + QDir::separator() + brushPreset + QDir::separator() + brushName;

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
        QString brushPath = MPCONF::getBrushesPath() + QDir::separator() + extension;

        QFile file(brushPath);
        if (!file.exists()) {
            brushPath = QString(BRUSH_QRC) + QDir::separator() + extension;
        }
        return brushPath;
    }

    static Status writeBrushToFile(const QString& brushPreset, const QString& brushName, const QByteArray& data)
    {
        Status status = Status::OK;
        QString brushesPath = MPCONF::getBrushesPath();
        const QString& groupPath = brushesPath + QDir::separator() + brushPreset + QDir::separator();
        const QString& brushPath = brushesPath + QDir::separator() + brushPreset + QDir::separator() + brushName;

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
        QString brushesPath = MPCONF::getBrushesPath();
        const QString groupPath = brushesPath + QDir::separator() + newPreset;
        const QString brushPath = brushesPath + QDir::separator() + newPreset + QDir::separator() + newName;

        const QString oldGroupPath = brushesPath + QDir::separator() + originalPreset;
        const QString oldBrushPath = brushesPath + QDir::separator() + originalPreset + QDir::separator() + originalName;

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

        const QString brushPath = MPCONF::getBrushesPath() + QDir::separator() + brushPreset + QDir::separator() + brushName;

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
        QString brushesPath = MPCONF::getBrushesPath();
        QString groupPath = brushesPath + QDir::separator() + newPreset;
        QString brushPath = brushesPath + QDir::separator() + newPreset + QDir::separator() + newName;

        QString oldGroupPath = brushesPath + QDir::separator() + originalPreset;
        QString oldBrushPath = brushesPath + QDir::separator() + originalPreset + QDir::separator() + originalName;

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
                                          QObject::tr(", the folder error was given: ") + file.errorString() + "\n" + fileImage.errorString());
                    return status;
                }

                newName = clonedName;
            }
        }

        return status;
    }
};

#endif // MPBRUSHUTILS_H
