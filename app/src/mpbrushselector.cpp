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
#include <QToolButton>
#include <QMessageBox>

#include <QJsonParseError>
#include <QJsonDocument>

#include <QSettings>
#include <QDebug>

#include "brushsetting.h"
#include "pencilerror.h"
#include "editor.h"
#include "toolmanager.h"
#include "combobox.h"

#include "mpbrushutils.h"
#include "mpbrushconfigurator.h"
#include "mpbrushpresetswidget.h"


MPBrushSelector::MPBrushSelector(QWidget *parent)
    : BaseDockWidget(parent)
{
    setWindowTitle(tr("Brush Selector", "Window title of mypaint brush selector"));

    QWidget* containerWidget = new QWidget(parent);
    mVLayout = new QVBoxLayout();

    mVLayout->setContentsMargins(2,2,2,2);
    QHBoxLayout* hLayout = new QHBoxLayout();

    mPresetComboBox = new ComboBox();

    QToolButton* configuratorButton = new QToolButton(this);
    QToolButton* presetsManagerButton = new QToolButton(this);

    configuratorButton->setText(tr("config"));
    configuratorButton->setToolTip(tr("Open brush configurator window"));

    hLayout->addWidget(configuratorButton);
    hLayout->addWidget(mPresetComboBox);
    hLayout->addWidget(presetsManagerButton);

    mVLayout->addLayout(hLayout);

    QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

    mVLayout->setContentsMargins(8,2,8,8);
    mVLayout->addSpacerItem(spacer);

    containerWidget->setLayout(mVLayout);
    setWidget(containerWidget);

    MPBrushParser::copyResourcesToAppData();

    mBrushesPath = MPCONF::getBrushesPath();
    // end of copy process

    // First, we parse the "brushes.conf" file to fill m_brushLib
    loadBrushes();

    connect(configuratorButton, &QToolButton::pressed, this, &MPBrushSelector::openConfigurator);
    connect(presetsManagerButton, &QToolButton::pressed, this, &MPBrushSelector::showPresetManager);
    connect(mPresetComboBox, &ComboBox::activated, this, &MPBrushSelector::changeBrushPreset);
}

void MPBrushSelector::initUI()
{

}

void MPBrushSelector::updateUI()
{

}

void MPBrushSelector::loadBrushes()
{
    QFile fileOrder(MPBrushParser::getBrushConfigPath(BrushConfigFile));

    if (fileOrder.open(QIODevice::ReadOnly))
    {
        // TODO: will probably have to create a brush importer
        mBrushPresets = MPBrushParser::parseConfig(fileOrder, mBrushesPath);

        if (mPresetComboBox->count() > 0) {
            mPresetComboBox->clear();
        }

        for (MPBrushPreset preset : mBrushPresets) {
            mPresetComboBox->addItem(preset.name);
        }

        QSettings settings(PENCIL2D,PENCIL2D);
        QString lastPreset = settings.value(SETTING_MPBRUSHPRESET).toString();

        if (lastPreset.isEmpty()) {
            currentPresetName = mBrushPresets.first().name;
            settings.setValue(SETTING_MPBRUSHPRESET, currentPresetName);
        } else {
            currentPresetName = lastPreset;
            mPresetComboBox->setCurrentItemFrom(lastPreset);
        }

        if (!mTabsLoaded) {
            addToolTabs();
            mTabsLoaded = true;
        }
        populateList();
    }
}

void MPBrushSelector::addToolTabs()
{

    for (int i = 0; i < TOOL_TYPE_COUNT; i++) {
        QListWidget* listWidget = new QListWidget(nullptr);
        listWidget->setUniformItemSizes(true);
        listWidget->setViewMode        (QListView::IconMode);
        listWidget->setResizeMode      (QListView::Adjust);
        listWidget->setMovement        (QListView::Static);
        listWidget->setFlow            (QListView::LeftToRight);
        listWidget->setSelectionMode   (QAbstractItemView::SingleSelection);
        listWidget->setIconSize        (QSize(ICON_SZ,ICON_SZ));
        connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(itemClicked(QListWidgetItem*)));

        ToolType tool = static_cast<ToolType>(i);
        QString toolName = BaseTool::TypeName(tool);

        mToolListWidgets.insert(toolName, listWidget);
    }
}

