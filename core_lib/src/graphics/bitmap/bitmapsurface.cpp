#include "bitmapsurface.h"

#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QtMath>
#include <QtConcurrent>

BitmapSurface::BitmapSurface()
{
    QList<QPoint> positions;
    QList<std::shared_ptr<QPixmap>> pixmaps;
    mSurface = Surface(positions, pixmaps, QRect());
}

BitmapSurface::BitmapSurface(BitmapSurface& pieces) : KeyFrame (pieces),
    mSurface(pieces.mSurface)
{
}

BitmapSurface::~BitmapSurface()
{
}

BitmapSurface* BitmapSurface::clone()
{
    return new BitmapSurface(*this);
}

void BitmapSurface::loadFile()
{
//    if (mImage == nullptr)
//    {
//        mImage = std::make_shared<QImage>(fileName());
//        mBounds.setSize(mImage->size());
//        mMinBound = false;
//    }
}

void BitmapSurface::unloadFile()
{
//    if (isModified() == false)
//    {
//        mImage.reset();
//    }
}

bool BitmapSurface::isLoaded()
{
    return (!mSurface.pixmaps.isEmpty());
}


bool BitmapSurface::isTransparent(QImage& image)
{
    if (!image.hasAlphaChannel()) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }

    for (int x = 0 ; x < image.width() ; x++) {
        for (int y = 0 ; y < image.height() ; y++) {
            QRgb currentPixel = (image.pixel(x, y));

            if (qAlpha(currentPixel) != 0) {
                return false;
            }
        }
    }
    return true;
}

void BitmapSurface::createNewSurfaceFromImage(QString& path, QPoint& topLeft)
{
    QImage image(path);
    createNewSurfaceFromImage(image, topLeft);
}

void BitmapSurface::appendBitmapSurface(const QPixmap& pixmap, const QPoint& pos)
{
    mSurface.appendPixmap(pixmap);
    mSurface.appendPosition(pos);

    extendBoundaries(QRect(pos, pixmap.size()));
}

void BitmapSurface::renderSurfaceImage()
{
    mCachedSurface = surfaceAsImage();
}

void BitmapSurface::extendBoundaries(const QRect& rect)
{
    QRect& bounds = mSurface.bounds;
    if (bounds.left() > rect.left()) { bounds.setLeft(rect.left()); }
    if (bounds.right() < rect.right()) { bounds.setRight(rect.right()); }
    if (bounds.top() > rect.top()) { bounds.setTop(rect.top()); }
    if (bounds.bottom() < rect.bottom()) { bounds.setBottom(rect.bottom()); }
}

Surface BitmapSurface::surfaceFromPixmap(QPixmap& pixmap)
{
    float imageWidth = static_cast<float>(pixmap.width());
    float imageHeight = static_cast<float>(pixmap.height());
    float tileWidth = static_cast<float>(TILESIZE.width());
    float tileHeight = static_cast<float>(TILESIZE.height());
    int nbTilesOnWidth = static_cast<int>(ceil(imageWidth / tileWidth));
    int nbTilesOnHeight = static_cast<int>(ceil(imageHeight / tileHeight));
    QList<QPoint> positions;
    QList<std::shared_ptr<QPixmap>> pixmaps;
    QPixmap paintTo(TILESIZE);
    const QRect& bounds = pixmap.rect();

    for (int h=0; h < nbTilesOnHeight; h++) {
        for (int w=0; w < nbTilesOnWidth; w++) {
            paintTo.fill(Qt::transparent);

            QPoint idx(w, h);
            QPoint tilePos = getTilePos(idx);

            QRect tileRect = QRect(tilePos, TILESIZE);
            QPixmap tilePixmap= pixmap.copy(tileRect);

            QPainter painter(&paintTo);
            painter.drawPixmap(QPoint(), tilePixmap);
            painter.end();

            pixmaps.append(std::make_shared<QPixmap>(paintTo));
            positions.append(tilePos);
        }
    }
    return Surface(positions, pixmaps, bounds);
}

