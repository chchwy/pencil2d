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

#include <QSettings>

#include <QDebug>

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
        QString currentGroup;
        QStringList brushesGroup;
        while (!fileOrder.atEnd())
        {
            QString line ( fileOrder.readLine().trimmed() );
            if (line.isEmpty() || line.startsWith("#")) continue;
            if (line.startsWith("Group:"))
            {
                // first, we store the last brushesGroup (if any). Note that declaring 2 groups with the same name is wrong (only the last one will be visible)
                if (!currentGroup.isEmpty() && !brushesGroup.isEmpty()) m_brushLib.insert(currentGroup, brushesGroup);

                currentGroup = line.section(':',1).trimmed(); // Get the name after the first ':' separator
                brushesGroup.clear();
                continue;
            }

            if (QFileInfo(brushLibPath + QDir::separator() + line + BRUSH_CONTENT_EXT).isReadable()) brushesGroup << line;
        }

        if (!currentGroup.isEmpty() && !brushesGroup.isEmpty()) m_brushLib.insert(currentGroup, brushesGroup);

        populateList();
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
            content.append( (char)0 );
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

    // TODO: select last known brush type for the tab...
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