void MPBrushSelector::updateBrushList(QString brushName, QString brushPreset)
{
    currentPresetName = brushPreset;

    loadBrushes();
    selectBrush(brushName);
}

void MPBrushSelector::reloadBrushList()
{
    loadBrushes();
}

void MPBrushSelector::populateList()
{
    // Make sure currentBrushPreset is loaded here
    for (const MPBrushPreset& pre : mBrushPresets) {
        if (pre.name == currentPresetName) {
            currentBrushPreset = pre;
            break;
        }
    }
    MPBrushPreset preset = currentBrushPreset;

    QMapIterator<QString, QListWidget*> toolListIt(mToolListWidgets);
    while(toolListIt.hasNext()) {
        toolListIt.next();
        QListWidget* widget = toolListIt.value();

        // clear existing widgets before re-adding
        widget->clear();

        const MPBrushPreset subList = preset;
        if (subList.isEmpty()) continue; // this should not happen...

        int brushIndex = 0;
        for (const QString& brush : subList.brushesForTool(toolListIt.key().toLower())) {

            QIcon preview(mBrushesPath + QDir::separator() + currentPresetName + QDir::separator() + brush + BRUSH_PREVIEW_EXT);
            QListWidgetItem* p_item = new QListWidgetItem(preview, nullptr, widget, brushIndex);
            p_item->setToolTip(QString("%1").arg(brush));
            brushIndex++;
        }
    }
}

void MPBrushSelector::itemClicked(QListWidgetItem *itemWidget)
{
    QListWidget* listWidget = itemWidget->listWidget();
    if (listWidget)
    {
        // fine, let's read this one and emit the content to any receiver:
        const MPBrushPreset preset = currentBrushPreset;

        QString brushName = "";
        int brushIndex = 0;
        for (const QString& brush : preset.brushesForTool(currentToolName)) {
            if (brushIndex == itemWidget->type()) {
                brushName = brush;
                break;
            }
            brushIndex++;
        }

        auto status = MPBrushParser::readBrushFromFile(currentPresetName, brushName);
        if (status.errorcode == Status::OK)
        {
            currentBrushName = brushName;
            currentBrushData = status.data;
            emit brushSelected(currentToolType, currentPresetName, brushName, status.data); // Read the whole file and broadcast is as a char* buffer
        }
    }
}

