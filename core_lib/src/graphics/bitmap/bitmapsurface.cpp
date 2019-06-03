#include "bitmapsurface.h"

#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QtMath>
#include <QtConcurrent>

BitmapSurface::BitmapSurface()
{
    for (std::shared_ptr<QPixmap> pix : mPixmaps) {
        pix = std::make_shared<QPixmap>(); // create null image
        pix->fill(Qt::transparent);
    }

    mBounds = QRect(0,0,0,0);
}

BitmapSurface::BitmapSurface(BitmapSurface& pieces) : KeyFrame (pieces),
    mPixmaps(pieces.mPixmaps),
    mBounds(pieces.mBounds)
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
    return (!mPixmaps.isEmpty());
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

void BitmapSurface::createSurfaceFromImage(QString& path, QPoint& topLeft)
{
    QImage image(path);
    appendToSurfaceFromImage(image, topLeft);
}

void BitmapSurface::appendBitmapSurface(const QPixmap& pixmap, const QPoint& pos)
{
    mPixmaps.append(std::make_shared<QPixmap>(pixmap));
    mTilePositions.append(pos);

    extendBoundaries(QRect(pos, pixmap.size()));
}

void BitmapSurface::renderSurfaceImage()
{
    mCachedSurface = surfaceAsImage();
}

void BitmapSurface::extendBoundaries(QRect rect)
{
    if (mBounds.left() > rect.left()) { mBounds.setLeft(rect.left()); }
    if (mBounds.right() < rect.right()) { mBounds.setRight(rect.right()); }
    if (mBounds.top() > rect.top()) { mBounds.setTop(rect.top()); }
    if (mBounds.bottom() < rect.bottom()) { mBounds.setBottom(rect.bottom()); }
}

void BitmapSurface::appendToSurfaceFromImage(QImage& image, QPoint& topLeft)
{
    float imageWidth = static_cast<float>(image.width());
    float imageHeight = static_cast<float>(image.height());
    float tileWidth = static_cast<float>(TILESIZE.width());
    float tileHeight = static_cast<float>(TILESIZE.height());
    int nbTilesOnWidth = static_cast<int>(ceil(imageWidth / tileWidth));
    int nbTilesOnHeight = static_cast<int>(ceil(imageHeight / tileHeight));

    QPixmap paintTo(TILESIZE);
    mPixmaps = QVector<std::shared_ptr< QPixmap >>();
    mBounds = QRect(topLeft, image.size());

    for (int h=0; h < nbTilesOnHeight; h++) {
        for (int w=0; w < nbTilesOnWidth; w++) {
            paintTo.fill(Qt::transparent);
            QPoint idx(w, h);
            QPoint tilePos = getTilePos(idx);

            QRect tileRect = QRect(tilePos, TILESIZE);
            QImage tileImage = image.copy(tileRect);

            QPainter painter(&paintTo);
            painter.drawImage(QPoint(), tileImage);
            painter.end();

            mPixmaps.append(std::make_shared<QPixmap>(paintTo));
        }
    }
}

QFuture<QImage> BitmapSurface::surfaceAsImage()
{
    auto asyncPaint = [this]() {
        QImage paintedImage(mBounds.size(), QImage::Format_ARGB32_Premultiplied);
        paintedImage.fill(Qt::transparent);

        QPainter painter(&paintedImage);
        painter.translate(-mBounds.topLeft());

        for (int i = 0; i < mPixmaps.count(); i++)
        {
            const QPixmap pix = *mPixmaps.at(i);
            const QPoint pos = mTilePositions.at(i);
            painter.drawPixmap(pos, pix);
        }
        painter.end();

        return paintedImage;
    };
    return QtConcurrent::run(asyncPaint);
}

void BitmapSurface::eraseSelection(const QPoint pos, QPixmap& pixmap, const QRect selection)
{
    fillSelection(pos, pixmap, Qt::transparent, selection);
}

void BitmapSurface::eraseSelection(const QRect selection)
{
    for (int i = 0; i < mPixmaps.count(); i++) {

        QPixmap& pixmap = *mPixmaps.at(i);
        const QPoint pos = mTilePositions.at(i);
        eraseSelection(pos, pixmap, selection);
    }
}

void BitmapSurface::fillSelection(const QPoint pos, QPixmap& pixmap, QColor color, const QRect selection)
{
    QPainter painter(&pixmap);
    painter.translate(-pos);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    const QPixmap copiedPix = pixmap;
    const QRect intersection = selection.intersected(getBoundingRectAtIndex(pos, copiedPix.size()));
    painter.fillRect(intersection, color);
    painter.end();

}

Surface BitmapSurface::intersectedSurface(const QRect rect)
{
    QList<QPixmap> selectionImages;
    QList<QPoint> selectionPos;
    for (int i = 0; i < mPixmaps.count(); i++)
    {
        QPixmap& pix = *mPixmaps.at(i);
        const QPoint pos = mTilePositions.at(i);

        if (rect.intersects(getBoundingRectAtIndex(pos))) {
            selectionImages.append(pix);
            selectionPos.append(pos);
        }
    }

    return Surface(selectionPos, selectionImages);
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
        const QPixmap pix = intersectSurface.pixmapAt(i);
        const QPoint pos = intersectSurface.pointAt(i);
        painter.drawPixmap(pos, pix);
    }
    return paintedImage;
}


QPixmap BitmapSurface::copySurfaceAsPixmap(const QRect selection)
{
    Q_ASSERT(!selection.isEmpty());

    Surface intersectSurface = intersectedSurface(selection);

    QPixmap paintedImage(selection.size());
    paintedImage.fill(Qt::transparent);

    QPainter painter(&paintedImage);
    painter.translate(-selection.topLeft());
    for (int i = 0; i < intersectSurface.countTiles(); i++)
    {
        const QPixmap pix = intersectSurface.pixmapAt(i);
        const QPoint pos = intersectSurface.pointAt(i);
        painter.drawPixmap(pos, pix);
    }
    return paintedImage;
}

QVector<QPoint> BitmapSurface::tilePositions()
{
    if (mTilePositions.isEmpty()) { return QVector<QPoint>(); }
    return mTilePositions;
}

QVector<std::shared_ptr< QPixmap >> BitmapSurface::pixmaps()
{
    if (mPixmaps.isEmpty()) { return QVector<std::shared_ptr< QPixmap >>(); }
    return mPixmaps;
}

void BitmapSurface::clear()
{
    mBounds = QRect();
    mCachedSurface = QImage();
    mPixmaps.clear();
    mTilePositions.clear();
}

Status BitmapSurface::writeFile(const QString& filename)
{
    if (mPixmaps.isEmpty()) {
        return Status::FAIL;
    }

    if (mPixmaps.first()) {
        bool b = mCachedSurface.save(filename);
        return (b) ? Status::OK : Status::FAIL;
    }
    return Status::FAIL;
}

const QPixmap BitmapSurface::getPixmapFromTilePos(const QPoint& pos)
{
    for (int i = 0; i < mPixmaps.count(); i++) {
        const QPoint& tilePos = mTilePositions.at(i);

        if (tilePos == pos) {
            return *pixmaps().at(i).get();
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


