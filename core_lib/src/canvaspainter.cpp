/*

Pencil - Traditional Animation Software
Copyright (C) 2012-2018 Matthew Chiawen Chang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/

#include "canvaspainter.h"

#include "object.h"
#include "mptile.h"
#include "layerbitmapsurface.h"
#include "layerbitmap.h"
#include "layervector.h"
#include "bitmapsurface.h"
#include "bitmapimage.h"
#include "layercamera.h"
#include "vectorimage.h"
#include "util.h"



// TODO: rendering not working
// keep looking through feeeef's git log..
// something must be missing -.-
CanvasPainter::CanvasPainter(QObject* parent) : QObject(parent)
, mLog("CanvasRenderer")
{
    ENABLE_DEBUG_LOG(mLog, false);
}

CanvasPainter::~CanvasPainter()
{
}

void CanvasPainter::setCanvas(QPixmap* canvas)
{
    Q_ASSERT(canvas);
    mCanvas = canvas;
}

void CanvasPainter::setViewTransform(const QTransform view)
{
    mViewTransform = view;
}

void CanvasPainter::setTransformedSelection(QRect selection, QRect movingSelection, QTransform transform)
{
    // Make sure that the selection is not empty
    if (selection.width() > 0 && selection.height() > 0)
    {
        mSelection = selection;
        mMovingSelection = movingSelection;
        mSelectionTransform = transform;
        mRenderTransform = true;
    }
    else
    {
        // Otherwise we shouldn't be in transformation mode
        ignoreTransformedSelection();
    }
}

void CanvasPainter::ignoreTransformedSelection()
{
    mRenderTransform = false;
}


void CanvasPainter::initPaint(const Object *object, int layer, int frame, QPainter& painter)
{
    Q_ASSERT( object );
    mObject = object;

    mCurrentLayerIndex = layer;
    mFrameNumber = frame;

    // Clear Canvas
    mCanvas->fill( Qt::transparent );

//    if (!quick) {
//        painter.setRenderHint( QPainter::SmoothPixmapTransform, mOptions.bAntiAlias );
//        painter.setRenderHint( QPainter::Antialiasing, true );
//    }
//    else {
//        painter.setRenderHint( QPainter::SmoothPixmapTransform, false );
//        painter.setRenderHint( QPainter::Antialiasing, false );
//    }

//    painter.setWorldMatrixEnabled( true );
}

void CanvasPainter::paint(QPainter& painter, const Object* object, int layerIndex, int frameIndex, QList<MPTile*> tilesToBeRendered)
{

    // Paint Canvas
    initPaint(object, layerIndex, frameIndex, painter);
    mTilesToBeRendered = tilesToBeRendered;

    paintCurrentFrame( painter, RENDER_LEVEL::ALL );
    paintCameraBorder( painter );

    // post effects
    paintPostEffects(painter);
    painter.end();
}

void CanvasPainter::paintPostEffects(QPainter& painter)
{
    if ( mOptions.axisEnabled )
    {
        paintAxis( painter );
    }

    if ( mOptions.gridEnabld )
    {
        paintGrid( painter );
    }
}

void CanvasPainter::paintOnionSkin(QPainter& painter)
{
    if (!mOptions.onionWhilePlayback && mOptions.isPlaying) { return; }

    Layer* layer = mObject->getLayer(mCurrentLayerIndex);

    if (layer->visible() == false)
        return;

    if (layer->keyFrameCount() == 0)
        return;

    qreal minOpacity = static_cast<qreal>(mOptions.onionSkinMinOpacity / 100);
    qreal maxOpacity = static_cast<qreal>(mOptions.onionSkinMaxOpacity / 100);

    if (mOptions.prevOnionSkinEnabled && mFrameNumber > 1)
    {
        // Paint onion skin before current frame.
        qreal prevOpacityIncrement = (maxOpacity - minOpacity) / mOptions.prevOnionSkinCount;
        qreal opacity = maxOpacity;

        int onionFrameNumber = layer->getPreviousFrameNumber(mFrameNumber, mOptions.onionSkinAbsoluteEnabled);
        int onionPosition = 0;

        while (onionPosition < mOptions.prevOnionSkinCount && onionFrameNumber > 0)
        {
            painter.setOpacity(opacity);

            switch (layer->type())
            {
            case Layer::BITMAP: { paintBitmapFrame(painter, layer, onionFrameNumber, mOptions.nextColoredOnionSkinEnabled, false); break; }
            case Layer::VECTOR: { paintVectorFrame(painter, layer, onionFrameNumber, mOptions.nextColoredOnionSkinEnabled, false); break; }
            default: break;
            }
            opacity = opacity - prevOpacityIncrement;

            onionFrameNumber = layer->getPreviousFrameNumber(onionFrameNumber, mOptions.onionSkinAbsoluteEnabled);
            onionPosition++;
        }
    }

    if (mOptions.nextOnionSkinEnabled)
    {
        // Paint onion skin after current frame.
        qreal nextOpacityIncrement = (maxOpacity - minOpacity) / mOptions.nextOnionSkinCount;
        qreal opacity = maxOpacity;

        int onionFrameNumber = layer->getNextFrameNumber(mFrameNumber, mOptions.onionSkinAbsoluteEnabled);
        int onionPosition = 0;

        while (onionPosition < mOptions.nextOnionSkinCount && onionFrameNumber > 0)
        {
            painter.setOpacity(opacity);

            switch (layer->type())
            {
            case Layer::BITMAP: { paintBitmapFrame(painter, layer, onionFrameNumber, mOptions.nextColoredOnionSkinEnabled, false); break; }
            case Layer::VECTOR: { paintVectorFrame(painter, layer, onionFrameNumber, mOptions.nextColoredOnionSkinEnabled, false); break; }
            default: break;
            }
            opacity = opacity - nextOpacityIncrement;

            onionFrameNumber = layer->getNextFrameNumber(onionFrameNumber, mOptions.onionSkinAbsoluteEnabled);
            onionPosition++;
        }
    }
}

void CanvasPainter::paintBitmapFrame(QPainter& painter, Layer* layer, int frameIndex, bool colorizeOnionSkin)
{

//    LayerBitmapSurface* bitmapLayer = static_cast<LayerBitmapSurface*>(layer);
//    BitmapSurface* bitmapSurface = nullptr;
//    if (useLastKeyFrame)
//    {
//        bitmapSurface = bitmapLayer->getLastBitmapImageAtFrame(nFrame, 0);
//    }
//    else
//    {
//    bitmapSurface = bitmapLayer->getBitmapImageAtFrame(frameIndex);
//    }

    if (colorizeOnionSkin) {
        paintColoredOnionSkin(painter, frameIndex);
    }


    QTransform v = mViewTransform;
    for (MPTile* item : mTilesToBeRendered) {

        QRectF tileRect = QRectF(item->pos(),
                               QSizeF(item->boundingRect().width(),
                                     item->boundingRect().height()));
        tileRect = v.mapRect(tileRect);

        QImage image = item->image();

        prescaleSurface(painter, image, tileRect);

        if (isRectInsideCanvas(tileRect)) {
            painter.drawImage(tileRect.toRect(), image);
        }
    }

    if (mRenderTransform) {
        paintTransformedSelection(painter);
    }

    // If the current frame on the current layer has a transformation, we apply it.
//    if (mRenderTransform && frameIndex == mFrameNumber && layer == mObject->getLayer(mCurrentLayerIndex))
//    {
//        paintTransformedSelection(painter);
//    }

//    painter.drawPixmap()

//    painter.setWorldMatrixEnabled(true);
//    painter.drawRect(paintToImage.bounds());
//    qDebug() << painter.transform();
//    paintToImage.setBounds(QRect(mCanvas->rect().topLeft(), paintToImage.size()));
//    paintToImage.paintImage(painter);
//    paintToImage.paintImage()
}

bool CanvasPainter::isRectInsideCanvas(const QRectF& rect) const
{
    return mCanvas->rect().adjusted(-rect.width(),
                                    -rect.width(),
                                    rect.width(),
                                    rect.width()).contains(rect.toRect());
}

void CanvasPainter::prescaleSurface(QPainter& painter, QImage& image, const QRectF& rect)
{
    if (mOptions.zoomLevel < 1.5f) { // 150%
        painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);

        // Only prescale when the content is small enough, as it's performance intensive to upscale larger images.
        if (mOptions.zoomLevel < 0.5f) { // 50%
        image = image.scaled(rect.size().toSize(),
                                             Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }
}

void CanvasPainter::paintColoredOnionSkin(QPainter& painter, const int frameIndex)
{
    QBrush colorBrush = QBrush(Qt::transparent); //no color for the current frame

    if (frameIndex < mFrameNumber)
    {
        colorBrush = QBrush(Qt::red);
    }
    else if (frameIndex > mFrameNumber)
    {
        colorBrush = QBrush(Qt::blue);
    }

//    paintToImage.drawRect(paintedImage->bounds(),
//                          Qt::NoPen,
//                          colorBrush,
//                          QPainter::CompositionMode_SourceIn,
//                          false);
//    painter.drawRect(paintedImage->bounds(),
//                     Qt::NoPen,
//                     colorBrush,
//                     QPainter::CompositionMode_SourceIn,
//                     false);
}

void CanvasPainter::paintBitmapFrame(QPainter& painter,
                                     Layer* layer,
                                     int nFrame,
                                     bool colorize,
                                     bool useLastKeyFrame)
{
#ifdef _DEBUG
    LayerBitmap* bitmapLayer = dynamic_cast<LayerBitmap*>(layer);
    if (bitmapLayer == nullptr)
    {
        Q_ASSERT(bitmapLayer);
        return;
    }
#else
    LayerBitmap* bitmapLayer = static_cast<LayerBitmap*>(layer);
#endif

    //qCDebug(mLog) << "Paint Onion skin bitmap, Frame = " << nFrame;
    BitmapImage* paintedImage = nullptr;
    if (useLastKeyFrame)
    {
        paintedImage = bitmapLayer->getLastBitmapImageAtFrame(nFrame, 0);
    }
    else
    {
        paintedImage = bitmapLayer->getBitmapImageAtFrame(nFrame);
    }

    if (paintedImage == nullptr || paintedImage->bounds().isEmpty())
    {
        return;
    }

//    QImage* pImage = new QImage( mCanvas->size(), QImage::Format_ARGB32_Premultiplied );

    paintedImage->loadFile(); // Critical! force the BitmapImage to load the image
    //qCDebug(mLog) << "Paint Image Size:" << paintedImage->image()->size();

    BitmapImage paintToImage;
    paintToImage.paste(paintedImage);

    if (colorize)
    {
//        paintColoredOnionSkin(painter);
    }

    // If the current frame on the current layer has a transformation, we apply it.
    if (mRenderTransform && nFrame == mFrameNumber && layer == mObject->getLayer(mCurrentLayerIndex))
    {
        paintToImage.clear(mSelection);
        paintTransformedSelection(painter);
    }

    qDebug() << paintToImage.bounds();
    painter.setWorldMatrixEnabled(true);
//    painter.drawRect(paintToImage.bounds());
//    qDebug() << painter.transform();
//    paintToImage.setBounds(QRect(mCanvas->rect().topLeft(), paintToImage.size()));
    paintToImage.paintImage(painter);
//    paintToImage.paintImage()
}


void CanvasPainter::prescale(BitmapImage* bitmapImage)
{
    QImage origImage = bitmapImage->image()->copy();

    // copy content of our unmodified qimage
    // to our (not yet) scaled bitmap
    mScaledBitmap = origImage.copy();

    if (mOptions.zoomLevel >= 1.0f)
    {
        // TODO: Qt doesn't handle huge upscaled qimages well...
        // possible solution, myPaintLib canvas renderer splits its canvas up in chunks.
    }
    else
    {
        // map to correct matrix
        QRectF mappedOrigImage = mViewTransform.mapRect(QRectF(origImage.rect()));
        mScaledBitmap = mScaledBitmap.scaled(mappedOrigImage.size().toSize(),
                                             Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
}

void CanvasPainter::paintVectorFrame(QPainter& painter,
                                     Layer* layer,
                                     int nFrame,
                                     bool colorize,
                                     bool useLastKeyFrame)
{
#ifdef _DEBUG
    LayerVector* vectorLayer = dynamic_cast<LayerVector*>(layer);
    if (vectorLayer == nullptr)
    {
        Q_ASSERT(vectorLayer);
        return;
    }
#else
    LayerVector* vectorLayer = static_cast<LayerVector*>(layer);
#endif

    qCDebug(mLog) << "Paint Onion skin vector, Frame = " << nFrame;
    VectorImage* vectorImage = nullptr;
    if (useLastKeyFrame)
    {
        vectorImage = vectorLayer->getLastVectorImageAtFrame(nFrame, 0);
    }
    else
    {
        vectorImage = vectorLayer->getVectorImageAtFrame(nFrame);
    }
    if (vectorImage == nullptr)
    {
        return;
    }

    QImage* pImage = new QImage(mCanvas->size(), QImage::Format_ARGB32_Premultiplied);
    vectorImage->outputImage(pImage,
                             mViewTransform,
                             mOptions.vectorOptions.outlineEnabled,
                             mOptions.vectorOptions.thinLinesEnabled,
                             mOptions.antiAliasingEnabled);

    //painter.drawImage( QPoint( 0, 0 ), *pImage );
    // Go through a Bitmap image to paint the onion skin colour
    BitmapImage tempBitmapImage;
    tempBitmapImage.setImage(pImage);

    if (colorize)
    {
        QBrush colorBrush = QBrush(Qt::transparent); //no color for the current frame

        if (nFrame < mFrameNumber)
        {
            colorBrush = QBrush(Qt::red);
        }
        else if (nFrame > mFrameNumber)
        {
            colorBrush = QBrush(Qt::blue);
        }
        tempBitmapImage.drawRect(pImage->rect(),
                                 Qt::NoPen, colorBrush,
                                 QPainter::CompositionMode_SourceIn, false);
    }

    painter.setWorldMatrixEnabled(false); // Don't transform the image here as we used the viewTransform in the image output
    tempBitmapImage.paintImage(painter);
}

void CanvasPainter::paintTransformedSelection(QPainter& painter)
{
    // Make sure there is something selected
    if (mSelection.width() == 0 || mSelection.height() == 0)
        return;

    Layer* layer = mObject->getLayer(mCurrentLayerIndex);

    if (layer->type() == Layer::BITMAP)
    {
        // Get the transformed image
        BitmapSurface* bitmapSurface = dynamic_cast<LayerBitmapSurface*>(layer)->getLastBitmapImageAtFrame(mFrameNumber, 0);
        QPixmap map = bitmapSurface->copySurfaceAsPixmap(mSelection);

        QRect selection = mViewTransform.mapRect(mSelection);
        QRect movingSelection = mViewTransform.mapRect(mMovingSelection);

        painter.save();

        // Fill the region where the selection started with white
        // to make it look like the surface has been modified
        painter.fillRect(selection, QColor(255,255,255,255));

        // Draw the selection image separately and on top
        painter.drawPixmap(movingSelection, map);
        painter.restore();
    }
}

void CanvasPainter::paintCurrentFrame(QPainter& painter, RENDER_LEVEL renderLevel)
{
    //bool isCamera = mObject->getLayer(mCurrentLayerIndex)->type() == Layer::CAMERA;
    painter.setOpacity(1.0);

    int layerIndex = mCurrentLayerIndex;
    for (int i = 0; i < mObject->getLayerCount(); ++i)
    {

        if (renderLevel == RENDER_LEVEL::CACHED) {
            paintCachedFrameAtLayer(painter, i);
        } else {
            paintCurrentFrameAtLayer(painter, i);
        }
//        if ( renderLevel == RENDER_LEVEL::ALL ||
//             ((renderLevel == RENDER_LEVEL::BACK_ONLY) && (i < layerIndex)) ||
//             ((renderLevel == RENDER_LEVEL::CURRENT_LAYER_ONLY) && (i == layerIndex)) ||
//             ((renderLevel == RENDER_LEVEL::TOP_ONLY) && (i > layerIndex)) )
//        {
//            paintCurrentFrameAtLayer(painter, i);
//        }
    }
}

void CanvasPainter::paintCurrentFrameAtLayer(QPainter& painter, int layerIndex)
{
    Layer* layer = mObject->getLayer(layerIndex);

//    if (layer->visible() == false)
//        continue;

    if (layerIndex == mCurrentLayerIndex || mOptions.showLayersCount > 0)
    {
        switch (layer->type())
        {
        case Layer::BITMAP: { paintBitmapFrame(painter, layer, mFrameNumber, false); break; }
//        case Layer::VECTOR: { paintVectorFrame(painter, layer, mFrameNumber, false); break; }
        default: break;
        }
    }
}

void CanvasPainter::paintCachedFrameAtLayer(QPainter& painter, int layerIndex)
{
    Layer* layer = mObject->getLayer(layerIndex);

    if (layerIndex == mCurrentLayerIndex || mOptions.showLayersCount > 0)
    {
        switch (layer->type())
        {
        case Layer::BITMAP: { paintBitmapFrame(painter, layer, mFrameNumber, false, true); break; }
        case Layer::VECTOR: { paintVectorFrame(painter, layer, mFrameNumber, false, true); break; }
        default: break;
        }
    }
}

void CanvasPainter::paintAxis(QPainter& painter)
{
    painter.setPen(Qt::green);
    painter.drawLine(QLineF(0, -500, 0, 500));

    painter.setPen(Qt::red);
    painter.drawLine(QLineF(-500, 0, 500, 0));
}

int round100(double f, int gridSize)
{
    qDebug() << f << " " << gridSize;
    return static_cast<int>(f) / gridSize * gridSize;
}

void CanvasPainter::paintGrid(QPainter& painter)
{
    painter.save();
    int gridSizeW = mOptions.gridSizeW;
    int gridSizeH = mOptions.gridSizeH;

    QRectF rect = mCanvas->rect();
    painter.setWorldTransform(mViewTransform);
    QRectF boundingRect = mViewTransform.inverted().mapRect(rect);

    int left = round100(boundingRect.left(), gridSizeW) - gridSizeW;
    int top = round100(boundingRect.top(), gridSizeH) - gridSizeH;
    int bottom = round100(boundingRect.bottom(), gridSizeH) + gridSizeH;
    int right = round100(boundingRect.right(), gridSizeW) + gridSizeW;

    QPen pen(Qt::lightGray);

    pen.setCosmetic(true);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.setRenderHint(QPainter::Antialiasing, false);
    // draw vertical gridlines
    for (int x = left; x < right; x += gridSizeW)
    {
        painter.drawLine(x, top, x, bottom);
    }

    // draw horizontal gridlines
    for (int y = top; y < bottom; y += gridSizeH)
    {
        painter.drawLine(left, y, right, y);
    }
    painter.restore();
}

void CanvasPainter::renderGrid(QPainter& painter)
{
    if (mOptions.gridEnabld)
    {
        painter.setWorldTransform(mViewTransform);
        paintGrid(painter);
    }
}

void CanvasPainter::paintCameraBorder(QPainter &painter)
{
    painter.save();
    LayerCamera* cameraLayer = nullptr;
    bool isCameraMode = false;

    // Find the first visiable camera layers
    for (int i = 0; i < mObject->getLayerCount(); ++i)
    {
        Layer* layer = mObject->getLayer(i);
        if (layer->type() == Layer::CAMERA && layer->visible())
        {
            cameraLayer = static_cast<LayerCamera*>(layer);
            isCameraMode = (i == mCurrentLayerIndex);
            break;
        }
    }

    if (cameraLayer == nullptr) { return; }

    QRectF viewRect = painter.viewport();
    QRect boundingRect;
    mCameraRect = cameraLayer->getViewRect();

    if (isCameraMode)
    {
        QTransform center = QTransform::fromTranslate(viewRect.width() / 2.0, viewRect.height() / 2.0);
        boundingRect = viewRect.toAlignedRect();
        mCameraRect = center.mapRect(mCameraRect);
    }
    else
    {
        painter.setWorldTransform(mViewTransform);
        painter.setWorldMatrixEnabled(true);
        QTransform viewInverse = mViewTransform.inverted();
        boundingRect = viewInverse.mapRect(viewRect.toRect());

        QTransform camTransform = cameraLayer->getViewAtFrame(mFrameNumber);
        mCameraRect = camTransform.inverted().mapRect(mCameraRect);
    }

    painter.setOpacity(1.0);
    painter.setBrush(QColor(0, 0, 0, 80));

    QRegion rg1(boundingRect);
    QRegion rg2(mCameraRect);
    QRegion rg3 = rg1.subtracted(rg2);
    painter.setClipRegion(rg3);
    painter.drawRect(boundingRect);

    paintCameraOutline(painter);

    painter.restore();
}

void CanvasPainter::paintCameraOutline(QPainter& painter)
{
    painter.setClipping(false);
    QPen pen( QColor(49, 127, 158),
                2,
                Qt::DashLine,
                Qt::FlatCap,
                Qt::MiterJoin );
    pen.setCosmetic(true);
    painter.setPen( pen );
    painter.setBrush( Qt::NoBrush );
    painter.drawRect( mCameraRect);
}

QRect CanvasPainter::getCameraRect()
{
    return mCameraRect;
}
