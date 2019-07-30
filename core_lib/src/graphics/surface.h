#ifndef SURFACE_H
#define SURFACE_H

#include <QList>
#include <QPixmap>

inline uint qHash(const QPoint& key)
{
    return (static_cast<uint>(key.x()) << 16) + static_cast<uint>(key.y());
}

typedef QHash<QPoint, std::shared_ptr<QPixmap>> Tiles;

struct Surface
{

    Tiles tiles;
    QRect bounds;

    Surface(){}
    Surface(Tiles tiles, QRect bounds = QRect()) :
        tiles(tiles), bounds(bounds)
    {
    }

    Surface& operator+=(const Surface& rhs) {
          tiles.unite(rhs.tiles);
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
        return tiles.count();
    }

    bool isEmpty() {
        return tiles.count() == 0;
    }

    QPixmap& pixmapAtPos(const QPoint& pos)
    {
        return *tiles[pos];
    }

    const QPixmap& pixmapAtPos(const QPoint& pos) const
    {
        return *tiles.value(pos).get();
    }

    const QPoint posFromPixmap(const std::shared_ptr<QPixmap>& pixmap) const
    {
        return tiles.key(pixmap);
    }

    void appendTile(const QPixmap& pixmap, const QPoint& pos)
    {
        tiles.insert(pos, std::make_shared<QPixmap>(pixmap));
    }

    void clear()
    {
        tiles.clear();
        bounds = QRect();
    }

    bool contains(const QPoint& pos) const {
        return tiles.contains(pos);
    }
};

#endif // SURFACE_H
