/* brushlib - The MyPaint Brush Library (demonstration project)
 * Copyright (C) 2013 POINTCARRE SARL / Sebastien Leon email: sleon at pointcarre.com
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "mptile.h"
#include "qdebug.h"

MPTile::MPTile(QGraphicsItem * parent) : QGraphicsItem(parent), m_cache_img(k_tile_dim, k_tile_dim, QImage::Format_ARGB32_Premultiplied)
{
    setCacheMode(QGraphicsItem::NoCache);
    clear(); //Default tiles are transparent
}

MPTile::MPTile(QPixmap& pixmap)
{
    m_cache_img = pixmap.toImage();
    setCacheMode(QGraphicsItem::NoCache);
}

MPTile::~MPTile()
{
}

QImage MPTile::image() 
{
    return m_cache_img;
}

QRectF MPTile::boundingRect() const 
{
    return m_cache_img.rect();
}

//bool MPTile::contains(const QPointF & point) const
//{
//    // opaque if alpha > 16
//    return qAlpha(m_cache_img.pixel(point.toPoint())) > 0x10;
//}

QPainterPath MPTile::shape() const
{
    QPainterPath path;
    path.addRect(m_cache_img.rect());
    return path;
}

void MPTile::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

//    qDebug() << "is painting tile";

    QBrush b = painter->brush();
//    painter->setTransform();
//    painter->setBrush(Qt::gray);

//    qDebug() << transform;
//    painter->
//    painter->setTransform(transform);
//    painter->drawRect(13, 13, 97, 57);
//    painter->drawEllipse(10,10,10,10);

//    painter->fillRect(boundingRect(), Qt::blue);

    if (!m_cache_valid) 
        updateCache(); // We need to transfer the uint16_t table to the QImage cache
    painter->drawImage(QPoint(), m_cache_img, m_cache_img.rect());
    painter->resetTransform();
}

uint16_t* MPTile::Bits(bool readOnly)
{
    // Correct C++ way of doing things is using "const" but MyPaint API is not compatible here
    m_cache_valid = readOnly ? m_cache_valid : false;
    return (uint16_t*)t_pixels;
}

// debug function (simply replace previous value of pixel in t_pixels)
//
void MPTile::drawPoint(uint x, uint y, uint16_t r, uint16_t g, uint16_t b, uint16_t a)
{
    m_cache_valid = false;
    t_pixels[y][x][k_red] = r;
    t_pixels[y][x][k_green] = g;
    t_pixels[y][x][k_blue ] = b;
    t_pixels[y][x][k_alpha] = a;
}


// Time to transfer the pixels from the uint16 to 32 bits cache before repaint...
//
void MPTile::updateCache()
{
    QRgb* dst = (QRgb*)m_cache_img.bits();
    for (int y = 0 ; y < k_tile_dim ; y++) {
         for (int x = 0 ; x < k_tile_dim ; x++) {
              uint16_t alpha = t_pixels[y][x][k_alpha];
              *dst = alpha ? qRgba(
              CONV_16_8(t_pixels[y][x][k_red]),
              CONV_16_8(t_pixels[y][x][k_green]),
              CONV_16_8(t_pixels[y][x][k_blue]),
              CONV_16_8(alpha)) : 0; // aplha is 0 => all is zero (little optimization)
              dst++; // next image pixel...
         }
    }
    m_cache_valid = true;
}

void MPTile::setImage(const QImage &image) {

    QSize tileSize = this->boundingRect().size().toSize();

    // Make sure the image has the same dimentions as the tile
    m_cache_img = image.scaled(tileSize, Qt::IgnoreAspectRatio);

    for (int y = 0 ; y < tileSize.height() ; y++) {
         for (int x = 0 ; x < tileSize.width() ; x++) {

             QRgb pixelColor = m_cache_img.pixel(x, y);

             t_pixels[y][x][k_alpha]    = CONV_8_16(qAlpha(pixelColor));
             t_pixels[y][x][k_red]      = CONV_8_16(qRed(pixelColor));
             t_pixels[y][x][k_green]    = CONV_8_16(qGreen(pixelColor));
             t_pixels[y][x][k_blue]     = CONV_8_16(qBlue(pixelColor));

         }
    }
    m_cache_valid = true;
}

void MPTile::clear()
{
    memset(t_pixels, 0, sizeof(t_pixels)); // Tile is transparent
    m_cache_img.fill( QColor(Qt::transparent) ); // image cache is transparent too, and aligned to the pixel table:
    m_cache_valid = true;
    m_dirty = false;
}
