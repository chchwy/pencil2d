#ifndef MAPPINGDISTRIBUTIONWIDGET_H
#define MAPPINGDISTRIBUTIONWIDGET_H

#include "qwidget.h"

#include <QPainter>

class GridPainter
{
public:
    GridPainter();

    void paint(QPainter& painter, int gridSpacingHorizontal, int gridSpacingVertical);
};

class MappingDistributionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MappingDistributionWidget(qreal min, qreal max, QVector<QPointF> points, QWidget *parent = nullptr);

    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    QSize sizeHint() const override { return QSize(500, 500); }

    int maxMappingPoints() { return mMaxPoints; }
    void setMaxMappingPoints(int maxPoints) { mMaxPoints = maxPoints; }

signals:
    void clicked();
    void mappingUpdated(QVector<QPointF> points);

public slots:
    void resetMapping();

private:

    void setupUI();
    QVector<QPointF> mapPointsToWidget();
    QVector<QPointF> mapPointsFromWidget();

    void initializePoints();

    bool mCanDrag = false;

    qreal m_penWidth;
    int mPointCount;
    int mPointUniformSize;
    int mPointHitSize;
    int mActivePoint;

    /// Max points that can be added
    /// 0 means that there's no limit
    int mMaxPoints = 0;

    QVector<QPointF> mPoints;

    bool initUI = false;

    qreal mMinX = 0.0;
    qreal mMaxX = 0.0;
    qreal mMinY = 0.0;
    qreal mMaxY = 0.0;

    QVector<QPointF> mMappedPoints;

    QTransform viewTransform;

    QPoint m_mousePress;
    bool m_mouseDrag;

    GridPainter mGridPainter;

    QRect adjustedRect;
    QString mDescription;
};

#endif // MAPPINGDISTRIBUTIONWIDGET_H