void BitmapSurface::moveSurfaceTo(const Surface& surface, const QPoint& newPos)
{
    for (int i = 0; i < surface.countTiles(); i++)
    {
        const QPixmap& pix = surface.pixmapAt(i);
        auto point = surface.pointAt(i);

        const QPoint& nonInterpolatedPos = point + newPos;
        const QPoint& pointIndex = getTileIndex(point);
        const QPoint& pointIndexNewPos = getTileIndex(newPos);
        const QPoint& tilePos = getTilePos(pointIndex);
        QPoint interpolatedPos = getTilePos(pointIndexNewPos);
        interpolatedPos += tilePos;

        // TODO: figure out a way to calculate where tile should be, has to be mapped to positions
        // otherwise selection will be moved

//        if (newPos.x()%64 != 0) {
//            const QPoint& posIndex = getTileIndex(point);
//            point = getTilePos(posIndex);
//            point += newPos;
////        }

        qDebug() << "point after moving: " << point;
//        qDebug() << surface.topLeft();

        this->appendBitmapSurface(pix, interpolatedPos);
    }

//    qDebug() << "number of tiles after moving: " << this->mSurface.countTiles();
}

void BitmapSurface::createNewSurfaceFromImage(QImage& image, QPoint& topLeft)
{
    float imageWidth = static_cast<float>(image.width());
    float imageHeight = static_cast<float>(image.height());
    float tileWidth = static_cast<float>(TILESIZE.width());
    float tileHeight = static_cast<float>(TILESIZE.height());
    int nbTilesOnWidth = static_cast<int>(ceil(imageWidth / tileWidth));
    int nbTilesOnHeight = static_cast<int>(ceil(imageHeight / tileHeight));

    QPixmap paintTo(TILESIZE);
    mSurface = Surface();
    mSurface.bounds = QRect(topLeft, image.size());

    for (int h=0; h < nbTilesOnHeight; h++) {
        for (int w=0; w < nbTilesOnWidth; w++) {
            paintTo.fill(Qt::transparent);
            const QPoint idx(w, h);
            const QPoint& tilePos = getTilePos(idx);

            const QRect& tileRect = QRect(tilePos, TILESIZE);
            const QImage& tileImage = image.copy(tileRect);

            QPainter painter(&paintTo);
            painter.drawImage(QPoint(), tileImage);
            painter.end();

            mSurface.appendPixmap(paintTo);
            mSurface.appendPosition(tilePos);
        }
    }
}

QFuture<QImage> BitmapSurface::futureSurfaceAsImage()
{
    auto asyncPaint = [this]() {
        return surfaceAsImage();
    };
    return QtConcurrent::run(asyncPaint);
}

QImage BitmapSurface::surfaceAsImage()
{
    QImage paintedImage(mSurface.bounds.size(), QImage::Format_ARGB32_Premultiplied);
    paintedImage.fill(Qt::transparent);

    QPainter painter(&paintedImage);
    painter.translate(-mSurface.topLeft());

    for (int i = 0; i < mSurface.countTiles(); i++)
    {
        const QPixmap& pix = mSurface.pixmapAt(i);
        const QPoint& pos = mSurface.pointAt(i);
        painter.drawPixmap(pos, pix);
    }
    painter.end();

    return paintedImage;
}

void BitmapSurface::eraseSelection(const QPoint pos, QPixmap& pixmap, const QRect selection)
{
    fillSelection(pos, pixmap, Qt::transparent, selection);
}

void BitmapSurface::eraseSelection(const QRect selection)
{
    for (int i = 0; i < mSurface.countTiles(); i++) {

        QPixmap& pixmap = *mSurface.pixmaps[i].get();
        const QPoint& pos = mSurface.pointAt(i);
        eraseSelection(pos, pixmap, selection);
    }
}

void BitmapSurface::fillSelection(const QPoint& pos, QPixmap& pixmap, QColor color, const QRect selection)
{
    QPainter painter(&pixmap);
    painter.translate(-pos);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    const QRect& intersection = selection.intersected(getBoundingRectAtIndex(pos, pixmap.size()));
    painter.fillRect(intersection, color);
    painter.end();

}

