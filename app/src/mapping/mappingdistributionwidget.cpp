#include "mappingdistributionwidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>

#include <QDebug>

#include "mathutils.h"
#include "brushsetting.h"

GridPainter::GridPainter()
{
}

void GridPainter::paint(QPainter& painter, int gridSpacingHorizontal, int gridSpacingVertical)
{
    int gridSizeW = gridSpacingHorizontal;
    int gridSizeH = gridSpacingVertical;

    QRect rect = painter.viewport();

    int left = rect.left();
    int right = rect.right();
    int top = rect.top();
    int bottom = rect.bottom();

    QPen pen(Qt::lightGray);
    pen.setCosmetic(true);
    painter.setPen(pen);
    painter.setWorldMatrixEnabled(true);
    painter.setBrush(Qt::NoBrush);

    painter.drawRect(rect);


    QVector<QLineF> lines;

    int count = 0;

    qDebug() << gridSizeW;
    qDebug() << gridSizeH;

    int numberOfLinesX = static_cast<int>(floor(right/gridSizeW));
    int numberOfLinesY = static_cast<int>(floor(bottom/gridSizeH));

    qDebug() << numberOfLinesX;
    qDebug() << numberOfLinesY;

    // draw vertical gridlines
    for (int x = left; x < right; x += gridSizeW) {

        // Prevents the last line from being shown when it's very close to the border
        if (count < numberOfLinesX) {
            lines << QLineF(x, top, x, bottom);
        }
        count++;
    }

    // draw horizontal gridlines
    count = 0;
    for (int y = top; y < bottom; y += gridSizeH) {

        // Prevents the last line from being shown when it's very close to the border
        if (count < numberOfLinesY) {
            lines << QLineF(left, y, right, y);
        }

        count++;
    }

    painter.drawLines(lines);
}

MappingDistributionWidget::MappingDistributionWidget(qreal min, qreal max, QVector<QPointF> points, QWidget *parent)
    : QWidget(parent), mPoints(points)
{
    mPointUniformSize = 3;
    mPointHitSize = 16;

    mActivePoint = -1;
    m_penWidth = 1;
    mMinX = min;
    mMaxX = max;
    mMinY = min;
    mMaxY = max;

    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setMouseTracking(true);
    setTabletTracking(true);

    adjustedRect = this->rect().adjusted(mPointUniformSize,mPointUniformSize,-mPointUniformSize,-mPointUniformSize);
    mGridPainter = GridPainter();

    mMappedPoints = mapPointsToWidget();
}

QVector<QPointF> MappingDistributionWidget::mapPointsToWidget()
{
    if (!mMappedPoints.isEmpty()) {
        mMappedPoints.clear();
    }
    for (QPointF point : mPoints) {
        qreal pointY = point.y();
        qreal pointX = point.x();

        mMinY = qMin(pointY, mMinY);
        mMinX = qMin(pointX, mMinX);
        mMaxY = qMax(pointY, mMaxY);
        mMaxX = qMax(pointX, mMaxX);
    }

    for (QPointF point : mPoints) {

        qreal pointY = point.y();
        qreal pointX = point.x();

        qDebug() << "===========";
        qDebug() << "orig " << "(x: " << pointX << ",y: " << pointY << ")";
        qreal normalizeX = MathUtils::normalize(pointX, mMinX, mMaxX);
        qreal normalizeY = MathUtils::normalize(pointY, mMinY, mMaxY);

        qDebug() << "normalized " << "(x: " << normalizeX << ",y: " << normalizeY << ")";
        qreal mappedX = MathUtils::linearMap(normalizeX, 0, 1, adjustedRect.left(), adjustedRect.right());
        qreal mappedY = MathUtils::linearMap(normalizeY, 0, 1, adjustedRect.bottom(), adjustedRect.top());
        qDebug() << "mapped " << "(x: " << mappedX << ",y: " << mappedY << ")";

        mMappedPoints << QPointF(mappedX, mappedY);
    }


    qDebug() << "minX is : " << mMinX << " minY is: " << mMinY;
    qDebug() << "maxX is : " << mMaxX << " maxY is: " << mMaxY;

    return mMappedPoints;
}

void MappingDistributionWidget::resetMapping()
{
    if (!mMappedPoints.isEmpty()) {
        mMappedPoints.clear();
        initializePoints();
    }

    mPoints = mapPointsFromWidget();
    emit mappingUpdated(mPoints);

    update();
}

QVector<QPointF> MappingDistributionWidget::mapPointsFromWidget()
{
    QVector<QPointF> mapToOrigPoints;
    for (QPointF point : mMappedPoints) {
        qreal normalizeX = MathUtils::normalize(point.x(), adjustedRect.left(), adjustedRect.right());
        qreal normalizeY = MathUtils::normalize(point.y(), adjustedRect.bottom(), adjustedRect.top());

        qreal mappedX = MathUtils::mapFromNormalized(normalizeX, mMinX, mMaxX);
        qreal mappedY = MathUtils::mapFromNormalized(normalizeY, mMinY, mMaxY);

        mapToOrigPoints << QPointF(mappedX, mappedY);
    }
    return mapToOrigPoints;
}

void MappingDistributionWidget::resizeEvent(QResizeEvent *)
{
    adjustedRect = this->rect().adjusted(mPointUniformSize,mPointUniformSize,-mPointUniformSize,-mPointUniformSize);
    mMappedPoints = mapPointsToWidget();
}