void MPBrushSelector::loadToolBrushes(QString toolName)
{
    // CONSIDERATION: Maybe there's no need to store brushes in individual lists...
    QMap<QString, QListWidget*>::iterator widgetIt;

    for (widgetIt = mToolListWidgets.begin(); widgetIt != mToolListWidgets.end(); ++widgetIt)
    {
        const QString& toolNameKey = widgetIt.key();
        QListWidget* newWidget = widgetIt.value();

        newWidget->setHidden(true);
        if (toolNameKey.compare(toolName,Qt::CaseInsensitive) == 0) {

            // make sure the widget exists already and has been added to the layout
            if (newWidget != currentListWidget) {
                if (currentListWidget != nullptr) {
                    mVLayout->replaceWidget(currentListWidget, newWidget);
                } else {
                    mVLayout->addWidget(newWidget);
                }
            }
            qDebug() << "layout item count: " << mVLayout->count();
            qDebug() << "layout children: " << mVLayout->children().count();
            currentListWidget = newWidget;

            // We found current tool, so show the related listwidget
            if (currentListWidget->isHidden()) {
                currentListWidget->setHidden(false);
            }
        }
    }

    const MPBrushPreset subList = currentBrushPreset;

    if (!subList.isEmpty()) {
        if (toolName == "empty") {

        } else /*if (!anyBrushSelected()) */{
            if (!subList.brushesForTool(toolName).isEmpty()) {
                QString brushName (subList.brushesForTool(toolName).first());
                selectBrush(brushName);
            }
        }
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

    currentToolType = eToolMode;
    currentToolName = toolName;
    loadToolBrushes(toolName);
}

void MPBrushSelector::selectBrush(QString brushName)
{
    Q_UNUSED(brushName)
    if (!isValid()) return;
    QListWidget* listWidget = currentListWidget;
    QListWidgetItem* itemWidget = nullptr;

    const MPBrushPreset preset = currentBrushPreset;

    Q_ASSERT(listWidget != nullptr);

    int listWidgetIdx = 0;
    for (int i = 0; i < listWidget->count(); i++)
    {
        QListWidgetItem* item = listWidget->item(i);
        if (item->type() == listWidgetIdx) {
            itemWidget = listWidget->item(listWidgetIdx);
            break;
        }
        listWidgetIdx++;
    }

    // default one : we use the first tab page & the first item available:
    if (!itemWidget && listWidget && listWidget->count() > 0)
    {
        itemWidget = listWidget->item(0);
    }

    // Update GUI + load the brush (if any)
    if (itemWidget)
    {
//        mTabWidget->setCurrentWidget(listWidget);
        listWidget->setCurrentItem(itemWidget);
        itemClicked(itemWidget);
    }
}

bool MPBrushSelector::anyBrushSelected()
{

//    int currentTabIndex = mTabWidget->currentIndex();
//    QListWidget* listWidget = static_cast<QListWidget*>(mTabWidget->widget(currentTabIndex));
//    QString caption = mTabWidget->tabText(currentTabIndex);
//    const MPBrushPreset subList = currentBrushPreset;

//    for (int idx = 0; idx < subList.brushesForTool(caption).count(); idx++)
//    {
//        item = listWidget->item(idx);
//        if (item->isSelected()) {
//            return true;
//        }
//    }

    return false;
}

void MPBrushSelector::openConfigurator()
{
    if (mBrushConfiguratorWidget == nullptr) {
        mBrushConfiguratorWidget = new MPBrushConfigurator(this);
        mBrushConfiguratorWidget->setCore(mEditor);
        mBrushConfiguratorWidget->initUI();
        mBrushConfiguratorWidget->show();

        connect(this, &MPBrushSelector::brushSelected, mBrushConfiguratorWidget, &MPBrushConfigurator::updateConfig);
        connect(mBrushConfiguratorWidget, &MPBrushConfigurator::refreshBrushList, this, &MPBrushSelector::reloadBrushList);
        connect(mBrushConfiguratorWidget, &MPBrushConfigurator::updateBrushList, this, &MPBrushSelector::updateBrushList);
        connect(mBrushConfiguratorWidget, &MPBrushConfigurator::reloadBrushSettings, this, &MPBrushSelector::reloadCurrentBrush);
    } else {
        if (!mBrushConfiguratorWidget->isVisible()) {
            mBrushConfiguratorWidget->show();
        }
    }
    emit brushSelected(currentToolType, currentPresetName, currentBrushName, currentBrushData);
}

void MPBrushSelector::reloadCurrentBrush()
{
    emit brushSelected(currentToolType, currentPresetName, currentBrushName, currentBrushData);
}

void MPBrushSelector::showPresetManager()
{
    if (mPresetsWidget == nullptr) {
        mPresetsWidget = new MPBrushPresetsWidget(mBrushPresets, this);
        connect(mPresetsWidget, &MPBrushPresetsWidget::presetsChanged, this, &MPBrushSelector::reloadBrushList);
    }
    mPresetsWidget->show();
}

void MPBrushSelector::changeBrushPreset(int index, QString name, int data)
{
    Q_UNUSED(index)
    Q_UNUSED(data)

    QSettings settings(PENCIL2D,PENCIL2D);
    settings.setValue(SETTING_MPBRUSHPRESET, name);

    currentPresetName = name;
    populateList();
}

void MPBrushSelector::showNotImplementedPopup()
{
    QMessageBox::information(this, tr("Not implemented"),
                                  tr("This feature is coming soon"));
}
