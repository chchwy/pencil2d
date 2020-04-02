/* brushlib - The MyPaint Brush Library (demonstration project)
 * Copyright (C) 2013 POINTCARRE SARL / Sebastien Leon email: sleon at pointcarre.com
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#ifndef MPBRUSHSELECTOR_H
#define MPBRUSHSELECTOR_H

#include <QMap>
#include <QString>
#include <QPointer>

#include "mpbrushutils.h"
#include "basedockwidget.h"
#include "pencildef.h"

class QListWidgetItem;
class QTabWidget;
class QListWidget;
class MPBrushConfigurator;
class ComboBox;
class QVBoxLayout;
class MPBrushPresetsWidget;

class MPBrushSelector : public BaseDockWidget
{

  Q_OBJECT
public:
  MPBrushSelector(QWidget* parent = nullptr );

  bool isValid() { return !mBrushPresets.isEmpty(); }
  void loadToolBrushes(QString toolName);

  void initUI() override;
  void updateUI() override;

  void setCore(Editor* editor) { mEditor = editor; }

public slots:
  void reloadCurrentBrush();
  void selectBrush(QString brushName);
  void typeChanged(ToolType);
  void reloadBrushList();
  void updateBrushList(QString brushName, QString brushPreset);
  void showPresetManager();

signals:
  void brushSelected (ToolType toolType, const QString& brushGroup, const QString& brushName, const QByteArray& content);

protected:
  QVector<MPBrushPreset> mBrushPresets;
  QString mBrushesPath;

protected slots:
  void itemClicked ( QListWidgetItem *);

private:

  Editor* mEditor = nullptr;
  bool mTabsLoaded = false;

  void loadBrushFromFile(const QString& brushName);
  void populateList();
  void loadBrushes();
  void addToolTabs();

  void openConfigurator();
  void showNotImplementedPopup();
  void changeBrushPreset(int index, QString name, int data);

//  QTabWidget* mTabWidget;

  QVBoxLayout* mVLayout = nullptr;
  QMap<QString, QListWidget*> mToolListWidgets;
  ComboBox* mPresetComboBox;

  MPBrushPreset currentBrushPreset;

  MPBrushConfigurator* mBrushConfiguratorWidget = nullptr;

  QString currentPresetName;
  QString currentBrushName;
  QString currentToolName;
  QString oldToolname;
  ToolType currentToolType;
  QByteArray currentBrushData;

  QPointer<QListWidget> currentListWidget;
  QPointer<MPBrushPresetsWidget> mPresetsWidget;
};



#endif // MPBRUSHSELECTOR_H
