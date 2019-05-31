#ifndef BitmapSurface_H
#define BitmapSurface_H

#include "util.h"
#include <QRect>

#include "keyframe.h"
#include <QImage>
#include <QFuture>

class QPixmap;

typedef struct Surface
{
    QList<QPoint> positions;
    QList<QPixmap> pixmaps;
    Surface(QList<QPoint> positions, QList<QPixmap> pixmaps) : positions(positions), pixmaps(pixmaps)
    {
    }

    int countTiles()
    {
        return pixmaps.count();
    }

    const QPixmap& pixmapAt(int index)
    {
        return pixmaps.at(index);
    }

    const QPoint pointAt(int index)
    {
        return positions.at(index);
    }
} Surface;

class BitmapSurface : public KeyFrame
{

public:
    BitmapSurface();
    BitmapSurface(BitmapSurface& pieces);
    ~BitmapSurface() override;

    // Keyframe
    BitmapSurface* clone() override;
    bool isLoaded() override;
    void loadFile() override;
    void unloadFile() override;

    /// Takes the pieces of the surface and paints them accordingly to form the whole image.
    /// returns a future image, painted in async
    QFuture<QImage> surfaceAsImage();

    Status writeFile(const QString& filename);

    void appendBitmapSurface(const QPixmap& pixmap, const QPoint& pos);

    void appendToSurfaceFromImage(QImage& image, QPoint& topLeft);
    void createPiecesFromImage(QString& path, QPoint& topLeft);

    QImage getSubImageFromImage(const QImage& image, const QRect& rect);
    bool isTransparent(QImage& image);

    const QRect getBoundingRectAtIndex(const QPoint& idx);

    const QPixmap getPixmapFromTilePos(const QPoint& pos);
    inline QPoint getTilePos(const QPoint& idx);
    inline QPoint getTileIndex(const QPoint& pos);
    inline QPointF getTileFIndex(const QPoint& pos);

    void extendBoundaries(QRect rect);

    QPixmap cutSurfaceAsPixmap(const QRect selection);

    /**
     * @brief BitmapSurface::intersectedSurface
     * Returns a Surface containing the intersected area
     * Additionally: deletes selected area
     * @param rect
     * @return Surface
     */
    Surface intersectedSurface(const QRect rect);

    void eraseSelection(const QPoint pos, QPixmap& pixmap, const QRect selection);
    void fillSelection(const QPoint pos, QPixmap &pixmap, QColor color, const QRect selection);
    void clear();

    QVector<std::shared_ptr< QPixmap >> pixmaps();
    QVector<QPoint> tilePositions();

    inline const QRect bounds() { return mBounds; }

    QImage surfaceImage() { return mCachedSurface; }

    void renderSurfaceImage();

public slots:
    void setSurfaceFromFuture(int index);

private:

    const std::shared_ptr<QPixmap> getPixmapAt(const int index) { return mPixmaps.at(index); }
    const QSize TILESIZE = QSize(64,64);

    QVector<std::shared_ptr< QPixmap >> mPixmaps;
    QVector<QPoint> mTilePositions;
    QRect mBounds;
    QImage mCachedSurface;
};

#endif // BitmapSurface_H
