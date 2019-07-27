#include "bitmapsurface.h"

#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QtMath>
#include <QtConcurrent>

BitmapSurface::BitmapSurface()
{

    QList<QPoint> positions;
    positions.append(QPoint(0,0));
    QList<std::shared_ptr<QPixmap>> pixmaps;
    pixmaps.append(std::make_shared<QPixmap>(QPixmap(0,0)));
    mSurface = Surface(positions, pixmaps, QRect(0,0,1,1));
}

BitmapSurface::BitmapSurface(const BitmapSurface& pieces) : KeyFrame (pieces),
    mSurface(pieces.mSurface)
{
}

BitmapSurface::~BitmapSurface()
{
}

BitmapSurface* BitmapSurface::clone() const
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

void BitmapSurface::createNewSurfaceFromImage(const QString& path, const QPoint& topLeft)
{
    QImage image(path);
    createNewSurfaceFromImage(image, topLeft);
}

void BitmapSurface::createNewSurfaceFromImage(const QImage& image, const QPoint& topLeft)
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
            const QPoint& idx = QPoint(w, h);
            const QPoint& tilePos = getTilePos(idx);

            const QRect& tileRect = QRect(tilePos, TILESIZE);
            const QImage& tileImage = image.copy(tileRect);

            QPainter painter(&paintTo);
            painter.drawImage(QPoint(), tileImage);
            painter.end();

            mSurface.appendPixmap(paintTo);
            mSurface.appendPosition(topLeft+tilePos);
        }
    }
}

void BitmapSurface::appendBitmapSurface(const QPixmap& pixmap, const QPoint& pos)
{
    mSurface.appendPixmap(pixmap);
    mSurface.appendPosition(pos);

    extendBoundaries(QRect(pos, pixmap.size()));
}

void BitmapSurface::appendBitmapSurface(const Surface &surface)
{
    mSurface += surface;
}

void BitmapSurface::renderSurfaceImage()
{
    mCachedSurface = surfaceAsImage();
}

void BitmapSurface::extendBoundaries(const QRect& rect)
{
    mSurface.extendBoundaries(rect);
}

Surface BitmapSurface::surfaceFromBounds(const QRect& bounds)
{
    const float& imageWidth = static_cast<float>(bounds.width());
    const float& imageHeight = static_cast<float>(bounds.height());
    const float& tileWidth = static_cast<float>(TILESIZE.width());
    const float& tileHeight = static_cast<float>(TILESIZE.height());
    const int& nbTilesOnWidth = static_cast<int>(ceil(imageWidth / tileWidth));
    const int& nbTilesOnHeight = static_cast<int>(ceil(imageHeight / tileHeight));
    QList<QPoint> positions;
    QList<std::shared_ptr<QPixmap>> pixmaps;
    QPixmap paintTo(TILESIZE);

    QList<QPoint> corners;

    corners.append(bounds.topLeft());
    corners.append(bounds.topRight());
    corners.append(bounds.bottomLeft());
    corners.append(bounds.bottomRight());

    for (int h=0; h < nbTilesOnHeight; h++) {
        for (int w=0; w < nbTilesOnWidth; w++) {
            paintTo.fill(Qt::transparent);

            const QPoint& idx = QPoint(w, h);
            const QPoint& origTilePos = getTilePos(idx);

            for (int corner = 0; corner < corners.count(); corner++) {

                QPoint tileOffsetCenter = getTilePos(getTileIndex(corners[corner]));
                tileOffsetCenter += origTilePos;

                const QPoint& topLeft = tileOffsetCenter+QPoint(-64,-64);

                if (bounds.intersects(getRectForPoint(topLeft))) {
                    positions.append(topLeft);
                    pixmaps.append(std::make_shared<QPixmap>(paintTo));
                }
            }
        }
    }
    return Surface(positions, pixmaps, bounds);
}

