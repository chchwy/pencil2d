#ifndef SURFACE_H
#define SURFACE_H

#include <QList>
#include <QPixmap>

typedef struct Surface
{
    QList<QPoint> positions;
    QList<std::shared_ptr<QPixmap>> pixmaps;
    QRect bounds;

    Surface(){}
    Surface(QList<QPoint> positions, QList<std::shared_ptr<QPixmap>> pixmaps, QRect bounds = QRect()) :
        positions(positions), pixmaps(pixmaps), bounds(bounds)
    {
    }

    Surface& operator+=(const Surface& rhs) {

          pixmaps += rhs.pixmaps;
          positions += rhs.positions;
          extendBoundaries(rhs.bounds);
          return *this;
    }

    void extendBoundaries(const QRect& rect)
    {
        if (bounds.left() > rect.left()) { bounds.setLeft(rect.left()); }
        if (bounds.right() < rect.right()) { bounds.setRight(rect.right()); }
        if (bounds.top() > rect.top()) { bounds.setTop(rect.top()); }
        if (bounds.bottom() < rect.bottom()) { bounds.setBottom(rect.bottom()); }
    }

    QPoint topLeft() const
    {
        return bounds.topLeft();
    }

    int countTiles() const
    {
        Q_ASSERT(pixmaps.count() == positions.count());
        return pixmaps.count();
    }

    bool isEmpty() {
        return countTiles() == 0 ? true : false;
    }

    QPixmap& pixmapAt(int index)
    {
        return *pixmaps[index];
    }


    const QPixmap& pixmapAt(int index) const
    {
        return *pixmaps.at(index).get();
    }

    const QPoint& pointAt(int index) const
    {
        return positions.at(index);
    }

    void appendPixmap(const QPixmap& pixmap)
    {
        pixmaps.append(std::make_shared<QPixmap>(pixmap));
    }

    void appendPosition(const QPoint& pos)
    {
        positions.append(pos);
    }

    void clear()
    {
        pixmaps.clear();
        positions.clear();
        bounds = QRect();
    }

    bool contains(const QPixmap& pixmap) const {
        return pixmaps.contains(std::make_shared<QPixmap>(pixmap));
    }

    bool contains(const QPoint& point) const {
        return positions.contains(point);
    }
} Surface;

#endif // SURFACE_H
