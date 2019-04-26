#ifndef BitmapSurface_H
#define BitmapSurface_H

#include "util.h"
#include <QRect>

#include "keyframe.h"

class QPixmap;


class BitmapSurface : public KeyFrame
{
public:
    BitmapSurface();
    BitmapSurface(BitmapSurface& pieces);
    ~BitmapSurface();

    // Keyframe
    BitmapSurface* clone() override;
    bool isLoaded() override;
    void loadFile() override;
    void unloadFile() override;

    /// Takes the pieces of the surface and paints them accordingly to form the whole image.
    QImage surfaceAsImage();

    Status writeFile(const QString& filename);

    void addBitmapPiece(const QPixmap& pixmap, const QPoint& pos);

    void createPiecesFromImage(QImage& image, QPoint& topLeft);
    void createPiecesFromImage(QString& path, QPoint& topLeft);

    QImage getSubImageFromImage(QImage& image, QRect& rect);
    bool isTransparent(QImage& image);

    const QRect getBoundingRectAtIndex(const int& index);
    const QPixmap getPixmapAtIndex(const int& index);

    void extendBoundaries(QRect rect);

    inline QPoint getTilePos(const QPoint& idx);
    inline QPoint getTileIndex(const QPoint& pos);
    inline QPointF getTileFIndex(const QPoint& pos);
    inline const QRect bounds() { return mBounds; }

    const QSize TILESIZE = QSize(64,64);

private:

    inline const std::shared_ptr<QPixmap> getPixmapAt(const int index) { return mPixmaps.at(index); }

    QVector<std::shared_ptr< QPixmap >> mPixmaps;
    QVector<QPoint> mTilePositions;
    QRect mBounds;
};

#endif // BitmapSurface_H
