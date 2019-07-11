#ifndef BitmapSurface_H
#define BitmapSurface_H

#include "util.h"
#include <QRect>

#include "keyframe.h"
#include <QImage>
#include <QFuture>

#include "surface.h"

class BitmapSurface : public KeyFrame
{

public:
    BitmapSurface();
    BitmapSurface(const BitmapSurface& pieces);
    ~BitmapSurface() override;

    // Keyframe
    BitmapSurface* clone() const override;
    bool isLoaded() override;
    void loadFile() override;
    void unloadFile() override;

    /// Takes the pieces of the surface and paints them accordingly to form the whole image.
    /// returns a future image, painted in async
    QFuture<QImage> futureSurfaceAsImage();
    QImage surfaceAsImage();

    Status writeFile(const QString& filename);

    void appendBitmapSurface(const QPixmap& pixmap, const QPoint& pos);
    void appendBitmapSurface(const Surface& surface);

    Surface movedSurface(const Surface& inSurface, const QPoint& newPos);

    void paintSurfaceUsing(const QPixmap& inPixmap, const QPoint& newPos);

    /**
     * @brief touchedTiles
     * Will find and return points within and surrounding the selection
     * @param QRect rect
     * @return list of touched points
     */
    QList<QPoint> touchedTiles(const QRect& rect);

    void createNewSurfaceFromImage(const QImage& image, const QPoint& topLeft);
    void createNewSurfaceFromImage(const QString& path, const QPoint& topLeft);


    /** @brief BitmapSurface::surfaceFromPixmap
     * Intended to be used for image imports, selections etc...
     *
     * Will slice a big image into tiles
     * @param pixmap
     * @return Surface */
    Surface surfaceFromPixmap(QPixmap& pixmap);

    Surface surfaceFromBounds(const QRect& bounds);

    bool isTransparent(QImage& image);

    const QRect getRectForPoint(const QPoint& point, const QSize size);
    const QRect getRectForPoint(const QPoint& point);

    const QPixmap getPixmapAt(const int index) { return mSurface.pixmapAt(index); }

    const QPixmap getPixmapFromTilePos(const QPoint& pos);
    inline QPoint getTilePos(const QPoint& idx);
    inline QPoint getTileIndex(const QPoint& pos);
    inline QPointF getTileFIndex(const QPoint& pos);

    void extendBoundaries(const QRect &rect);

    QPixmap cutSurfaceAsPixmap(const QRect selection);
    QPixmap copySurfaceAsPixmap(const QRect selection);

    /**
     * @brief BitmapSurface::intersectedSurface
     * Returns a Surface containing the tiles that intersected the region
     * @param rect
     * @return Surface
     */
    Surface intersectedSurface(const QRect rect);

    void eraseSelection(const QRect selection);
    void eraseSelection(const QPoint pos, QPixmap& pixmap, const QRect selection);
    void fillSelection(const QPoint &pos, QPixmap &pixmap, QColor color, const QRect selection);
    void clear();

    QList<std::shared_ptr<QPixmap> > pixmaps();
    QList<QPoint> tilePositions();

    inline const QRect bounds() { return mSurface.bounds; }

    QImage cachedSurfaceImage() { return mCachedSurface; }

    void renderSurfaceImage();

    Surface surface() { return mSurface; }
    const Surface readOnlySurface() const {return mSurface; }

public slots:
    void setSurfaceFromFuture(int index);

private:

    QList<QPoint> scanForSurroundingTiles(const QRect& rect);
    QList<QPoint> scanForTilesAtSelection(const QRect& rect);

    const QSize TILESIZE = QSize(64,64);

    Surface mSurface;
    QImage mCachedSurface;
};

#endif // BitmapSurface_H
