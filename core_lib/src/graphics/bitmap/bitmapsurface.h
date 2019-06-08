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
    BitmapSurface(BitmapSurface& pieces);
    ~BitmapSurface() override;

    // Keyframe
    BitmapSurface* clone() override;
    bool isLoaded() override;
    void loadFile() override;
    void unloadFile() override;

    /// Takes the pieces of the surface and paints them accordingly to form the whole image.
    /// returns a future image, painted in async
    QFuture<QImage> futureSurfaceAsImage();
    QImage surfaceAsImage();

    Status writeFile(const QString& filename);

    void appendBitmapSurface(const QPixmap& pixmap, const QPoint& pos);

    void moveSurfaceTo(const Surface& surface, const QPoint& newPos);

    void createNewSurfaceFromImage(QImage& image, QPoint& topLeft);
    void createNewSurfaceFromImage(QString& path, QPoint& topLeft);


    /** @brief BitmapSurface::surfaceFromPixmap
     * Intended to be used for image imports, selections etc...
     *
     * Will slice a big image into tiles
     * @param pixmap
     * @return Surface */
    Surface surfaceFromPixmap(QPixmap& pixmap);

    bool isTransparent(QImage& image);

    const QRect getBoundingRectAtIndex(const QPoint& idx, const QSize size);

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

public slots:
    void setSurfaceFromFuture(int index);

private:
    const QSize TILESIZE = QSize(64,64);

    Surface mSurface;
    QImage mCachedSurface;
};

#endif // BitmapSurface_H
