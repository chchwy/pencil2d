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

    QPoint topLeft() const
    {
        return bounds.topLeft();
    }

    int countTiles() const
    {
        return pixmaps.count();
    }

    bool isEmpty() {
        return countTiles() == 0 ? true : false;
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
} Surface;

#endif // SURFACE_H
