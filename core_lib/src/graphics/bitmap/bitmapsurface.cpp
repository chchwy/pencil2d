#include "bitmapsurface.h"

#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QtMath>

BitmapSurface::BitmapSurface()
{
    for (std::shared_ptr<QPixmap> pix : mPixmaps) {
        pix = std::make_shared<QPixmap>(); // create null image
        pix->fill(Qt::transparent);

    }

    for (QRect rect : mBoundingRects) {
        rect = QRect(0, 0, 0, 0);
    }
}

BitmapSurface::BitmapSurface(BitmapSurface& pieces) :
    mPixmaps(pieces.mPixmaps),
    mBoundingRects(pieces.mBoundingRects),
    mCombinedSize(pieces.mCombinedSize)
{
}

const QRect BitmapSurface::getBoundingRectAtIndex(int index)
{
    return mBoundingRects.at(index);
}

const QPixmap BitmapSurface::getPixmapAtIndex(const int index)
{
    return *getPixmapAt(index);
}

const QSize BitmapSurface::size()
{
    return mCombinedSize;
}

void BitmapSurface::createPiecesFromImage(QImage& image)
{
    int nbTilesOnWidth = ceil((float)image.width() / (float)TILESIZE.width());
    int nbTilesOnHeight = ceil((float)image.height() / (float)TILESIZE.height());

    QPixmap paintTo(TILESIZE);
    paintTo.fill(Qt::transparent);
    mPixmaps = QVector<std::shared_ptr< QPixmap >>();
    mCombinedSize = image.size();
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
            mBoundingRects.append(tileRect);
        }
    }
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

QImage BitmapSurface::getSubImageFromImage(QImage& image, QRect rect)
{
    size_t offset = rect.x() * image.depth() / 8 + rect.y() * image.bytesPerLine();
    return QImage(image.bits() + offset,
                  rect.width(),
                  rect.height(),
                  image.bytesPerLine(),
                  image.format());
}

void BitmapSurface::addBitmapPiece(const QPixmap& pixmap, const QRect& rect)
{
    mPixmaps.append(std::make_shared<QPixmap>(pixmap));
    mBoundingRects.append(rect);
    mCombinedSize += rect.size();
}

void BitmapSurface::paintWholeImage(QString path)
{
    QPixmap paintedImage(mCombinedSize);
    paintedImage.fill(Qt::transparent);
    QPainter painter(&paintedImage);
    for (int i = 0; i < mPixmaps.count(); i++)
    {
        const QPixmap& pix = *mPixmaps.at(i);
        const QRect& rect = mBoundingRects.at(i);
        painter.drawPixmap(rect, pix);
    }
    painter.end();
    paintedImage.save(path);
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


