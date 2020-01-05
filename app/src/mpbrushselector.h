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


// MPBrushSelector is a TabWidget showing the various brushes (display the small screenshots)
// it allows the user to select it and emit a signal.
// NOTE : The order is not properly kept as I did not realize the file order.conf
//        was containing this information. Will be fixed soon.
//
class MPBrushSelector : public BaseDockWidget
{

  Q_OBJECT
public:
  MPBrushSelector( const QString& brushLibPath, QWidget* parent = 0 );

  bool isValid() { return !m_brushLib.isEmpty(); }
  void loadToolBrushes(QString toolName);

  void initUI() override;
  void updateUI() override;

public slots:
  void selectBrush(QString brushName = QString()); // Give the brush name (no extension) i.e. : "classic/blend+paint"
  void typeChanged(ToolType);

signals:
  void brushSelected (QString toolName, QString brushName, const QByteArray& content);

protected:
  QMap<QString, QStringList> m_brushLib;
  const QString              m_brushesPath;

protected slots:
  void itemClicked ( QListWidgetItem *);

private:

  void populateList();
  bool anyBrushSelected();

  QTabWidget* mTabWidget;

};



#endif // MPBRUSHSELECTOR_H
