/* brushlib - The MyPaint Brush Library (demonstration project)
 * Copyright (C) 2013 POINTCARRE SARL / Sebastien Leon email: sleon at pointcarre.com
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#ifndef MPBRUSHSELECTOR_H
#define MPBRUSHSELECTOR_H

//#include <QTabWidget>
#include <QMap>
#include <QString>

#include "basedockwidget.h"
#include "pencildef.h"

class QListWidgetItem;
class QTabWidget;
class QListWidget;
class MPBrushConfigurator;
class QComboBox;

// MPBrushSelector is a TabWidget showing the various brushes (display the small screenshots)
// it allows the user to select it and emit a signal.
// NOTE : The order is not properly kept as I did not realize the file order.conf
//        was containing this information. Will be fixed soon.
//
class MPBrushSelector : public BaseDockWidget
{

  Q_OBJECT
public:
  MPBrushSelector(QWidget* parent = 0 );

  bool isValid() { return !m_brushLib.isEmpty(); }
  void loadToolBrushes(QString toolName);

  void initUI() override;
  void updateUI() override;

  void setCore(Editor* editor) { mEditor = editor; }

public slots:
  void reloadCurrentBrush();
  void selectBrush(QString brushPreset, QString brushName);
  void typeChanged(ToolType);
  void reloadBrushList();
  void updateBrushList(QString brushName, QString brushPreset);

signals:
  void brushSelected (ToolType toolType, const QString& brushGroup, const QString& brushName, const QByteArray& content);

protected:
  QMap<QString, QStringList> m_brushLib;
  QString              m_brushesPath;

protected slots:
  void itemClicked ( QListWidgetItem *);

private:

  Editor* mEditor = nullptr;
  bool mTabsLoaded = false;

  void populateList();
  bool anyBrushSelected();
  void loadBrushes();
  void addToolTabs();

  void openConfigurator();
  void showNotImplementedPopup();

  QTabWidget* mTabWidget;
  QMap<QString, QListWidget*> mToolListWidgets;
  QComboBox* mPresetComboBox;

  MPBrushConfigurator* mBrushConfiguratorWidget = nullptr;

  QString currentBrushPreset;
  QString currentBrushName;
  QString currentToolName;
  ToolType currentToolType;
  QByteArray currentBrushData;
};



#endif // MPBRUSHSELECTOR_H
