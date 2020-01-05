/* brushlib - The MyPaint Brush Library (demonstration project)
 * Copyright (C) 2013 POINTCARRE SARL / Sebastien Leon email: sleon at pointcarre.com
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "mpbrushselector.h"

#include <QDir>
#include <QListWidget>
#include <QTabWidget>
#include <QLayout>

#include <QJsonParseError>
#include <QJsonDocument>

#include <QSettings>
#include <QDebug>

#include "brushsetting.h"

static const QString BRUSH_CONTENT_EXT = ".myb";
static const QString BRUSH_PREVIEW_EXT = "_prev.png";
static const QString BRUSH_LIST = "brushes.conf";
static const int ICON_SZ = 64;

struct MPBrushParser {

    /// Parses the mypaint brush config ".conf" format and returns a map of the brush groups
    static QMap<QString, QStringList> parseConfig(QFile& file, QString brushConfigPath)
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

            if (QFileInfo(brushConfigPath + QDir::separator() + line + BRUSH_CONTENT_EXT).isReadable()) {
                brushesGroup << line;
            }

            if (!currentGroup.isEmpty() && !brushesGroup.isEmpty()) brushes.insert(currentGroup, brushesGroup);
        }
        return brushes;
    }

//    static BrushSettingInfo getBrushSettingInfo()
//    {

//    }

    static float getBaseValue(BrushSettingType brushSetting, QString brushFile)
    {
        qDebug() << "brush path" << brushFile;
        QFile file(brushFile+BRUSH_CONTENT_EXT);
        file.open(QIODevice::ReadOnly | QIODevice::Text);

        QJsonParseError jsonError;
        QJsonDocument flowerJson = QJsonDocument::fromJson(file.readAll(),&jsonError);
        if (jsonError.error != QJsonParseError::NoError){
        qDebug() << jsonError.errorString();
        }

//        qDebug() << flowerJson;
        QMap<QString, QVariant> list = flowerJson.toVariant().toMap();
//        qDebug() << list["settings"].toMap()[""];

        QMap<QString, QVariant> brushSettingMap = list["settings"].toMap();
        QMap<QString, QVariant> brushBaseValue = brushSettingMap[getName(brushSetting)].toMap();

//        qDebug() << brushBaseValue;
        qDebug() << brushBaseValue["base_value"].toFloat();
//        if (getName(brushSetting))
//        {

//        }
//        QMap<QString, QVariant> map = list.first().toMap();
//        qDebug() << map["baseValue"].toString();
//        return list["settings"].toFloat();
        return 0;
    }

    static QString getName(BrushSettingType& type)
    {
        switch(type)
        {
        case BrushSettingType::BRUSH_SETTING_OPAQUE:                      return "opaque";
        case BrushSettingType::BRUSH_SETTING_OPAQUE_MULTIPLY:             return "opaque_multiply";
        case BrushSettingType::BRUSH_SETTING_OPAQUE_LINEARIZE:            return "opaque_linearize";
        case BrushSettingType::BRUSH_SETTING_RADIUS_LOGARITHMIC:          return "radius_logarithmic";
        case BrushSettingType::BRUSH_SETTING_HARDNESS:                    return "hardness";
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
};


MPBrushSelector::MPBrushSelector(const QString &brushLibPath, QWidget *parent)
    : BaseDockWidget(parent),
      m_brushesPath(brushLibPath)
{
    setWindowTitle(tr("Brush Selector", "Window title of mypaint brush selector"));

    mTabWidget = new QTabWidget(parent);

    setWidget(mTabWidget);
    mTabWidget->tabBar()->hide();

    // First, we parse the "order.conf" file to fill m_brushLib
    QFile fileOrder(brushLibPath + QDir::separator() + BRUSH_LIST);

    if (fileOrder.open(QIODevice::ReadOnly))
    {
        m_brushLib = MPBrushParser::parseConfig(fileOrder, brushLibPath);
        populateList();
    }

    for (QStringList brushList : m_brushLib) {

        for (QString brush : brushList) {

            for (BrushSettingType setting : allSettings) {
                qDebug() << MPBrushParser::getBaseValue(setting, brushLibPath+QDir::separator()+brush);
            }
        }
    }
}

void MPBrushSelector::initUI()
{

}

void MPBrushSelector::updateUI()
{

}

void MPBrushSelector::populateList()
{
    foreach (const QString &caption, m_brushLib.keys())
    {
        const QStringList subList = m_brushLib.value(caption);
        if (subList.isEmpty()) continue; // this should not happen...
        QListWidget* listWidget = new QListWidget();
        listWidget->setUniformItemSizes(true);
        listWidget->setViewMode        (QListView::IconMode);
        listWidget->setResizeMode      (QListView::Adjust);
        listWidget->setMovement        (QListView::Static);
        listWidget->setFlow            (QListView::LeftToRight);
        listWidget->setSelectionMode   (QAbstractItemView::SingleSelection);
        listWidget->setIconSize        (QSize(ICON_SZ,ICON_SZ));
        connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(itemClicked(QListWidgetItem*)));

        mTabWidget->addTab(listWidget, caption);
        for (int n = 0 ; n < subList.count() ; n++)
        {
            QString name (subList.at(n));
            QIcon preview(m_brushesPath + QDir::separator() + name + BRUSH_PREVIEW_EXT);
            QListWidgetItem* p_item = new QListWidgetItem(preview, QString(), listWidget, n); // no need to show the name as it is already visible in preview
            p_item->setToolTip(QString("%1 in \"%2\".").arg(name).arg(caption));
        }
    }
    QListWidget* emptyWidget = new QListWidget();

    mTabWidget->addTab(emptyWidget, "");
}

void MPBrushSelector::itemClicked(QListWidgetItem *itemWidget)
{
    QListWidget* listWidget = itemWidget->listWidget();
    if (listWidget)
    {
        QString toolName;
        // first of all, we will deselect all other items in other panels :
        for (int p = 0 ; p < mTabWidget->count() ; p++)
        {
            QListWidget* otherListWidget = dynamic_cast<QListWidget*>(mTabWidget->widget(p));
            if (otherListWidget != listWidget)
            {
                otherListWidget->clearSelection();
            }
            else
            {
                toolName = mTabWidget->tabText(p);
            }

        }
        // fine, let's read this one and emit the content to any receiver:
        const QStringList subList = m_brushLib.value(toolName);
        QString brushName (subList.at(itemWidget->type()));

        QFile f( m_brushesPath + QDir::separator() + subList.at(itemWidget->type()) + BRUSH_CONTENT_EXT );
        if (f.open( QIODevice::ReadOnly ))
        {
            QByteArray content = f.readAll();
            content.append( static_cast<char>(0) );
            emit brushSelected(toolName, brushName, content); // Read the whole file and broadcast is as a char* buffer
        }
    }
}

void MPBrushSelector::loadToolBrushes(QString toolName)
{
    // first of all, we will deselect all other items in other panels :
    bool foundSelection = false;

    for (int i = 0 ; i < mTabWidget->count() ; i++)
    {
        QString caption = mTabWidget->tabText(i);

        if (caption == toolName) {
            mTabWidget->setCurrentIndex(i);
            foundSelection = true;
        }
    }
    // If there is no tab matching the current tool,
    // we select the last one (empty)
    if (!foundSelection) {
        mTabWidget->setCurrentIndex(mTabWidget->count() - 1);
    }

    const QStringList subList = m_brushLib.value(toolName);

    if (!subList.isEmpty()) {

        QString lastUsed;
        if (toolName != "empty") {
            QSettings settings(PENCIL2D, PENCIL2D);
            lastUsed = settings.value("LastBrushFor_"+toolName).toString();
            selectBrush(lastUsed);
        }

        if (!anyBrushSelected() && lastUsed.isEmpty()) {
            QString brushName (subList.at(0));
            selectBrush(brushName);
        } else {}
    }
}

void MPBrushSelector::typeChanged(ToolType eToolMode)
{
    QString toolName = "";
    switch ( eToolMode )
    {
    case ToolType::PENCIL:
        toolName = "pencil";
        break;
    case ToolType::ERASER:
        toolName = "eraser";
        break;
    case ToolType::PEN:
        toolName = "pen";
        break;
    case ToolType::BRUSH:
    case ToolType::POLYLINE:
        toolName = "brush";
        break;
    case ToolType::SMUDGE:
        toolName = "smudge";
        break;
    default:
        toolName = "empty";
        break;
    }

    loadToolBrushes(toolName);
}

void MPBrushSelector::selectBrush (QString brushName)
{
    if (!isValid()) return;
    QListWidget* listWidget = nullptr;
    QListWidgetItem* itemWidget = nullptr;

    for (int page = mTabWidget->count()-1 ; page >= 0 && !itemWidget ; page--)
    {
        // reverse loop so we leave it with first page
        listWidget = dynamic_cast<QListWidget*>(mTabWidget->widget(page));
        QString caption = mTabWidget->tabText(page);
        const QStringList subList = m_brushLib.value(caption);
        if (brushName.isEmpty()) { break; }

        for (int idx = 0; idx < subList.count(); idx++)
        {
            if (subList.at(idx) == brushName) {
                itemWidget = listWidget->item(idx);
                break;
            }
        }
    }
    // default one : we use the first tab page & the first item available:
    if (!itemWidget && listWidget && listWidget->count())
    {
        itemWidget = listWidget->item(0);
    }

    // Update GUI + load the brush (if any)
    if (itemWidget)
    {
        mTabWidget->setCurrentWidget(listWidget);
        listWidget->setCurrentItem (itemWidget);
        itemClicked(itemWidget);
    }
}

bool MPBrushSelector::anyBrushSelected()
{
    QListWidgetItem* item = nullptr;

    int currentTabIndex = mTabWidget->currentIndex();
    QListWidget* listWidget = static_cast<QListWidget*>(mTabWidget->widget(currentTabIndex));
    QString caption = mTabWidget->tabText(currentTabIndex);
    const QStringList subList = m_brushLib.value(caption);

    for (int idx = 0; idx < subList.count(); idx++)
    {
        item = listWidget->item(idx);
        if (item->isSelected()) {
            return true;
        }
    }

    return false;
}
