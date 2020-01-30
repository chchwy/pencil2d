#ifndef MPMAPPINGWIDGET_H
#define MPMAPPINGWIDGET_H

#include "mappingconfiguratorwidget.h"
#include "brushsetting.h"

class MPMappingWidget : public MappingConfiguratorWidget
{
    Q_OBJECT
public:
    MPMappingWidget(QWidget* parent = nullptr);
    MPMappingWidget(QString description, qreal min, qreal max, BrushInputType inputType, QVector<QPointF> points, int maxPoints, QWidget* parent = nullptr);

    QSize sizeHint() const override { return QSize(150, 150); }

public slots:
    void updateMapping(QVector<QPointF> points);

signals:
    void mappingForInputUpdated(QVector<QPointF> points, BrushInputType inputType);
private:
    BrushInputType mInputType;

};

#endif // MPMAPPINGWIDGET_H
