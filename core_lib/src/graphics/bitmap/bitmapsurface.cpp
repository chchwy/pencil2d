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

void BitmapSurface::createPiecesFromImage(QString& path, QPoint& topLeft)
{
    QImage image(path);
    createPiecesFromImage(image, topLeft);
}

void BitmapSurface::addBitmapPiece(const QPixmap& pixmap, const QPoint& pos)
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

void BitmapSurface::createPiecesFromImage(QImage& image, QPoint& topLeft)
{
    int nbTilesOnWidth = ceil((float)image.width() / (float)TILESIZE.width());
    int nbTilesOnHeight = ceil((float)image.height() / (float)TILESIZE.height());

    QPixmap paintTo(TILESIZE);
    paintTo.fill(Qt::transparent);
    mPixmaps = QVector<std::shared_ptr< QPixmap >>();
    mBounds = QRect(topLeft, image.size());

    for (int h=0; h < nbTilesOnHeight; h++) {
        for (int w=0; w < nbTilesOnWidth; w++) {
            QPoint idx(w, h);
            QPoint tilePos = getTilePos(idx);

            QRect tileRect = QRect(tilePos, TILESIZE);
            QImage tileImage = getSubImageFromImage(image, tileRect);

            QPainter painter(&paintTo);
            painter.drawImage(QPoint(), tileImage);
            painter.end();

            mPixmaps.append(std::make_shared<QPixmap>(paintTo));
        }
    }
}

const QPixmap BitmapSurface::getPixmapAtIndex(const int& index)
{
    return *getPixmapAt(index);
}

QImage BitmapSurface::getSubImageFromImage(QImage& image, QRect& rect)
{
    size_t offset = rect.x() * image.depth() / 8 + rect.y() * image.bytesPerLine();
    return QImage(image.bits() + offset,
                  rect.width(),
                  rect.height(),
                  image.bytesPerLine(),
                  image.format());
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

//void BitmapSurface::setSurfaceFromFuture(int index)
//{
//    mCachedSurface = mImageAssembler->resultAt(index);
//}


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