Surface BitmapSurface::surfaceFromPixmap(QPixmap& pixmap)
{
    const float& imageWidth = static_cast<float>(pixmap.width());
    const float& imageHeight = static_cast<float>(pixmap.height());
    const float& tileWidth = static_cast<float>(TILESIZE.width());
    const float& tileHeight = static_cast<float>(TILESIZE.height());
    const int& nbTilesOnWidth = static_cast<int>(ceil(imageWidth / tileWidth));
    const int& nbTilesOnHeight = static_cast<int>(ceil(imageHeight / tileHeight));

    QList<QPoint> positions;
    QList<std::shared_ptr<QPixmap>> pixmaps;
    QPixmap paintTo(TILESIZE);
    const QRect& bounds = pixmap.rect();

    QList<QPoint> corners;

    corners.append(bounds.topLeft());
    corners.append(bounds.topRight());
    corners.append(bounds.bottomLeft());
    corners.append(bounds.bottomRight());

    for (int h=0; h < nbTilesOnHeight; h++) {
        for (int w=0; w < nbTilesOnWidth; w++) {
            paintTo.fill(Qt::transparent);

            const QPoint& idx = QPoint(w, h);
            const QPoint& origTilePos = getTilePos(idx);
            const QRect& tileRect = QRect(origTilePos, TILESIZE);
            const QPixmap& tilePixmap = pixmap.copy(tileRect);

            QPainter painter(&paintTo);
            painter.drawPixmap(QPoint(), tilePixmap);
            painter.end();

            positions.append(origTilePos);
            pixmaps.append(std::make_shared<QPixmap>(paintTo));
        }
    }
    return Surface(positions, pixmaps, bounds);
}

Surface BitmapSurface::movedSurface(const Surface& inSurface, const QPoint& newPos)
{
    Surface newSurface;
    newSurface.bounds = QRect(newPos, inSurface.bounds.size());
    for (int i = 0; i < inSurface.countTiles(); i++)
    {
        const QPixmap& pix = inSurface.pixmapAt(i);
        const QPoint& point = inSurface.pointAt(i);

        const QPoint& nonInterpolatedPos = point + newPos;
        const QPoint& pointIndex = getTileIndex(point);
        const QPoint& pointIndexNewPos = getTileIndex(newPos);
        const QPoint& tilePos = getTilePos(pointIndex);
        QPoint interpolatedPos = getTilePos(pointIndexNewPos);
        interpolatedPos += tilePos;

        newSurface.appendPixmap(pix);
        newSurface.appendPosition(nonInterpolatedPos);
    }
    return newSurface;
}

void BitmapSurface::paintSurfaceUsing(const QPixmap& inPixmap, const QPoint& newPos)
{
    Surface outSurface;

    QPixmap outPix = QPixmap(TILESIZE);
    outPix.fill(Qt::transparent);

    const QRect& adjustedPixRect = QRect(newPos, inPixmap.size());
    const QList<QPoint>& touchedPoints = touchedTiles(adjustedPixRect);

    // paint input pixmap on tiles
    for (int point = 0; point < touchedPoints.count(); point++) {

        const QPoint& touchedPoint = touchedPoints.at(point);
        QPainter painter(&outPix);
        outPix.fill(Qt::transparent);

        painter.save();
        painter.translate(-touchedPoint);
        painter.drawPixmap(newPos, inPixmap);
        painter.restore();
        painter.end();

        QImage testImage = outPix.toImage();
        if (isTransparent(testImage)) {
            continue;
        }

        outSurface.appendPixmap(outPix);
        outSurface.appendPosition(touchedPoint);
        outSurface.bounds = adjustedPixRect;
    }

    Surface extraSurface;

    // paint new tiles on previous tiles if possible, otherwise
    // prepare to be added to bitmapsurface
    for (int t = 0; t < outSurface.countTiles(); t++)
    {
        QPixmap newPix = outSurface.pixmapAt(t);
        QPoint newPos = outSurface.pointAt(t);
        QPixmap paintToPix = QPixmap(newPix.size());
        paintToPix.fill(Qt::transparent);

        bool noMatch = false;
        for (int i = 0; i < this->mSurface.countTiles(); i++)
        {
            QPixmap& existingPix = mSurface.pixmapAt(i);
            QPoint existingPos = mSurface.pointAt(i);

            if (existingPos == newPos) {
                QPainter painter(&existingPix);
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                painter.drawPixmap(QPoint(), newPix);
                painter.end();
            } else {

                if (mSurface.contains(newPos)) {
                    continue;
                }
                noMatch = true;
            }
        }

        if (noMatch)
        {
            extraSurface.appendPixmap(newPix);
            extraSurface.appendPosition(newPos);
            extraSurface.bounds = outSurface.bounds;
            noMatch = false;
        }
    }

    appendBitmapSurface(extraSurface);
}

