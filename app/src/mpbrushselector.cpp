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
#include <QComboBox>
#include <QMessageBox>

#include <QJsonParseError>
#include <QJsonDocument>

#include <QSettings>
#include <QDebug>

#include "brushsetting.h"
#include "pencilerror.h"

#include "mpbrushutils.h"
#include "mpbrushconfigurator.h"


MPBrushSelector::MPBrushSelector(QWidget *parent)
    : BaseDockWidget(parent)
{
    setWindowTitle(tr("Brush Selector", "Window title of mypaint brush selector"));

    QWidget* containerWidget = new QWidget(this);
    QVBoxLayout* vLayout = new QVBoxLayout(nullptr);
    QHBoxLayout* hLayout = new QHBoxLayout(this);

    mPresetComboBox = new QComboBox(this);


    // TODO: show presets based on brush folders...
    mPresetComboBox->addItem("Deevad");

    QToolButton* configuratorButton = new QToolButton(this);
    QToolButton* addPresetButton = new QToolButton(this);

    mTabWidget = new QTabWidget(this);

    configuratorButton->setText(tr("config"));
    configuratorButton->setToolTip(tr("Open brush configurator window"));

    hLayout->addWidget(configuratorButton);
    hLayout->addWidget(mPresetComboBox);
    hLayout->addWidget(addPresetButton);

    vLayout->addLayout(hLayout);
    vLayout->addWidget(mTabWidget);

    vLayout->setMargin(8);

    containerWidget->setLayout(vLayout);
    setWidget(containerWidget);
    mTabWidget->tabBar()->hide();

    MPBrushParser::copyResourcesToAppData();

    m_brushesPath = MPBrushParser::getBrushesPath();
    // end of copy process

    // First, we parse the "brushes.conf" file to fill m_brushLib
    loadBrushes();

    connect(configuratorButton, &QToolButton::pressed, this, &MPBrushSelector::openConfigurator);
    connect(addPresetButton, &QToolButton::pressed, this, &MPBrushSelector::showNotImplementedPopup);
}

void MPBrushSelector::initUI()
{

}

void MPBrushSelector::updateUI()
{

}

void MPBrushSelector::loadBrushes()
{
    // TODO: removing brushes is almost there
    // we just need to change brush path from using the internal to an external
    // when should this happen though? and should we save it in settings?
    QFile fileOrder(MPBrushParser::getBrushConfigPath(BRUSH_CONFIG));

    if (fileOrder.open(QIODevice::ReadOnly))
    {
        // TODO: will probably have to create a brush importer, group has to match brush type
        // otherwise there could be undefined behaviour...
        m_brushLib = MPBrushParser::parseConfig(fileOrder, m_brushesPath);

        if (!mTabsLoaded) {

            // add tabs corresponding to the drawing tools pencil use
            // FIXME: this is a HACK but I'm too lazy to change it.
            addToolTabs();
            mTabsLoaded = true;
        }
        populateList();
    }
}

void MPBrushSelector::addToolTabs()
{
    for (const QString &brushGroup : m_brushLib.keys())
    {
        QListWidget* listWidget = new QListWidget(mTabWidget);
        listWidget->setUniformItemSizes(true);
        listWidget->setViewMode        (QListView::IconMode);
        listWidget->setResizeMode      (QListView::Adjust);
        listWidget->setMovement        (QListView::Static);
        listWidget->setFlow            (QListView::LeftToRight);
        listWidget->setSelectionMode   (QAbstractItemView::SingleSelection);
        listWidget->setIconSize        (QSize(ICON_SZ,ICON_SZ));
        connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(itemClicked(QListWidgetItem*)));

        mTabWidget->addTab(listWidget, brushGroup);
        mToolListWidgets.insert(brushGroup, listWidget);
    }
    QListWidget* emptyWidget = new QListWidget();

    mTabWidget->addTab(emptyWidget, "");
}

void MPBrushSelector::brushListChanged()
{
    loadBrushes();
    updateSelectedBrushForTool(currentToolName);
}