void MappingDistributionWidget::paintEvent(QPaintEvent*)
{
    if (mMappedPoints.isEmpty())
        initializePoints();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setViewport(adjustedRect);

    QPainterPath path;
    path.moveTo(mMappedPoints.at(0));

    for (int i=1; i<mMappedPoints.size(); ++i) {
        path.lineTo(mMappedPoints.at(i));
    }

    mGridPainter.paint(painter, adjustedRect.width()/4,adjustedRect.height()/4);

    QPen pen(Qt::blue);
    painter.strokePath(path, pen);

    // Draw the control points
    painter.setPen(QColor(255, 255, 255, 255));
    painter.setBrush(QColor(200, 200, 210, 255));

    for (int i=0; i < mMappedPoints.size(); ++i) {
        QPointF pos = mMappedPoints.at(i);

        if (i == mActivePoint) {
            painter.setPen(Qt::green);
        } else {
            painter.setPen(QColor(255, 255, 255, 255));
        }
        painter.drawText(QPointF(pos.x()+20,pos.y()), QString::number(i));
        painter.drawRect(QRectF(pos.x() - mPointUniformSize,
                                pos.y() - mPointUniformSize,
                                mPointUniformSize*2, mPointUniformSize*2));
    }
    painter.setPen(QPen(Qt::lightGray, 0, Qt::SolidLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawPolyline(mMappedPoints);
}


/// Creates three initial points
/// [y]
/// |          x
/// |
/// |     x
/// |
/// |x
/// |---------- [x]
void MappingDistributionWidget::initializePoints()
{
    mPoints.clear();
    QPointF center(adjustedRect.width() / 2, adjustedRect.height() / 2);
    QVector<QPointF> initPoints = { QPointF(0,adjustedRect.height()),
                                   center,
                                    QPointF(adjustedRect.width(),0)
                                  };

    for (QPointF point : initPoints) {
        mMappedPoints << point;
    }
}

void MappingDistributionWidget::mousePressEvent(QMouseEvent *e)
{
    mActivePoint = -1;
    qreal distance = -1;

    for (int i=0; i<mMappedPoints.size(); ++i) {
        qreal d = QLineF(e->pos(), mMappedPoints.at(i)).length();
        if ((distance < 0 && d < mPointHitSize) || d < distance) {
            distance = d;
            mActivePoint = i;
            break;
        }
    }

    if (mActivePoint == -1) {

        // Ensure not adding more points than allowed
        if (mMaxPoints > 0) {
            if (mMappedPoints.size() >= mMaxPoints) {
                return;
            }
        }

        for (int i=0; i < mMappedPoints.size(); ++i) {
            if (i+1 < mMappedPoints.size() && i >= 0) {
                QPointF nextPoint = mMappedPoints[i+1];
                QPointF prevPoint = mMappedPoints[i];

                if ((e->pos().x() <= nextPoint.x() && e->pos().x() >= prevPoint.x())) {
                    const int activePoint = i+1;
                    mMappedPoints.insert(activePoint, e->pos());
                    mActivePoint = activePoint;
                    break;
                }
            }
        }
    }

    mCanDrag = true;

    if (mActivePoint != -1) {
        mouseMoveEvent(e);
    }

    m_mousePress = e->pos();
}

void MappingDistributionWidget::mouseMoveEvent(QMouseEvent *e)
{
    // If we've moved more then 25 pixels, assume user is dragging
    if (mCanDrag) {
        if (!m_mouseDrag && QPoint(m_mousePress - e->pos()).manhattanLength() > 25) {
            m_mouseDrag = true;
        }
    }

    QPointF pos = e->pos();
    if (mCanDrag) {
        if (m_mouseDrag && mActivePoint >= 0 && mActivePoint < mMappedPoints.size()) {

            // boundary check
            if (pos.x() < adjustedRect.left()) {
                pos.setX(adjustedRect.left());
            }
            if (pos.x() > adjustedRect.right()) {
                pos.setX(adjustedRect.right());
            }
            if (pos.y() < adjustedRect.top()) {
                pos.setY(adjustedRect.top());
            }
            if (pos.y() > adjustedRect.bottom()) {
                pos.setY(adjustedRect.bottom());
            }


            // dragged point cannot pass previous or next point horizontally
            if (mActivePoint > 0 && mActivePoint < mMappedPoints.size()-1) {
                auto lastPoint = mMappedPoints[mActivePoint-1];
                auto nextPoint = mMappedPoints[mActivePoint+1];
                auto currentPoint = pos;

                if (currentPoint.x() < lastPoint.x()) {
                    pos.setX(lastPoint.x());
                } else if (currentPoint.x() > nextPoint.x()) {
                    pos.setX(nextPoint.x());
                }
                mMappedPoints[mActivePoint] = pos;
            }

            // Can only move first and last point vertically
            if (mActivePoint == mMappedPoints.size()-1 || mActivePoint == 0) {
                mMappedPoints[mActivePoint].setY(pos.y());
            }
        }
    }
    update();
}

void MappingDistributionWidget::mouseReleaseEvent(QMouseEvent *)
{
    mCanDrag = false;

    mPoints = mapPointsFromWidget();
    emit mappingUpdated(mPoints);
}

void MappingDistributionWidget::keyPressEvent(QKeyEvent *event)
{
    if (mMappedPoints.size() <= 2) {
        return;
    }

    // Avoid deleting the select the first and last point
    if (mActivePoint == mMappedPoints.count()-1 || mActivePoint == 0) {
        return;
    }

    if (event->key() == Qt::Key_Backspace) {
        mMappedPoints.removeAt(mActivePoint);
    }
    update();
}
