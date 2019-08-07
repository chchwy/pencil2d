#ifndef BITMAPUTILS_H
#define BITMAPUTILS_H

#include "bitmapimage.h"
#include "bitmapsurface.h"
#include "qmath.h"

struct BitmapUtils {

static QRgb constScanLine(QImage& inImage, QRect bounds, QPoint topLeft, int x, int y)
{
    QRgb result = qRgba(0, 0, 0, 0);
    if (bounds.contains(QPoint(x,y))) {
        result = *(reinterpret_cast<const QRgb*>(inImage.constScanLine(y - topLeft.y())) + x - topLeft.x() );
    }
    return result;
}

static void scanLine(QImage* outImage, QRect bounds, QPoint offset, const int& x, const int& y, const QRgb& colour)
{
        // Make sure color is premultiplied before calling

    if (bounds.contains(QPoint(x,y))) {
        QRgb toColor = qRgba(qRed(colour),qGreen(colour), qBlue(colour), qAlpha(colour));
        QRgb* sourceColor = (reinterpret_cast<QRgb*>(outImage->scanLine(y - offset.y()))+x-offset.x());
        *sourceColor = toColor;
    }
}


static QRgb pixel(const QImage& sourceImage, QPoint topLeft, int x, int y)
{
    return pixel(sourceImage, topLeft, QPoint(x, y));
}

static QRgb pixel(const QImage& sourceImage, QPoint topLeft,  const QPoint& pos)
{
    return sourceImage.pixel(pos-topLeft);
}

static void setPixel(QImage& outImage, QPoint offset, const int& x, const int& y, const QRgb& colour)
{
    outImage.setPixel(QPoint(x, y)-offset, colour);
}

/** Compare colors for the purposes of flood filling
 *
 *  Calculates the Eulcidian difference of the RGB channels
 *  of the image and compares it to the tolerance
 *
 *  @param[in] newColor The first color to compare
 *  @param[in] oldColor The second color to compare
 *  @param[in] tolerance The threshold limit between a matching and non-matching color
 *  @param[in,out] cache Contains a mapping of previous results of compareColor with rule that
 *                 cache[someColor] = compareColor(someColor, oldColor, tolerance)
 *
 *  @return Returns true if the colors have a similarity below the tolerance level
 *          (i.e. if Eulcidian distance squared is <= tolerance)
 */
static bool compareColor(const QRgb& newColor, const QRgb& oldColor, const int& tolerance, QHash<QRgb, bool> *cache)
{
    // Handle trivial case
    if (newColor == oldColor) return true;

    if(cache && cache->contains(newColor)) return cache->value(newColor);

    // Get Eulcidian distance between colors
    // Not an accurate representation of human perception,
    // but it's the best any image editing program ever does
    int diffRed = static_cast<int>(qPow(qRed(oldColor) - qRed(newColor), 2));
    int diffGreen = static_cast<int>(qPow(qGreen(oldColor) - qGreen(newColor), 2));
    int diffBlue = static_cast<int>(qPow(qBlue(oldColor) - qBlue(newColor), 2));
    // This may not be the best way to handle alpha since the other channels become less relevant as
    // the alpha is reduces (ex. QColor(0,0,0,0) is the same as QColor(255,255,255,0))
    int diffAlpha = static_cast<int>(qPow(qAlpha(oldColor) - qAlpha(newColor), 2));

    bool isSimilar = (diffRed + diffGreen + diffBlue + diffAlpha) <= tolerance;

    if(cache)
    {
        Q_ASSERT(cache->contains(isSimilar) ? isSimilar == (*cache)[newColor] : true);
        (*cache)[newColor] = isSimilar;
    }

    return isSimilar;
}

// Flood fill
// ----- http://lodev.org/cgtutor/floodfill.html
static void floodFill(QImage& targetImage,
                      const QRect& bounds,
                      QPoint point,
                      const QRgb& newColor,
                      int tolerance)
{
    QPoint offset = bounds.topLeft();
    point = QPoint(point.x(), point.y());

    // Square tolerance for use with compareColor
    tolerance = static_cast<int>(qPow(tolerance, 2));

    QRgb oldColor = pixel(targetImage, offset, point);
    oldColor = qRgba(qRed(oldColor), qGreen(oldColor), qBlue(oldColor), qAlpha(oldColor));

    // Preparations
    QList<QPoint> queue; // queue all the pixels of the filled area (as they are found)

    QImage replaceImage = QImage(bounds.size(), QImage::Format_ARGB32_Premultiplied);
    replaceImage.fill(Qt::transparent);
    QPoint tempPoint;
    QRgb newPlacedColor = 0;
    QScopedPointer< QHash<QRgb, bool> > cache(new QHash<QRgb, bool>());

    int xTemp = 0;
    bool spanLeft = false;
    bool spanRight = false;

    queue.append(point);
    // Preparations END

    while (!queue.empty())
    {
        tempPoint = queue.takeFirst();

        point.setX(tempPoint.x());
        point.setY(tempPoint.y());

        xTemp = point.x();

        newPlacedColor = constScanLine(replaceImage, bounds, offset, xTemp, point.y());
        while (xTemp >= bounds.left() &&
               compareColor(constScanLine(targetImage, bounds, offset, xTemp, point.y()), oldColor, tolerance, cache.data())) xTemp--;
        xTemp++;

        spanLeft = spanRight = false;
        while (xTemp <= bounds.right() &&
               compareColor(constScanLine(targetImage, bounds, offset, xTemp, point.y()), oldColor, tolerance, cache.data()) &&
               newPlacedColor != newColor)
        {

            scanLine(&replaceImage, bounds, offset, xTemp, point.y(), newColor);

            if (!spanLeft && (point.y() > bounds.top()) &&
                compareColor(constScanLine(targetImage, bounds, offset, xTemp, point.y() - 1), oldColor, tolerance, cache.data())) {
                queue.append(QPoint(xTemp, point.y() - 1));
                spanLeft = true;
            }
            else if (spanLeft && (point.y() > bounds.top()) &&
                     !compareColor(constScanLine(targetImage, bounds, offset, xTemp, point.y() - 1), oldColor, tolerance, cache.data())) {
                spanLeft = false;
            }

            if (!spanRight && point.y() < bounds.bottom() &&
                compareColor(constScanLine(targetImage, bounds, offset, xTemp, point.y() + 1), oldColor, tolerance, cache.data())) {
                queue.append(QPoint(xTemp, point.y() + 1));
                spanRight = true;

            }
            else if (spanRight && point.y() < bounds.bottom() &&
                     !compareColor(constScanLine(targetImage, bounds, offset, xTemp, point.y() + 1), oldColor, tolerance, cache.data())) {
                spanRight = false;
            }

//            if (targetImage.rect() != bounds) {
//                targetImage.extend(QPoint(x, y));
//            }
            Q_ASSERT(queue.count() < (targetImage.width() * targetImage.height()));
            xTemp++;
        }
    }

    QPainter painter(&targetImage);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(QPoint(), replaceImage);
    painter.end();
}

};

#endif // BITMAPUTILS_H
