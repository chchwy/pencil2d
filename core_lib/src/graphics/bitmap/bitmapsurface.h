#ifndef BitmapSurface_H
#define BitmapSurface_H

#include "util.h"
#include <QRect>

class QPixmap;


class BitmapSurface
{
public:
    BitmapSurface();
    BitmapSurface(BitmapSurface& pieces);
    ~BitmapSurface();

    /// For debugging only
    void paintWholeImage(QString path);

    void addBitmapPiece(const QPixmap& pixmap, const QRect& region);
    void createPiecesFromImage(QImage& image);

    QImage getSubImageFromImage(QImage& image, QRect rect);
    bool isTransparent(QImage& image);

    const QSize size();
    const QRect getBoundingRectAtIndex(const int index);
    const QPixmap getPixmapAtIndex(const int index);

    inline QPoint getTilePos(const QPoint& idx);
    inline QPoint getTileIndex(const QPoint& pos);
    inline QPointF getTileFIndex(const QPoint& pos);

    const QSize TILESIZE = QSize(64,64);

private:

    inline const std::shared_ptr<QPixmap> getPixmapAt(const int index) { return mPixmaps.at(index); }

    QVector<std::shared_ptr< QPixmap >> mPixmaps;
    QVector<QRect> mBoundingRects;
    QSize mCombinedSize;
};

#endif // BitmapSurface_H