QList<QPoint> BitmapSurface::scanForSurroundingTiles(const QRect& rect)
{
    QPoint kernel[] = {QPoint(-64,-64),QPoint(0,-64), QPoint(64,-64),
                      QPoint(-64,0), QPoint(0,0), QPoint(64,0),
                      QPoint(-64,64), QPoint(0,64), QPoint(64,64)};

    QList<QPoint> points;
    QList<QPoint> corners;

    corners.append({rect.topLeft(), rect.topRight(), rect.bottomLeft(), rect.bottomRight()});
    for (unsigned int i = 0; i < sizeof(kernel)/sizeof(kernel[0]); i++) {

        for (int p = 0; p < corners.count(); p++) {
            const QPoint& corner = corners[p];
            const QPoint& idx = getTileIndex(corner+kernel[i]);
            const QPoint& pos = getTilePos(idx);
            const QRect& rectToIntersect = getRectForPoint(pos);

            if (rectToIntersect.intersects(rect)) {
                if (points.contains(pos)) {
                    continue;
                }

                points.append(pos);
            }
        }
    }

    return points;
}

QList<QPoint> BitmapSurface::scanForTilesAtSelection(const QRect& rect)
{
    const float& imageWidth = static_cast<float>(rect.width());
    const float& imageHeight = static_cast<float>(rect.height());
    const float& tileWidth = static_cast<float>(TILESIZE.width());
    const float& tileHeight = static_cast<float>(TILESIZE.height());
    const int& nbTilesOnWidth = static_cast<int>(ceil(imageWidth / tileWidth));
    const int& nbTilesOnHeight = static_cast<int>(ceil(imageHeight / tileHeight));

    QList<QPoint> points;

    QList<QPoint> corners;
    const QPoint& cornerOffset = QPoint(TILESIZE.width(), TILESIZE.height());

    corners.append({rect.topLeft(), rect.topRight(), rect.bottomLeft(), rect.bottomRight()});
    for (int h=0; h < nbTilesOnHeight; h++) {
        for (int w=0; w < nbTilesOnWidth; w++) {

            const QPoint& tileIndex = QPoint(TILESIZE.width()*w,TILESIZE.height()*h);
            for (int i = 0; i < corners.count(); i++) {
                QPoint movedPos = getTileIndex(corners[i]-cornerOffset);
                movedPos = getTilePos(movedPos)+tileIndex;

                if (points.contains(movedPos)) {
                    continue;
                }

                if (getRectForPoint(movedPos).intersects(rect)) {
                    points.append(movedPos);
                }
            }
        }
    }
    return points;
}

QList<QPoint> BitmapSurface::touchedTiles(const QRect& rect)
{
    return scanForTilesAtSelection(rect);
}

void BitmapSurface::drawRect(QRect rect, QColor color)
{
    float rectWidth = static_cast<float>(rect.width());
    float rectHeight = static_cast<float>(rect.height());
    float tileWidth = static_cast<float>(TILESIZE.width());
    float tileHeight = static_cast<float>(TILESIZE.height());
    int nbTilesOnWidth = static_cast<int>(ceil(rectWidth / tileWidth));
    int nbTilesOnHeight = static_cast<int>(ceil(rectHeight / tileHeight));

    QPixmap paintTo(TILESIZE);
    mSurface = Surface();
    mSurface.bounds = QRect(rect.topLeft(), rect.size());

    for (int h=0; h < nbTilesOnHeight; h++) {
        for (int w=0; w < nbTilesOnWidth; w++) {
            paintTo.fill(Qt::transparent);
            const QPoint& idx = QPoint(w, h);
            const QPoint& tilePos = getTilePos(idx);

            const QRect& tileRect = QRect(tilePos, TILESIZE);
            QImage colorImage = QImage(rect.size(), QImage::Format_ARGB32_Premultiplied);
            colorImage.fill(color);
            const QImage& tileImage = colorImage.copy(tileRect);

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

    const QRect& intersection = selection.intersected(getRectForPoint(pos, pixmap.size()));
    painter.fillRect(intersection, color);
    painter.end();

}

Surface BitmapSurface::intersectedSurface(const QRect rect)
{
    Surface outSurface;
    for (int i = 0; i < mSurface.countTiles(); i++)
    {
        const QPixmap& pix = mSurface.pixmapAt(i);
        const QPoint& pos = mSurface.pointAt(i);

        if (getRectForPoint(pos, pix.size()).intersects(rect)) {
            outSurface.appendPixmap(pix);
            outSurface.appendPosition(pos);
            outSurface.bounds = rect;
        }
    }

    return outSurface;
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
        QFile f(filename);
        if(f.exists())
        {
            bool b = f.remove();
            return (b) ? Status::OK : Status::FAIL;
        }
        return Status::SAFE;
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

const QRect BitmapSurface::getRectForPoint(const QPoint& point, const QSize size)
{
    return QRect(point.x(), point.y(), size.width(), size.height());
}

const QRect BitmapSurface::getRectForPoint(const QPoint& point)
{
    return QRect(point.x(), point.y(), TILESIZE.width(), TILESIZE.height());
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


