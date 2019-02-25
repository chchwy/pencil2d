#include "buffercanvas.h"
#include <QImage>


BufferCanvas::BufferCanvas()
{
    mImage = new QImage(1920, 1080, QImage::Format_ARGB32_Premultiplied);
}

BufferCanvas::~BufferCanvas()
{
    delete mImage;
}

void BufferCanvas::clear()
{

}

QRect BufferCanvas::bounds()
{
    return QRect(mRect.topLeft(), mRect.bottomRight());
}

void BufferCanvas::drawLine(QPointF P1, QPointF P2, QPen pen, QPainter::CompositionMode cm, bool antialiasing)
{
}

void BufferCanvas::drawPath(QPainterPath path, QPen pen, QBrush brush, QPainter::CompositionMode cm, bool antialiasing)
{

}

void BufferCanvas::drawEllipse(QRectF rectangle, QPen pen, QBrush brush, QPainter::CompositionMode cm, bool antialiasing)
{

}

void BufferCanvas::paintImage(QPainter& painter)
{

}

void BufferCanvas::paste(BitmapImage* bitmapImage, QPainter::CompositionMode cm)
{

}