Surface BitmapSurface::intersectedSurface(const QRect rect)
{
    QList<std::shared_ptr<QPixmap>> selectionImages;
    QList<QPoint> selectionPos;
    for (int i = 0; i < mSurface.countTiles(); i++)
    {
        const QPixmap& pix = *mSurface.pixmaps[i].get();
        const QPoint& pos = mSurface.pointAt(i);

        if (getBoundingRectAtIndex(pos, pix.size()).intersects(rect)) {
            selectionImages.append(std::make_shared<QPixmap>(pix));
            selectionPos.append(pos);
        }
    }

    return Surface(selectionPos, selectionImages, rect);
}

QPixmap BitmapSurface::cutSurfaceAsPixmap(const QRect selection)
{
    Q_ASSERT(!selection.isEmpty());

    Surface intersectSurface = intersectedSurface(selection);

    eraseSelection(selection);

    QPixmap paintedImage(selection.size());
    paintedImage.fill(Qt::transparent);

    QPainter painter(&paintedImage);
    painter.translate(-selection.topLeft());
    for (int i = 0; i < intersectSurface.countTiles(); i++)
    {
        const QPixmap& pix = intersectSurface.pixmapAt(i);
        const QPoint& pos = intersectSurface.pointAt(i);
        painter.drawPixmap(pos, pix);
    }
    return paintedImage;
}


QPixmap BitmapSurface::copySurfaceAsPixmap(const QRect selection)
{
    Q_ASSERT(!selection.isEmpty());

    const Surface& intersectSurface = intersectedSurface(selection);

    QPixmap paintedImage(selection.size());
    paintedImage.fill(Qt::transparent);

    QPainter painter(&paintedImage);
    painter.translate(-selection.topLeft());
    for (int i = 0; i < intersectSurface.countTiles(); i++)
    {
        const QPixmap& pix = intersectSurface.pixmapAt(i);
        const QPoint& pos = intersectSurface.pointAt(i);
        painter.drawPixmap(pos, pix);
    }
    return paintedImage;
}

QList<QPoint> BitmapSurface::tilePositions()
{
    if (mSurface.countTiles() == 0) { return QList<QPoint>(); }
    return mSurface.positions;
}

QList<std::shared_ptr<QPixmap>> BitmapSurface::pixmaps()
{
    if (mSurface.countTiles()) { return QList<std::shared_ptr<QPixmap>>(); }
    return mSurface.pixmaps;
}

void BitmapSurface::clear()
{
    mCachedSurface = QImage();
    mSurface.clear();
}

Status BitmapSurface::writeFile(const QString& filename)
{
    if (mSurface.isEmpty()) {
        return Status::FAIL;
    }

    if (mSurface.pixmaps.first()) {
        const QImage& image = surfaceAsImage();
        bool b = image.save(filename);
        return (b) ? Status::OK : Status::FAIL;
    }
    return Status::FAIL;
}

const QPixmap BitmapSurface::getPixmapFromTilePos(const QPoint& pos)
{
    for (int i = 0; i < mSurface.countTiles(); i++) {
        const QPoint& tilePos = mSurface.pointAt(i);

        if (tilePos == pos) {
            return mSurface.pixmapAt(i);
        }
    }
    return QPixmap();
}

const QRect BitmapSurface::getBoundingRectAtIndex(const QPoint& idx, const QSize size)
{
    return QRect(idx.x(), idx.y(), size.width(), size.height());
}

inline QPoint BitmapSurface::getTilePos(const QPoint& idx)
{
    return QPoint(TILESIZE.width()*idx.x(), TILESIZE.height()*idx.y());
}

inline QPoint BitmapSurface::getTileIndex(const QPoint& pos)
{
    return QPoint(pos.x()/TILESIZE.width(), pos.y()/TILESIZE.height());
}

inline QPointF BitmapSurface::getTileFIndex(const QPoint& pos)
{
    return QPointF((qreal)pos.x()/TILESIZE.width(), (qreal)pos.y()/TILESIZE.height());
}


