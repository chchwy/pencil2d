#ifndef BUFFERIMAGE_H
#define BUFFERIMAGE_H

#include "blitrect.h"
#include <QPainter>

class QImage;
class BitmapImage;


class BufferCanvas
{
public:
    BufferCanvas();
    ~BufferCanvas();

    void clear();
    QRect bounds();

    void drawLine(QPointF P1, QPointF P2, QPen pen, QPainter::CompositionMode cm, bool antialiasing);
    void drawPath(QPainterPath path, QPen pen, QBrush brush,
                               QPainter::CompositionMode cm, bool antialiasing);
    void drawEllipse(QRectF rectangle, QPen pen, QBrush brush, QPainter::CompositionMode cm, bool antialiasing);

    void paintImage(QPainter& painter);
    void paste(BitmapImage* bitmapImage, QPainter::CompositionMode cm = QPainter::CompositionMode_SourceOver);

private:
    QImage* mImage = nullptr;
    BlitRect mRect;
};

#endif // BUFFERIMAGE_H