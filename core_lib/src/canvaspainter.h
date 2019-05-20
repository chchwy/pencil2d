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

#ifndef CANVASPAINTER_H
#define CANVASPAINTER_H

#include <memory>
#include <QObject>
#include <QTransform>
#include <QPainter>
#include "log.h"


class Object;
class Layer;
class BitmapImage;
class ViewManager;
class MPTile;

enum class RENDER_LEVEL
{
    ALL,
    BACK_ONLY,
    CURRENT_LAYER_ONLY,
    TOP_ONLY,
    CACHED // for preview
};

struct CanvasPainterOptions
{

    struct VectorOptions
    {
        bool thinLinesEnabled = false;
        bool outlineEnabled = false;
    };

    float onionSkinMaxOpacity = 0.5f;
    float onionSkinMinOpacity = 0.1f;
    float zoomLevel = 1.0f;
    bool prevColoredOnionSkinEnabled = false;
    bool nextColoredOnionSkinEnabled = false;
    bool antiAliasingEnabled = false;
    bool onionSkinAbsoluteEnabled = false;
    bool isPlaying = false;
    bool onionWhilePlayback = false;
    bool gridEnabld = false;
    bool axisEnabled = false;
    bool prevOnionSkinEnabled = false;
    bool nextOnionSkinEnabled = false;
    int gridSizeW = 50; /* This is the grid Width IN PIXELS. The grid will scale with the image, though */
    int gridSizeH = 50; /* This is the grid Height IN PIXELS. The grid will scale with the image, though */
    int prevOnionSkinCount = 3;
    int nextOnionSkinCount = 3;
    int showLayersCount = 3;
    VectorOptions vectorOptions;
};


class CanvasPainter : public QObject
{
    Q_OBJECT

public:
    explicit CanvasPainter(QObject* parent = 0);
    virtual ~CanvasPainter();

    void setCanvas(QPixmap* canvas);
    void setViewTransform(const QTransform view);
    void setOptions(const CanvasPainterOptions& p) { mOptions = p; }
    void setTransformedSelection(QRect selection, QTransform transform);
    void ignoreTransformedSelection();
    QRect getCameraRect();

    void paint(QPainter& painter, const Object* object, int layerIndex, int frameIndex, QList<MPTile*> tilesToBeRendered);
    void paintFrameAtLayer(QPixmap &image, Object* object, int layer, int frame);
    void renderGrid(QPainter& painter);

    void initPaint(const Object *object, int layer, int frame, QPainter& painter);

private:
    void paintBackground(QPainter& painter);
    void paintOnionSkin(QPainter& painter);

    void paintPostEffects(QPainter& painter);

    void paintCurrentFrame(QPainter& painter, RENDER_LEVEL renderLevel);
    void paintCurrentFrameAtLayer(QPainter& painter, int layerIndex);
    void paintCachedFrameAtLayer(QPainter& painter, int layerIndex);

    void paintColoredOnionSkin(QPainter& painter, const int frameIndex);

    void paintBitmapFrame(QPainter&, Layer* layer, int nFrame, bool colorize, bool useLastKeyFrame);


    void paintBitmapFrame(QPainter& painter, Layer* layer, int frameIndex, bool colorizeOnionSkin);
    void paintVectorFrame(QPainter&, Layer* layer, int nFrame, bool colorize, bool useLastKeyFrame);

    void paintTransformedSelection(QPainter& painter);
    void paintGrid(QPainter& painter);
    void paintCameraBorder(QPainter& painter);
    void paintCameraOutline(QPainter& painter);
    void paintAxis(QPainter& painter);
    void prescale(BitmapImage* bitmapImage);
    void prescaleSurface(QPainter& painter, QImage& image, const QRectF& rect);

    bool isRectInsideCanvas(const QRectF& rect) const;

private:
    CanvasPainterOptions mOptions;

    const Object* mObject = nullptr;
    QPixmap* mCanvas = nullptr;
    QTransform mViewTransform;
    QTransform mViewInverse;

    QRect mCameraRect;

    int mCurrentLayerIndex = 0;
    int mFrameNumber = 0;

    QImage mScaledBitmap;

    bool bMultiLayerOnionSkin = false;

    // Handle selection transformation
    bool mRenderTransform = false;
    QRect mSelection;
    QTransform mSelectionTransform;

    QList<MPTile*> mTilesToBeRendered;

    QLoggingCategory mLog;
};

#endif // CANVASRENDERER_H