void MPBrushSelector::populateList()
{
    QMapIterator<QString, QListWidget*> toolListIt(mToolListWidgets);
//    toolListIt.hasNext();
    while(toolListIt.hasNext()) {
        toolListIt.next();
        QListWidget* widget = toolListIt.value();

        // clear existing widgets before re-adding
        widget->clear();

        Q_ASSERT(mToolListWidgets.count() == m_brushLib.keys().count());

        // Has to be a tool, can't be any arbitrary name
        QString brushGroup = m_brushLib.find(toolListIt.key()).key();

        const QStringList subList = m_brushLib.value(brushGroup);
        if (subList.isEmpty()) continue; // this should not happen...

        for (int n = 0 ; n < subList.count() ; n++) {
            QString brushGroupAndName (subList.at(n));
            QIcon preview(m_brushesPath + QDir::separator() + brushGroupAndName + BRUSH_PREVIEW_EXT);
            QListWidgetItem* p_item = new QListWidgetItem(preview, QString(), widget, n);
            p_item->setToolTip(QString("%1 in '%2'.").arg(brushGroupAndName).arg(brushGroup));
        }
    }
}

void MPBrushSelector::itemClicked(QListWidgetItem *itemWidget)
{
    QListWidget* listWidget = itemWidget->listWidget();
    if (listWidget)
    {
        QString toolName = "";
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
        QString brushGroupAndName (subList.at(itemWidget->type()));

        QRegularExpression re("(\\w+?[\\/])(\\w+)");
        QRegularExpressionMatch match = re.match(brushGroupAndName);
        QString brushGroup = "";
        QString brushName = "";
        if (match.hasMatch()) {
            brushGroup = match.captured(1);
            brushName = match.captured(2);
//            qDebug() << brushGroup;
//            qDebug() << brushName;
        }

        auto status = MPBrushParser::readBrushFromFile(brushGroup, brushName);
        if (status.errorcode == Status::OK)
        {
            currentBrushName = brushName;
            currentBrushGroup = brushGroup;
            currentToolName = toolName;
            currentBrushData = status.data;
            emit brushSelected(toolName, brushGroup, brushName, status.data); // Read the whole file and broadcast is as a char* buffer
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
            break;
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
            updateSelectedBrushForTool(toolName);
        } else {
            if (!anyBrushSelected() && lastUsed.isEmpty()) {
                QString brushName (subList.at(0));
                selectBrush(brushName);
            }
        }
    }
}

void MPBrushSelector::updateSelectedBrushForTool(QString toolName)
{
    QSettings settings(PENCIL2D, PENCIL2D);

    if (toolName.isEmpty()) {
        toolName = currentToolName;
    }

    QString lastUsed = settings.value("LastBrushFor_"+toolName).toString();
    QMapIterator<QString, QStringList> brushLibIt(m_brushLib);
    while (brushLibIt.hasNext()) {
        brushLibIt.next();

        if (brushLibIt.key() == toolName) {
            if (!brushLibIt.value().contains(lastUsed)) {
                lastUsed = brushLibIt.value().first();
                settings.setValue("LastBrushFor_"+toolName, lastUsed);
            }
        }
    }
    selectBrush(lastUsed);
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

void MPBrushSelector::selectBrush(QString brushName)
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

void MPBrushSelector::openConfigurator()
{
    if (mBrushConfiguratorWidget == nullptr) {
        mBrushConfiguratorWidget = new MPBrushConfigurator(this);
        mBrushConfiguratorWidget->setCore(mEditor);
        mBrushConfiguratorWidget->initUI();
        mBrushConfiguratorWidget->show();
        mBrushConfiguratorWidget->raise();

        connect(this, &MPBrushSelector::brushSelected, mBrushConfiguratorWidget, &MPBrushConfigurator::updateConfig);
        connect(mBrushConfiguratorWidget, &MPBrushConfigurator::updateBrushList, this, &MPBrushSelector::brushListChanged);
    } else {
        if (!mBrushConfiguratorWidget->isVisible()) {
            mBrushConfiguratorWidget->show();
            mBrushConfiguratorWidget->raise();
        }
    }
    emit brushSelected(currentToolName, currentBrushGroup, currentBrushName, currentBrushData);
}

void MPBrushSelector::showNotImplementedPopup()
{
    QMessageBox::information(this, tr("Not implemented"),
                                  tr("This feature is coming soon"));
}
