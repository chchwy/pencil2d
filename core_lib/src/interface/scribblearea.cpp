/*

Pencil - Traditional Animation Software
Copyright (C) 2005-2007 Patrick Corrieri & Pascal Naidon
Copyright (C) 2012-2018 Matthew Chiawen Chang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/

#include "scribblearea.h"

#include <cmath>
#include <QMessageBox>
#include <QPixmapCache>
#include <QVBoxLayout>

#include "pointerevent.h"
#include "beziercurve.h"
#include "object.h"
#include "editor.h"
#include "layerbitmap.h"
#include "layerbitmapsurface.h"
#include "layervector.h"
#include "layercamera.h"
#include "bitmapimage.h"
#include "bitmapsurface.h"
#include "vectorimage.h"

#include "colormanager.h"
#include "toolmanager.h"
#include "strokemanager.h"
#include "layermanager.h"
#include "playbackmanager.h"
#include "viewmanager.h"

#include "mphandler.h"
#include "mpbrush.h"
#include "mpsurface.h"
#include "mptile.h"


ScribbleArea::ScribbleArea(QWidget* parent) : QWidget(parent),
mLog("ScribbleArea")
{
    setObjectName("ScribbleArea");

    // Qt::WA_StaticContents ensure that the widget contents are rooted to the top-left corner
    // and don't change when the widget is resized.
    setAttribute(Qt::WA_StaticContents);

    mStrokeManager.reset(new StrokeManager);
}

ScribbleArea::~ScribbleArea()
{
    delete mBufferImg;
}

bool ScribbleArea::init()
{
    mPrefs = mEditor->preference();
    mDoubleClickTimer = new QTimer(this);
    mMyPaint = MPHandler::handler();

    connect(mMyPaint, SIGNAL(newTile(MPSurface*, MPTile*)), this, SLOT(newTileCreated(MPSurface*, MPTile*)));
    connect(mMyPaint, SIGNAL(updateTile(MPSurface*, MPTile*)), this, SLOT(existingTileUpdated(MPSurface*, MPTile*)));
    connect(mMyPaint, SIGNAL(clearedSurface(MPSurface*)), this, SLOT(onClearedSurface(MPSurface*)));

//    mScene.setSceneRect(this->rect());
//    setScene(&mScene);
//    setAlignment((Qt::AlignTop | Qt::AlignLeft));
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    setOptimizationFlags(QGraphicsView::DontSavePainterState);
//    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
//    setTransformationAnchor(QGraphicsView::NoAnchor);
//    setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));

    connect(mPrefs, &PreferenceManager::optionChanged, this, &ScribbleArea::settingUpdated);
    connect(mDoubleClickTimer, &QTimer::timeout, this, &ScribbleArea::handleDoubleClick);
    mDoubleClickTimer->setInterval(50);

    const int curveSmoothingLevel = mPrefs->getInt(SETTING::CURVE_SMOOTHING);
    mCurveSmoothingLevel = curveSmoothingLevel / 20.0; // default value is 1.0

    mQuickSizing = mPrefs->isOn(SETTING::QUICK_SIZING);
    mMakeInvisible = false;
    mSomethingSelected = false;

    mIsSimplified = mPrefs->isOn(SETTING::OUTLINES);
    mMultiLayerOnionSkin = mPrefs->isOn(SETTING::MULTILAYER_ONION);

    mBufferImg = new BitmapImage;

    QRect newSelection(QPoint(0, 0), QSize(0, 0));
    mySelection = newSelection;
    myTransformedSelection = newSelection;
    myTempTransformedSelection = newSelection;
    mOffset.setX(0);
    mOffset.setY(0);
    selectionTransformation.reset();

//    mBackgroundItem = new QGraphicsPixmapItem();
//    mScene.addItem(mBackgroundItem);
//    mBackgroundItem->setZValue(0);

//    mCanvasItem = new QGraphicsPixmapItem();
//    mScene.addItem(mCanvasItem);
//    mCanvasItem->setZValue(1);

//    mCanvasBackItem = new QGraphicsPixmapItem();
//    mScene.addItem(mCanvasBackItem);
//    mCanvasBackItem->setZValue(2);

//    mCanvasTopItem = new QGraphicsPixmapItem();
//    mScene.addItem(mCanvasTopItem);
//    mCanvasTopItem->setZValue(10000);

    updateCanvasCursor();
//    updateBackground();

    setMouseTracking(true); // reacts to mouse move events, even if the button is not pressed

#if QT_VERSION >= 0x50900

    // tablet tracking first added in 5.9
    setTabletTracking(true);

#endif

    mDebugRect = QRectF(0, 0, 0, 0);

    QPixmapCache::setCacheLimit(100 * 1024); // unit is kb, so it's 100MB cache

    int nLength = mEditor->layers()->animationLength();
    mPixmapCacheKeys.resize(std::max(nLength, 240));

    mNeedUpdateAll = false;

    return true;
}

void ScribbleArea::settingUpdated(SETTING setting)
{
    switch (setting)
    {
    case SETTING::CURVE_SMOOTHING:
        setCurveSmoothing(mPrefs->getInt(SETTING::CURVE_SMOOTHING));
        break;
    case SETTING::TOOL_CURSOR:
        updateToolCursor();
        break;
    case SETTING::ONION_PREV_FRAMES_NUM:
    case SETTING::ONION_NEXT_FRAMES_NUM:
    case SETTING::ONION_MIN_OPACITY:
    case SETTING::ONION_MAX_OPACITY:
    case SETTING::ANTIALIAS:
    case SETTING::GRID:
    case SETTING::GRID_SIZE_W:
    case SETTING::GRID_SIZE_H:
    case SETTING::PREV_ONION:
    case SETTING::NEXT_ONION:
    case SETTING::ONION_BLUE:
    case SETTING::ONION_RED:
    case SETTING::INVISIBLE_LINES:
    case SETTING::OUTLINES:
        updateAllFrames();
        break;
    case SETTING::QUICK_SIZING:
        mQuickSizing = mPrefs->isOn(SETTING::QUICK_SIZING);
        break;
    case SETTING::MULTILAYER_ONION:
        mMultiLayerOnionSkin = mPrefs->isOn(SETTING::MULTILAYER_ONION);
        updateAllFrames();
    case SETTING::BACKGROUND_STYLE:
    case SETTING::SHADOW:
//        updateBackground();
        break;
    default:
        break;
    }

}

void ScribbleArea::updateToolCursor()
{
    setCursor(currentTool()->cursor());
    updateCanvasCursor();
    updateAllFrames();
}

void ScribbleArea::setCurveSmoothing(int newSmoothingLevel)
{
    mCurveSmoothingLevel = newSmoothingLevel / 20.0;
    updateAllFrames();
}

void ScribbleArea::setEffect(SETTING e, bool isOn)
{
    mPrefs->set(e, isOn);
    updateAllFrames();
}

void ScribbleArea::updateBackground()
{
//    // Init background with the size of the canvas
//    //
//    QPixmap bgPixmap = QPixmap(size());
//    bgPixmap.fill(QColor("#FFFFFF"));

//    QPainter painter;
//    painter.begin( &bgPixmap );

//    QString bgName = mPrefs->getString(SETTING::BACKGROUND_STYLE);

//    if (bgName.startsWith("#")) {
//        // Fill with color
//        //
//        bgPixmap.fill(QColor(bgName));
//    }
//    else {
//        // Fill with Pattern
//        //
//        QPixmap pattern = QPixmap(":background/"+bgName);
//        painter.drawTiledPixmap( bgPixmap.rect(), pattern );
//    }

//    // Draw Shadow
//    //
//    applyBackgroundShadow(painter);

//    mBackgroundItem->setPixmap(bgPixmap);
//    painter.end();
}

void ScribbleArea::applyBackgroundShadow(QPainter& painter)
{
    if (mPrefs->isOn(SETTING::SHADOW)) {
        int radius1 = 12;
        int radius2 = 8;

        QColor colour = Qt::black;
        qreal opacity = 0.15;

        QLinearGradient shadow = QLinearGradient( 0, 0, 0, radius1 );

        int r = colour.red();
        int g = colour.green();
        int b = colour.blue();
        qreal a = colour.alphaF();
        shadow.setColorAt( 0.0, QColor( r, g, b, qRound( a * 255 * opacity ) ) );
        shadow.setColorAt( 1.0, QColor( r, g, b, 0 ) );


        painter.setPen( Qt::NoPen );
        painter.setBrush( shadow );
        painter.drawRect( QRect( 0, 0, width(), radius1 ) );

        shadow.setFinalStop( radius1, 0 );
        painter.setBrush( shadow );
        painter.drawRect( QRect( 0, 0, radius1, height() ) );

        shadow.setStart( 0, height() );
        shadow.setFinalStop( 0, height() - radius2 );
        painter.setBrush( shadow );
        painter.drawRect( QRect( 0, height() - radius2, width(), height() ) );

        shadow.setStart( width(), 0 );
        shadow.setFinalStop( width() - radius2, 0 );
        painter.setBrush( shadow );
        painter.drawRect( QRect( width() - radius2, 0, width(), height() ) );
    }
}

void ScribbleArea::didCreateNewFrame(int frame)
{
    Q_UNUSED(frame);
    mMyPaint->clearSurface();
    update();
}

void ScribbleArea::prepareForDrawing()
{
    qDebug() << "prepare for drawing";
    mMyPaint->clearSurface();
    refreshSurface();

    // This loads the whole surface image into mypaint backend
    // should be used with caution, highly dependent on surface since.
    // if possible only load when frame changes.
    Layer* layer = mEditor->layers()->currentLayer();
    BitmapSurface* surfaceImage = currentBitmapSurfaceImage(layer);

    mMyPaint->loadTiles(surfaceImage->pixmaps(), surfaceImage->tilePositions());
}

void ScribbleArea::updatePreviousFrame(int index)
{
//    Layer* layer = mEditor->layers()->currentLayer();
//    BitmapSurface* surfaceImage = static_cast<LayerBitmapSurface*>(layer)->getBitmapImageAtFrame(index);

//    qDebug() << "previous frame is: " << index;
//    qDebug() << "previous keyframe updated";
//    if (surfaceImage->isModified()) {
//        qDebug() << "previous keyframe modified";
//        mMyPaint->clearSurface();
////        refreshSurface();
//        surfaceImage->setModified(false);
//    }
}

void ScribbleArea::showCurrentFrame()
{
    Layer* layer = mEditor->layers()->currentLayer();

    BitmapSurface* surfaceImage = currentBitmapSurfaceImage(layer);

    if (surfaceImage->isModified()) {
        qDebug() << "keyframe has been modified";
        prepareForDrawing();
        surfaceImage->setModified(false);

        // render tiles to image for faster playback
        surfaceImage->renderSurfaceImage();
    } else {
        mMyPaint->clearSurface();
        refreshSurface();
        frameFirstLoad = true;
    }

    qDebug() << "show current frame";
//    mMyPaint->clearSurface();

    // If the canvas needs to be updated, we make sur that there is no cache
//    //
//    if (mNeedUpdateAll) {
//        QPixmapCache::clear();
//    }
//    mNeedUpdateAll = false;

//    // Load mypaint surface

//    // ---- checks ------
//    Q_ASSERT( layer );
//    if ( layer == NULL ) { return; } // TODO: remove in future.

//    QPixmap currentImage = QPixmap(mCanvas.size());
//    mCanvasPainter.setViewTransform(mEditor->view()->getView(), mEditor->view()->getViewInverse());

//    Layer* layer = mEditor->layers()->currentLayer();
//    BitmapSurface* surfaceImage = currentBitmapSurfaceImage(layer);

    // render surface image to to tile
    // tile is then rendered in paintEvent
    for (int i = 0; i < surfaceImage->pixmaps().count(); i++) {

        const QPixmap* pixmap = surfaceImage->pixmaps().at(i).get();
        const QPoint pos = surfaceImage->tilePositions().at(i);
        MPTile *tile = getTileFromPos(pos);
        tile->setImage(pixmap->toImage());
    }

//    updateDirtyTiles();
    update();


    // We retrieve the entire canvas from the cache;
    // We draw it if it doesn't exist.
    //
    // This is used for optimizing scrubbing and move/scale speed.
    //
//    QString cachedFrameKey = getCachedFrameKey( frame );

//    bool hasCache = QPixmapCache::find( cachedFrameKey, mCanvas );
//    if ( !hasCache )
//    {
//        drawCanvas(frame, mCanvas.rect());
//    }

//    if (isInPreviewMode) {
//        loadFullCanvas();
//    }
//    else {
//        drawCanvasBack(frame, mCanvasBack.rect());
//        drawCanvasLayer(frame, mCanvasLayer.rect());
//        drawCanvasTop(frame, mCanvasTop.rect());

//        loadBackCanvas();
//        loadTiles();
//        loadTopCanvas();
//    }
}

void ScribbleArea::loadTiles()
{
//    QSize tileSize = QSize(MYPAINT_TILE_SIZE, MYPAINT_TILE_SIZE);


//    int nbTilesOnWidth = ceil((float)mCanvasLayer.width() / (float)tileSize.width());
//    int nbTilesOnHeight = ceil((float)mCanvasLayer.height() / (float)tileSize.height());

//    for (int h=0; h < nbTilesOnHeight; h++) {

//        for (int w=0; w < nbTilesOnWidth; w++) {

//            QPoint tilePos = QPoint(w*MYPAINT_TILE_SIZE, h*MYPAINT_TILE_SIZE);
//            MPTile *tile = getTileFromPos(tilePos);

////            QTransform v = mEditor->view()->getView();
////            QTransform t;

////            t.translate(v.dx()*0.5, v.dy()*0.5);
////            t.scale(mEditor->view()->scaling(), mEditor->view()->scaling());
////            t.translate(-v.dx()*0.5, -v.dy()*0.5);

////            tile->setTransform(t);

//            QRect tileRect = QRect(tilePos, tileSize);
//            QPixmap tileImage = mCanvasLayer.copy(tileRect);

//            tile->setImage(tileImage.toImage());
//        }
//    }
}

void ScribbleArea::switchToPreviewMode()
{
    if (!isInPreviewMode) {

        // Hide Decomposed Canvas show Full Canvas
        //
//        foreach (QGraphicsItem* gItem, mScene.items()) {
//            gItem->(false);
//        }
//        mBackgroundItem->setVisible(true);
//        mCanvasItem->setVisible(true);

        mNeedQuickUpdate = true;
        isInPreviewMode = true;

//        showCurrentFrame();
    }
}

void ScribbleArea::switchToDrawingMode()
{
    if (isInPreviewMode) {

        // Hide Full Canvas show Decomposed Canvas
        //
//        foreach (QGraphicsItem* gItem, mScene.items()) {
//            gItem->setVisible(true);
//        }
//        mCanvasItem->setVisible(false);

//        mNeedQuickUpdate = false;
//        isInPreviewMode = false;

//        showCurrentFrame();
    }
}

void ScribbleArea::loadFullCanvas()
{
    mCanvasItem->setPixmap(mCanvas);
    mCanvasItem->setVisible(true);
}

void ScribbleArea::loadBackCanvas()
{
    mCanvasBackItem->setPixmap(mCanvasBack);
}

void ScribbleArea::loadTopCanvas()
{
    mCanvasTopItem->setPixmap(mCanvasTop);
}

void ScribbleArea::drawCanvas( int frame, QRect rect )
{
    Object* object = mEditor->object();

    mCanvasPainter.setOptions( getRenderOptions() );
    mCanvasPainter.setCanvas( &mCanvas );
    mCanvasPainter.setViewTransform( mEditor->view()->getView(), mEditor->view()->getViewInverse());
    mCanvasPainter.paint(object, mEditor->layers()->currentLayerIndex(), frame, rect, mNeedQuickUpdate);

    // Cache current frame for faster render
    //
    QString cachedFrameKey = getCachedFrameKey( frame );

    QPixmap pm;
    if (QPixmapCache::find(cachedFrameKey, &pm)) {
        QPixmapCache::remove( cachedFrameKey );
    }

    QPixmapCache::insert( cachedFrameKey, mCanvas );

    return;
}

CanvasPainterOptions ScribbleArea::getRenderOptions()
{
    CanvasPainterOptions o;
    o.bPrevOnionSkin = mPrefs->isOn(SETTING::PREV_ONION);
    o.bNextOnionSkin = mPrefs->isOn(SETTING::NEXT_ONION);
    o.bColorizePrevOnion = mPrefs->isOn(SETTING::ONION_RED);
    o.bColorizeNextOnion = mPrefs->isOn(SETTING::ONION_BLUE);
    o.nPrevOnionSkinCount = mPrefs->getInt(SETTING::ONION_PREV_FRAMES_NUM);
    o.nNextOnionSkinCount = mPrefs->getInt(SETTING::ONION_NEXT_FRAMES_NUM);
    o.fOnionSkinMaxOpacity = mPrefs->getInt(SETTING::ONION_MAX_OPACITY);
    o.fOnionSkinMinOpacity = mPrefs->getInt(SETTING::ONION_MIN_OPACITY);
    o.bAntiAlias = mPrefs->isOn(SETTING::ANTIALIAS);
    o.bGrid = mPrefs->isOn(SETTING::GRID);
    o.nGridSizeW = mPrefs->getInt(SETTING::GRID_SIZE_W);
    o.nGridSizeH = mPrefs->getInt(SETTING::GRID_SIZE_H);
    o.bAxis = false;
    o.bThinLines = mPrefs->isOn(SETTING::INVISIBLE_LINES);
    o.bOutlines = mPrefs->isOn(SETTING::OUTLINES);
    o.nShowAllLayers = mShowAllLayers;
    o.bIsOnionAbsolute = (mPrefs->getString(SETTING::ONION_TYPE) == "absolute");
    o.scaling = mEditor->view()->scaling();
    o.onionWhilePlayback = mPrefs->getInt(SETTING::ONION_WHILE_PLAYBACK);
    o.isPlaying = mEditor->playback()->isPlaying() ? true : false;

    return o;
}

void ScribbleArea::drawCanvasBack(int frame, QRect rect)
{
    Object* object = mEditor->object();

    mCanvasPainter.setOptions(getRenderOptions());
    mCanvasPainter.setCanvas(&mCanvasBack);
    mCanvasPainter.setViewTransform(mEditor->view()->getView(), mEditor->view()->getViewInverse());
    mCanvasPainter.paintBackgroundToLayer(object, mEditor->layers()->currentLayerIndex(), frame, rect, mNeedQuickUpdate);
}

void ScribbleArea::drawCanvasLayer( int frame, QRect rect )
{
    Object* object = mEditor->object();

    mCanvasPainter.setOptions( getRenderOptions() );
    mCanvasPainter.setCanvas( &mCanvasLayer );
    mCanvasPainter.setViewTransform(mEditor->view()->getView(), mEditor->view()->getViewInverse());
    mCanvasPainter.paintLayer( object, mEditor->layers()->currentLayerIndex(), frame, rect, mNeedQuickUpdate );
}

void ScribbleArea::drawCanvasTop( int frame, QRect rect )
{
    Object* object = mEditor->object();

    mCanvasPainter.setOptions( getRenderOptions() );
    mCanvasPainter.setCanvas( &mCanvasTop );
    mCanvasPainter.setViewTransform(mEditor->view()->getView(), mEditor->view()->getViewInverse());
    mCanvasPainter.paintTopToLayer( object, mEditor->layers()->currentLayerIndex(), frame, rect, mNeedQuickUpdate );
}


QString ScribbleArea::getCachedFrameKey(int frame)
{
    int lastFrameNumber = mEditor->layers()->LastFrameAtFrame( frame );
    return "frame" + QString::number( lastFrameNumber );
}


/************************************************************************************/
// update methods

void ScribbleArea::updateCurrentFrame()
{
    updateFrame(mEditor->currentFrame());
}

void ScribbleArea::updateFrame(int frame)
{
//    QString cachedFrameKey = getCachedFrameKey(frame);
//    QPixmapCache::remove( cachedFrameKey );

    qDebug() << "update frame";
    if (mEditor) {
        showCurrentFrame();
    }
}

void ScribbleArea::updateAllFrames()
{
    QPixmapCache::clear();

    if (mEditor) {
//        showCurrentFrame();
    }
    mNeedUpdateAll = false;
}

void ScribbleArea::updateAllVectorLayersAtCurrentFrame()
{
    updateAllVectorLayersAt(mEditor->currentFrame());
}

void ScribbleArea::updateAllVectorLayersAt(int frameNumber)
{
    Layer* layer = mEditor->layers()->currentLayer();
    for (int i = 0; i < mEditor->object()->getLayerCount(); i++)
    {
        layer = mEditor->object()->getLayer(i);
        if (layer->type() == Layer::VECTOR)
        {
            currentVectorImage(layer)->modification();
        }
    }

    if (layer->type() == layer->VECTOR) {
        updateFrame(frameNumber);
    }
}

void ScribbleArea::setModified(int layerNumber, int frameNumber)
{
    Layer* layer = mEditor->object()->getLayer(layerNumber);
    if (layer)
    {
        layer->setModified(frameNumber, true);
        emit modification(layerNumber);
        updateAllFrames();
    }
}

/************************************************************************/
/* key event handlers                                                   */
/************************************************************************/

void ScribbleArea::keyPressEvent(QKeyEvent *event)
{
    // Don't handle this event on auto repeat
    if (event->isAutoRepeat()) { return; }

    mKeyboardInUse = true;

    if (mMouseInUse) { return; } // prevents shortcuts calls while drawing
    if (mInstantTool) { return; } // prevents shortcuts calls while using instant tool

    if (currentTool()->keyPressEvent(event))
    {
        return; // has been handled by tool
    }

    // --- fixed control key shortcuts ---
    if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
    {
        setTemporaryTool(ERASER);
        return;
    }

    // ---- fixed normal keys ----
    switch (event->key())
    {
    case Qt::Key_Right:
        if (mSomethingSelected)
        {
            myTempTransformedSelection.translate(1, 0);
            myTransformedSelection = myTempTransformedSelection;
            calculateSelectionTransformation();
            paintTransformedSelection();
        }
        else
        {
            mEditor->scrubForward();
            event->ignore();
        }
        break;
    case Qt::Key_Left:
        if (mSomethingSelected)
        {
            myTempTransformedSelection.translate(-1, 0);
            myTransformedSelection = myTempTransformedSelection;
            calculateSelectionTransformation();
            paintTransformedSelection();
        }
        else
        {
            mEditor->scrubBackward();
            event->ignore();
        }
        break;
    case Qt::Key_Up:
        if (mSomethingSelected)
        {
            myTempTransformedSelection.translate(0, -1);
            myTransformedSelection = myTempTransformedSelection;
            calculateSelectionTransformation();
            paintTransformedSelection();
        }
        else
        {
            mEditor->layers()->gotoPreviouslayer();
            event->ignore();
        }
        break;
    case Qt::Key_Down:
        if (mSomethingSelected)
        {
            myTempTransformedSelection.translate(0, 1);
            myTransformedSelection = myTempTransformedSelection;
            calculateSelectionTransformation();
            paintTransformedSelection();
        }
        else
        {
            mEditor->layers()->gotoNextLayer();
            event->ignore();
        }
        break;
    case Qt::Key_Return:
        if (mSomethingSelected)
        {
            applyTransformedSelection();
            //paintTransformedSelection();
            deselectAll();
        }
        else
        {
            event->ignore();
        }
        break;
    case Qt::Key_Escape:
        if (mSomethingSelected)
        {
            deselectAll();
            applyTransformedSelection();
        }
        break;
    case Qt::Key_Backspace:
        if (mSomethingSelected)
        {
            deleteSelection();
            deselectAll();
        }
        break;
    case Qt::Key_Space:
        setTemporaryTool(HAND); // just call "setTemporaryTool()" to activate temporarily any tool
        break;
    default:
        event->ignore();
    }
}

void ScribbleArea::keyReleaseEvent(QKeyEvent *event)
{
    // Don't handle this event on auto repeat
    //
    if (event->isAutoRepeat()) {
        return;
    }

    mKeyboardInUse = false;

    if (mMouseInUse) { return; }

    if (mInstantTool) // temporary tool
    {
        currentTool()->keyReleaseEvent(event);
        setPrevTool();
        return;
    }
    if (currentTool()->keyReleaseEvent(event))
    {
        // has been handled by tool
        return;
    }
}



/************************************************************************************/
// mouse and tablet event handlers
void ScribbleArea::wheelEvent(QWheelEvent* event)
{
    // Don't change view if tool is in use
    if (mMouseInUse) return;

    const QPoint pixels = event->pixelDelta();
    const QPoint angle = event->angleDelta();

    if (!pixels.isNull())
    {
        float delta = pixels.y();
        float currentScale = mEditor->view()->scaling();

//        qDebug() << delta;
//        int newD = delta > 0 ? 2 : -2;
        float newScale = currentScale * (1.f + (delta * 0.01f));
//        float newScale = currentScale * (1.f + (newD * 0.02f));
//        qDebug() << newScale;
        mEditor->view()->scale(newScale);
    }
    else if (!angle.isNull())
    {
        float delta = angle.y();
        if (delta < 0)
        {
            mEditor->view()->scaleDown();
        }
        else
        {
            mEditor->view()->scaleUp();
        }
    }

    if (mEditor->view()->transformUpdated()) {
//        refreshSurface();
    }
    updateCanvasCursor();
    event->accept();

    update();
}

void ScribbleArea::tabletEvent(QTabletEvent *e)
{
    PointerEvent event(e);


    if (event.pointerType() == QTabletEvent::Eraser)
    {
        editor()->tools()->tabletSwitchToEraser();
    }
    else
    {
        editor()->tools()->tabletRestorePrevTool();
    }

    if (isLayerPaintable())
    {
        if (event.type() == QTabletEvent::TabletPress)
        {
            mStrokeManager->setTabletinUse(true);
            mStrokeManager->pointerPressEvent(&event);
            if (mIsFirstClick)
            {
                mIsFirstClick = false;
                mDoubleClickTimer->start();
                pointerPressEvent(&event);
            }
            else
            {
                qreal distance = QLineF(currentTool()->getCurrentPressPoint(), currentTool()->getLastPressPoint()).length();

                if (mDoubleClickMillis <= DOUBLE_CLICK_THRESHOLD && distance < 5.0) {
                    currentTool()->pointerDoubleClickEvent(&event);
                }
                else
                {
                    // in case we handled the event as double click but really should have handled it as single click.
                    pointerPressEvent(&event);
                }
            }
        }
        else if (event.type() == QTabletEvent::TabletMove)
        {
            mStrokeManager->pointerMoveEvent(&event);
            pointerMoveEvent(&event);
        }
        else if (event.type() == QTabletEvent::TabletRelease)
        {
            mStrokeManager->pointerReleaseEvent(&event);
            pointerReleaseEvent(&event);
            mStrokeManager->setTabletinUse(false);
        }
    }
    event.accept();
}

void ScribbleArea::pointerPressEvent(PointerEvent* event)
{
    if (event->button() & Qt::LeftButton || event->button() & Qt::RightButton)
    {
        mOffset = getCurrentOffset();
    }

    if (event->button() == Qt::RightButton)
    {
        mMouseRightButtonInUse = true;
        getTool(HAND)->pointerPressEvent(event);
        return;
    }

    if (currentTool()->type() != HAND && (event->button() != Qt::RightButton))
    {
        Layer* layer = mEditor->layers()->currentLayer();
        if (!layer->visible())
        {
            showLayerNotVisibleWarning(); // FIXME: crash when using tablets
            return;
        }
    }

    if (event->buttons() & Qt::MidButton)
    {
        setTemporaryTool(HAND);
    }

    const bool isPressed = event->buttons() & Qt::LeftButton;
    if (isPressed && mQuickSizing)
    {
        //qDebug() << "Start Adjusting" << event->buttons();
        if (isDoingAssistedToolAdjustment(event->modifiers()))
        {
            return;
        }
    }

    if (event->button() == Qt::LeftButton)
    {
        currentTool()->pointerPressEvent(event);
    }
}

void ScribbleArea::pointerMoveEvent(PointerEvent* event)
{
    updateCanvasCursor();

    // TODO: fix view doesn't update on drag....
    if (mEditor->view()->transformUpdated()) {
//        refreshSurface();
        mEditor->view()->transformUpdatedState(false);
        update();
    }

    if (event->buttons() & (Qt::LeftButton | Qt::RightButton))
    {
        mOffset = getCurrentOffset();

        // --- use SHIFT + drag to resize WIDTH / use CTRL + drag to resize FEATHER ---
        if (currentTool()->isAdjusting())
        {
            currentTool()->adjustCursor(mOffset.x(), event->modifiers()); //updates cursors given org width or feather and x
            return;
        }
    }

    if (event->buttons() == Qt::RightButton)
    {
        setCursor(getTool(HAND)->cursor());
        getTool(HAND)->pointerMoveEvent(event);
        event->accept();
        return;
    }
    currentTool()->pointerMoveEvent(event);
}

void ScribbleArea::pointerReleaseEvent(PointerEvent* event)
{
    if (currentTool()->isAdjusting())
    {
        currentTool()->stopAdjusting();
        mEditor->tools()->setWidth(currentTool()->properties.width);
        return; // [SHIFT]+drag OR [CTRL]+drag
    }

    if (event->button() == Qt::RightButton)
    {
        getTool(HAND)->pointerReleaseEvent(event);
        mMouseRightButtonInUse = false;
        return;
    }

    //qDebug() << "release event";
    currentTool()->pointerReleaseEvent(event);

    // ---- last check (at the very bottom of mouseRelease) ----
    if (mInstantTool && !mKeyboardInUse) // temp tool and released all keys ?
    {
        setPrevTool();
    }
}

void ScribbleArea::handleDoubleClick()
{
    mDoubleClickMillis += 100;

    if (mDoubleClickMillis >= DOUBLE_CLICK_THRESHOLD)
    {
        mDoubleClickMillis = 0;
        mIsFirstClick = true;
        mDoubleClickTimer->stop();
    }
}

bool ScribbleArea::isLayerPaintable() const
{
    if (!areLayersSane())
        return false;

    Layer* layer = mEditor->layers()->currentLayer();
    return layer->type() == Layer::BITMAP || layer->type() == Layer::VECTOR;
}

bool ScribbleArea::areLayersSane() const
{
    Layer* layer = mEditor->layers()->currentLayer();
    // ---- checks ------
    if (layer == nullptr) { return false; }
    if (layer->type() == Layer::VECTOR)
    {
        VectorImage* vectorImage = currentVectorImage(layer);
        if (vectorImage == nullptr) { return false; }
    }
    if (layer->type() == Layer::BITMAP)
    {
        BitmapImage* bitmapImage = currentBitmapImage(layer);
        if (bitmapImage == nullptr) { return false; }
    }
    // ---- end checks ------

    return true;
}

bool ScribbleArea::allowSmudging()
{
    ToolType toolType = currentTool()->type();
    if (toolType == SMUDGE)
    {
        return true;
    }
    return false;
}

void ScribbleArea::mousePressEvent(QMouseEvent* e)
{
    if (mStrokeManager->isTabletInUse()) { e->ignore(); return; }

    PointerEvent event(e);
    mMouseInUse = true;

    mStrokeManager->pointerPressEvent(&event);

    pointerPressEvent(&event);
}

void ScribbleArea::mouseMoveEvent(QMouseEvent* e)
{
    PointerEvent event(e);

    mStrokeManager->pointerMoveEvent(&event);

    pointerMoveEvent(&event);

#ifdef DEBUG_FPS
    if (mMouseInUse)
    {
        clock_t curTime = clock();
        mDebugTimeQue.push_back(curTime);

        while (mDebugTimeQue.size() > 30)
        {
            mDebugTimeQue.pop_front();
        }

        if (mDebugTimeQue.size() > 30)
        {
            clock_t interval = mDebugTimeQue.back() - mDebugTimeQue.front();
            double fps = mDebugTimeQue.size() / ((double)interval) * CLOCKS_PER_SEC;
            qDebug() << fps;
        }
    }
#endif
}

void ScribbleArea::mouseReleaseEvent(QMouseEvent* e)
{
    // Workaround for tablet issue (#677 part 2)
    if (mStrokeManager->isTabletInUse() || !isMouseInUse()) { e->ignore(); return; }
    PointerEvent event(e);

    mStrokeManager->pointerReleaseEvent(&event);

    pointerReleaseEvent(&event);
    mMouseInUse = false;
}

void ScribbleArea::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (mStrokeManager->isTabletInUse()) { e->ignore(); return; }
    PointerEvent event(e);
    mStrokeManager->pointerPressEvent(&event);

    currentTool()->pointerDoubleClickEvent(&event);
}

void ScribbleArea::resizeEvent(QResizeEvent* event)
{
    int width = size().width();
    int height = size().height();

    // Canvas size must be even for the coordinate translation
    // to the center, to be pixel precise
    //
    if ((width%2) == 1) {
        width ++;
    }

    if ((height%2) == 1) {
        height ++;
    }

    QSize newSize(width, height);

    mCanvas = QPixmap( newSize );
    mCanvas.fill(Qt::transparent);

    mCanvasBack = QPixmap( newSize );
    mCanvasBack.fill(Qt::transparent);

    mCanvasLayer = QPixmap( newSize );
    mCanvasLayer.fill(Qt::transparent);

    mCanvasTop = QPixmap( newSize );
    mCanvasTop.fill(Qt::transparent);


//    QTransform v = mEditor->view()->getViewInverse();
//    QRect mappedRect = v.mapRect(this->rect());
//    mMyPaint->setSurfaceSize(mappedRect.size());
    mMyPaint->setSurfaceSize(newSize);

//    this->setStyl

    QWidget::resizeEvent( event );

//    this->setStyleSheet("background-color:blue;");

    mEditor->view()->setCanvasSize( newSize );
//    this->clearSurfaceBuffer();
//    mMyPaint->clearSurface();

    updateBackground();
    updateAllFrames();
}

bool ScribbleArea::isDoingAssistedToolAdjustment(Qt::KeyboardModifiers keyMod)
{
    if ((keyMod == Qt::ShiftModifier) && (currentTool()->properties.width > -1))
    {
        //adjust width if not locked
        currentTool()->startAdjusting(WIDTH, 1);
        return true;
    }
    if ((keyMod == Qt::ControlModifier) && (currentTool()->properties.feather > -1))
    {
        //adjust feather if not locked
        currentTool()->startAdjusting(FEATHER, 1);
        return true;
    }
    if ((keyMod == (Qt::ControlModifier | Qt::AltModifier)) &&
        (currentTool()->properties.feather > -1))
    {
        //adjust feather if not locked
        currentTool()->startAdjusting(FEATHER, 0);
        return true;
    }
    return false;
}

QPointF ScribbleArea::getCurrentOffset()
{
    return currentTool()->getCurrentPoint() - currentTool()->getCurrentPressPoint();
}

void ScribbleArea::showLayerNotVisibleWarning()
{
    QMessageBox::warning(this, tr("Warning"),
                         tr("You are drawing on a hidden layer! Please select another layer (or make the current layer visible)."),
                         QMessageBox::Ok,
                         QMessageBox::Ok);
}

void ScribbleArea::paintBitmapBuffer()
{
//    LayerBitmap* layer = static_cast<LayerBitmap*>(mEditor->layers()->currentLayer());
    LayerBitmapSurface* layer = static_cast<LayerBitmapSurface*>(mEditor->layers()->currentLayer());
    Q_ASSERT(layer);
    Q_ASSERT(layer->type() == Layer::BITMAP);

    int frameNumber = mEditor->currentFrame();

    // If there is no keyframe at or before the current position,
    // just return (since we have nothing to paint on).
    if (layer->getLastKeyFrameAtPosition(frameNumber) == nullptr)
    {
        updateCurrentFrame();
        return;
    }

    // Clear the temporary pixel path
//    BitmapImage* targetImage = currentBitmapImage(layer);
    BitmapSurface* surfaceImage = currentBitmapSurfaceImage(layer);
    if (surfaceImage != nullptr)
    {
        // We use source here because mypaint contains the same image as target image..
        QPainter::CompositionMode cm = QPainter::CompositionMode_Source;
        switch (currentTool()->type())
        {
        case ERASER:
        case BRUSH:
        case PEN:
        case PENCIL:
            if (getTool(currentTool()->type())->properties.preserveAlpha)
            {
                cm = QPainter::CompositionMode_SourceAtop;
            }
            break;
        default: //nothing
            break;
        }

        // adds content from canvas and saves to surfaceimage
        for (MPTile* item : mTempTiles.values()) {
            QPixmap tilePixmap = QPixmap::fromImage(item->image());
            surfaceImage->addBitmapPiece(QPixmap::fromImage(item->image()), item->pos().toPoint());

            // load the new tiles from buffer into mypaint
            mMyPaint->loadTile(tilePixmap, item->pos().toPoint());
        }

        // render tiles to image for faster playback
        surfaceImage->renderSurfaceImage();
//        surfaceImage->writeFile("/Users/CandyFace/Desktop/surfaceBuffer.png");

        // add tiles to mypaint
//        mMyPaint->clearSurface();
//        mMyPaint->loadTiles(surfaceImage->pixmaps(), surfaceImage->tilePositions());
//        surfaceImage->writeFile("/Users/CandyFace/Desktop/frame.png");

//        BitmapSurface* bitmap = new BitmapSurface();

//        QImage* strokeImage = new QImage(mMyPaint->renderImage(mEditor->view()->getView()));

//            mMyPaint->renderImage(mEditor->view()->getView());
//        QRect rect = mEditor->view()->mapScreenToCanvas(strokeImage->rect()).toRect();

//        mBufferImg->setImage(strokeImage);
//        mBufferImg->transform(rect, false);

//        targetImage->paste(mBufferImg, cm);
    }

//    BitmapSurface* bitmap = new BitmapSurface();

//    for (MPTile* item : mTiles.values()) {

//        QRect tileRect = QRect(QPoint(item->x(),item->y()),QSize(item->boundingRect().width(), item->boundingRect().height()));
//        bitmap->addBitmapPiece(QPixmap::fromImage(item->image()), tileRect);
//    }
//    bitmap->writeFile("/Users/CandyFace/Desktop/glued.png");

//    drawCanvas(frameNumber, this->rect());

    // Update the cache for the last key-frame.
//    auto lastKeyFramePosition = mEditor->layers()->LastFrameAtFrame(frameNumber);
//    QPixmapCache::remove(mPixmapCacheKeys[lastKeyFramePosition]);
//    mPixmapCacheKeys[lastKeyFramePosition] = QPixmapCache::Key();

//    mMyPaint->clearSurface();

    refreshSurface();
    update();

//    if (!mTiles.isEmpty()) {
//        mTiles.clear();
//    }

//    Layer* layer = mEditor->layers()->currentLayer();
//    BitmapSurface* surfaceImage = currentBitmapSurfaceImage(layer);
//    mMyPaint->clearSurface();
//    clearSurfaceBuffer();
//    clearSurfaceBuffer();
//    mMyPaint->refreshSurface();

//    mMyPaint->refreshSurface();


    layer->setModified(frameNumber, true);
    emit modification(frameNumber);
}

void ScribbleArea::paintBitmapBufferRect(const QRect& rect)
{
//    if (allowSmudging() || mEditor->playback()->isPlaying())
//    {
        Layer* layer = mEditor->layers()->currentLayer();
        Q_ASSERT(layer);

        BitmapImage* targetImage = currentBitmapImage(layer);

        if (targetImage != nullptr)
        {
            QPainter::CompositionMode cm = QPainter::CompositionMode_SourceOver;
            switch (currentTool()->type())
            {
            case ERASER:
                cm = QPainter::CompositionMode_DestinationOut;
                break;
            case BRUSH:
            case PEN:
            case PENCIL:
                if (getTool(currentTool()->type())->properties.preserveAlpha)
                {
                    cm = QPainter::CompositionMode_SourceAtop;
                }
                break;
            default: //nothing
                break;
            }
            targetImage->paste(mBufferImg, cm);

//            QImage* strokeImage = new QImage(mMyPaint->renderImage(mEditor->view()->getView()));

//            QRect rect = mEditor->view()->mapScreenToCanvas(strokeImage->rect()).toRect();

//            mBufferImg->setImage(strokeImage);
//            mBufferImg->transform(rect, false);
//            drawCanvas(1, this->rect());
//            mCanvasPainter.paintBuffer(mBufferImg);
        }

        // Clear the buffer
        mBufferImg->clear();

        int frameNumber = mEditor->currentFrame();
        layer->setModified(frameNumber, true);

        QPixmapCache::remove(mPixmapCacheKeys[frameNumber]);
        mPixmapCacheKeys[frameNumber] = QPixmapCache::Key();

//        drawCanvas(frameNumber, rect.adjusted(-1, -1, 1, 1));
//    }
}

void ScribbleArea::clearBitmapBuffer()
{
    mBufferImg->clear();
}

//void ScribbleArea::clearSurfaceBuffer()
//{
//    if (!mTiles.isEmpty()) {
//        mTiles.clear();
//    }
//}

void ScribbleArea::drawLine(QPointF P1, QPointF P2, QPen pen, QPainter::CompositionMode cm)
{
    mBufferImg->drawLine(P1, P2, pen, cm, mPrefs->isOn(SETTING::ANTIALIAS));
}

void ScribbleArea::drawPath(QPainterPath path, QPen pen, QBrush brush, QPainter::CompositionMode cm)
{
    mBufferImg->drawPath(path, pen, brush, cm, mPrefs->isOn(SETTING::ANTIALIAS));
}

void ScribbleArea::refreshBitmap(const QRectF& rect, int rad)
{
    QRectF updatedRect = mEditor->view()->mapCanvasToScreen(rect.normalized().adjusted(-rad, -rad, +rad, +rad));
    update(updatedRect.toRect());
}

void ScribbleArea::refreshVector(const QRectF& rect, int rad)
{
    rad += 1;
    //QRectF updatedRect = mEditor->view()->mapCanvasToScreen( rect.normalized().adjusted( -rad, -rad, +rad, +rad ) );
    update(rect.normalized().adjusted(-rad, -rad, +rad, +rad).toRect());

    //qDebug() << "Logical:  " << rect;
    //qDebug() << "Physical: " << mEditor->view()->mapCanvasToScreen( rect.normalized() );
    //update();
}

void ScribbleArea::paintCanvasCursor(QPainter& painter)
{
    QTransform view = mEditor->view()->getView();
    QPointF mousePos = currentTool()->getCurrentPoint();
    int centerCal = mCursorImg.width() / 2;

    mTransformedCursorPos = view.map(mousePos);

    // reset matrix
    view.reset();

    painter.setTransform(view);
    mCursorCenterPos.setX(centerCal);
    mCursorCenterPos.setY(centerCal);

    painter.drawPixmap(QPoint(mTransformedCursorPos.x() - mCursorCenterPos.x(),
                              mTransformedCursorPos.y() - mCursorCenterPos.y()),
                       mCursorImg);

    // update center of transformed img for rect only
    mTransCursImg = mCursorImg.transformed(view);

    mCursorCenterPos.setX(centerCal);
    mCursorCenterPos.setY(centerCal);
}

void ScribbleArea::updateCanvasCursor()
{
    float scalingFac = mEditor->view()->scaling();
    qreal brushWidth = currentTool()->properties.width;
    qreal brushFeather = currentTool()->properties.feather;
    if (currentTool()->isAdjusting())
    {
        mCursorImg = currentTool()->quickSizeCursor(brushWidth, brushFeather, scalingFac);
    }
    else if (mEditor->preference()->isOn(SETTING::DOTTED_CURSOR))
    {
        bool useFeather = currentTool()->properties.useFeather;
        mCursorImg = currentTool()->canvasCursor(brushWidth, brushFeather, useFeather, scalingFac, width());
    }
    else
    {
        mCursorImg = QPixmap(); // if above does not comply, deallocate image
    }

    // update cursor rect
    QPoint translatedPos = QPoint(mTransformedCursorPos.x() - mCursorCenterPos.x(),
                                  mTransformedCursorPos.y() - mCursorCenterPos.y());

    update(mTransCursImg.rect().adjusted(-1, -1, 1, 1)
           .translated(translatedPos));

}

void ScribbleArea::handleDrawingOnEmptyFrame()
{
    auto layer = mEditor->layers()->currentLayer();

    if (!layer || !layer->isPaintable())
    {
        return;
    }

    int frameNumber = mEditor->currentFrame();
    auto previousKeyFrame = layer->getLastKeyFrameAtPosition(frameNumber);

    if (layer->getKeyFrameAt(frameNumber) == nullptr)
    {
        // Drawing on an empty frame; take action based on preference.
        int action = mPrefs->getInt(SETTING::DRAW_ON_EMPTY_FRAME_ACTION);

        switch (action)
        {
        case CREATE_NEW_KEY:
            mEditor->addNewKey();
            mEditor->scrubTo(frameNumber);  // Refresh timeline.

            // Hack to clear previous frame's content.
            if (layer->type() == Layer::BITMAP  &&  previousKeyFrame)
            {
                auto asBitmapImage = dynamic_cast<BitmapImage *> (previousKeyFrame);

                if (asBitmapImage)
                {
                    drawCanvas(frameNumber, asBitmapImage->bounds());
                }
            }

            if (layer->type() == Layer::VECTOR)
            {
                auto asVectorImage = dynamic_cast<VectorImage *> (previousKeyFrame);

                if (asVectorImage)
                {
                    auto copy(*asVectorImage);
                    copy.selectAll();

                    drawCanvas(frameNumber, copy.getSelectionRect().toRect());
                }
            }

            break;
        case DUPLICATE_PREVIOUS_KEY:
        {
            if (previousKeyFrame)
            {
                KeyFrame* dupKey = previousKeyFrame->clone();
                layer->addKeyFrame(frameNumber, dupKey);
                mEditor->scrubTo(frameNumber);  // Refresh timeline.
            }
            break;
        }
        case KEEP_DRAWING_ON_PREVIOUS_KEY:
            // No action needed.
            break;
        default:
            break;
        }
    }
}

void ScribbleArea::paintEvent(QPaintEvent* event)
{
//    if (!mMouseInUse || currentTool()->type() == MOVE || currentTool()->type() == HAND || mMouseRightButtonInUse)
//    {
//        // --- we retrieve the canvas from the cache; we create it if it doesn't exist
//        int curIndex = mEditor->currentFrame();
//        int frameNumber = mEditor->layers()->LastFrameAtFrame(curIndex);

//        QPixmapCache::Key cachedKey = mPixmapCacheKeys[frameNumber];

//        if (!QPixmapCache::find(cachedKey, &mCanvas))
//        {
//            drawCanvas(mEditor->currentFrame(), event->rect());

//            mPixmapCacheKeys[frameNumber] = QPixmapCache::insert(mCanvas);
//            //qDebug() << "Repaint canvas!";
//        }
//    }

//    if (currentTool()->type() == MOVE)
//    {
//        Layer* layer = mEditor->layers()->currentLayer();
//        Q_CHECK_PTR(layer);
//        if (layer->type() == Layer::VECTOR)
//        {
//            currentVectorImage(layer)->setModified(true);
//        }
//    }

    QPainter painter(this);

//    // paints the canvas
//    painter.setWorldMatrixEnabled(false);

//     qDebug() << "did update";
    QTransform v = mEditor->view()->getView();
    if (editor()->playback()->isPlaying()) {

        // optimize playback by using saved surface image
        // because the entire canvas has to be updated
        Layer* layer = mEditor->layers()->currentLayer();
        BitmapSurface* surface = currentBitmapSurfaceImage(layer);
        QRect mappedBounds = v.mapRect(surface->bounds());
        painter.drawImage(mappedBounds,surface->surfaceImage());
    } else {
        // otherwise paint tiles
        painter.setClipping(true);
        painter.setClipRect(this->rect());
        painter.save();

        int tilesUpdated = 0;

        QHash<QString, MPTile*> tilesToBeRendered;

        if (isPainting) {
            tilesToBeRendered = mTempTiles;
        } else {
            tilesToBeRendered = mTiles;
        }

        for (MPTile* item : tilesToBeRendered.values()) {

            QRectF tileRect = QRectF(item->pos(),QSize(item->boundingRect().width(), item->boundingRect().height()));
            tileRect = v.mapRect(tileRect);

            QImage image = item->image();

            // TODO: move to prescale method
            if (mEditor->view()->scaling() < 1.5f) {
                painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);

                // Only prescale when the content is small enough, as it's performance intensive to upscale larger images.
                if (mEditor->view()->scaling() < 0.5f) {
                image = item->image().scaled(tileRect.size().toSize(),
                                                     Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
            }


            bool visualCanvasContainstiles = this->rect().adjusted(-tileRect.width(),
                                                                   -tileRect.width(),
                                                                   tileRect.width(),
                                                                   tileRect.width()).contains(tileRect.toRect());
            if (visualCanvasContainstiles) {
                painter.drawPixmap(tileRect.toRect(), QPixmap::fromImage(image));
                tilesUpdated++;
            }

        }
        qDebug() << "number of tiles updated: " << tilesUpdated;
        painter.restore();
    }

//    painter.drawPixmap(QPoint(0, 0), mCanvas);
//    painter.draw

//    Layer* layer = mEditor->layers()->currentLayer();

//    if (!editor()->playback()->isPlaying())    // we don't need to display the following when the animation is playing
//    {
//        if (layer->type() == Layer::VECTOR)
//        {
//            VectorImage* vectorImage = currentVectorImage(layer);
//            switch (currentTool()->type())
//            {
//            case SMUDGE:
//            case HAND:
//            {
//                painter.save();
//                painter.setWorldMatrixEnabled(false);
//                painter.setRenderHint(QPainter::Antialiasing, false);
//                // ----- paints the edited elements
//                QPen pen2(Qt::black, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
//                painter.setPen(pen2);
//                QColor colour;
//                // ------------ vertices of the edited curves
//                colour = QColor(200, 200, 200);
//                painter.setBrush(colour);
//                for (int k = 0; k < vectorSelection.curve.size(); k++)
//                {
//                    int curveNumber = vectorSelection.curve.at(k);

//                    for (int vertexNumber = -1; vertexNumber < vectorImage->getCurveSize(curveNumber); vertexNumber++)
//                    {
//                        QPointF vertexPoint = vectorImage->getVertex(curveNumber, vertexNumber);
//                        QRectF rectangle(mEditor->view()->mapCanvasToScreen(vertexPoint) - QPointF(3.0, 3.0), QSizeF(7, 7));
//                        if (rect().contains(mEditor->view()->mapCanvasToScreen(vertexPoint).toPoint()))
//                        {
//                            painter.drawRect(rectangle);
//                        }
//                    }
//                }
//                // ------------ selected vertices of the edited curves
//                colour = QColor(100, 100, 255);
//                painter.setBrush(colour);
//                for (int k = 0; k < vectorSelection.vertex.size(); k++)
//                {
//                    VertexRef vertexRef = vectorSelection.vertex.at(k);
//                    QPointF vertexPoint = vectorImage->getVertex(vertexRef);
//                    QRectF rectangle0 = QRectF(mEditor->view()->mapCanvasToScreen(vertexPoint) - QPointF(3.0, 3.0), QSizeF(7, 7));
//                    painter.drawRect(rectangle0);
//                }
//                // ----- paints the closest vertices
//                colour = QColor(255, 0, 0);
//                painter.setBrush(colour);
//                if (vectorSelection.curve.size() > 0)
//                {
//                    for (int k = 0; k < mClosestVertices.size(); k++)
//                    {
//                        VertexRef vertexRef = mClosestVertices.at(k);
//                        QPointF vertexPoint = vectorImage->getVertex(vertexRef);

//                        QRectF rectangle = QRectF(mEditor->view()->mapCanvasToScreen(vertexPoint) - QPointF(3.0, 3.0), QSizeF(7, 7));
//                        painter.drawRect(rectangle);
//                    }
//                }
//                painter.restore();
//                break;
//            }
//            default:
//            {
//                break;
//            }
//            } // end siwtch
//        }

//        // paints the  buffer image
//        if (mEditor->layers()->currentLayer() != nullptr)
//        {
//            painter.setOpacity(1.0);
//            if (mEditor->layers()->currentLayer()->type() == Layer::BITMAP)
//            {
//                painter.setWorldMatrixEnabled(true);
//                painter.setTransform(mEditor->view()->getView());
//            }
//            else if (mEditor->layers()->currentLayer()->type() == Layer::VECTOR)
//            {
//                painter.setWorldMatrixEnabled(false);
//            }

//            // TODO: move to above if vector statement
//            mBufferImg->paintImage(painter);

//            paintCanvasCursor(painter);
//        }

//        mCanvasPainter.renderGrid(painter);

//        // paints the selection outline
//        if (mSomethingSelected && !myTempTransformedSelection.isNull())
//        {
//            paintSelectionVisuals(painter);
//        }
//    }

//    // outlines the frame of the viewport
//#ifdef _DEBUG
//    painter.setWorldMatrixEnabled(false);
//    painter.setPen(QPen(Qt::gray, 2));
//    painter.setBrush(Qt::NoBrush);
//    painter.drawRect(QRect(0, 0, width(), height()));
//#endif

    event->accept();
}

void ScribbleArea::paintSelectionVisuals(QPainter& painter)
{
    // outline of the transformed selection
    painter.setWorldMatrixEnabled(false);
    painter.setOpacity(1.0);
    mCurrentTransformSelection = mEditor->view()->getView().mapToPolygon(myTempTransformedSelection.toAlignedRect());
    mLastTransformSelection = mEditor->view()->getView().mapToPolygon(myTransformedSelection.toAlignedRect());

    Layer* layer = mEditor->layers()->currentLayer();
    if (layer != nullptr)
    {
        if (layer->type() == Layer::BITMAP)
        {
            painter.setBrush(Qt::NoBrush);

            QPen pen = QPen(Qt::DashLine);
            painter.setPen(pen);

            // Draw previous selection
            painter.drawPolygon(mLastTransformSelection.toPolygon());

            // Draw current selection
            painter.drawPolygon(mCurrentTransformSelection.toPolygon());

        }
        if (layer->type() == Layer::VECTOR)
        {
            painter.setBrush(QColor(0, 0, 0, 20));
            painter.setPen(Qt::gray);
            painter.drawPolygon(mCurrentTransformSelection);
        }

        if (layer->type() != Layer::VECTOR || currentTool()->type() != SELECT)
        {
            painter.setPen(Qt::SolidLine);
            painter.setBrush(QBrush(Qt::gray));
            int width = 6;
            int radius = width / 2;

            const QRectF topLeftCorner = QRectF(mCurrentTransformSelection[0].x() - radius,
                                                mCurrentTransformSelection[0].y() - radius,
                                                width, width);
            painter.drawRect(topLeftCorner);

            const QRectF topRightCorner = QRectF(mCurrentTransformSelection[1].x() - radius,
                                                 mCurrentTransformSelection[1].y() - radius,
                                                 width, width);
            painter.drawRect(topRightCorner);

            const QRectF bottomRightCorner = QRectF(mCurrentTransformSelection[2].x() - radius,
                                                    mCurrentTransformSelection[2].y() - radius,
                                                    width, width);
            painter.drawRect(bottomRightCorner);

            const QRectF bottomLeftCorner = QRectF(mCurrentTransformSelection[3].x() - radius,
                                                   mCurrentTransformSelection[3].y() - radius,
                                                   width, width);
            painter.drawRect(bottomLeftCorner);

            painter.setBrush(QColor(0, 255, 0, 50));
            painter.setPen(Qt::green);
        }
    }
}

BitmapImage* ScribbleArea::currentBitmapImage(Layer* layer) const
{
    Q_ASSERT(layer->type() == Layer::BITMAP);
    auto bitmapLayer = static_cast<LayerBitmap*>(layer);
    return bitmapLayer->getLastBitmapImageAtFrame(mEditor->currentFrame());
}

BitmapSurface* ScribbleArea::currentBitmapSurfaceImage(Layer* layer) const
{
    Q_ASSERT(layer->type() == Layer::BITMAP);
    auto bitmapLayer = static_cast<LayerBitmapSurface*>(layer);
    return bitmapLayer->getLastBitmapImageAtFrame(mEditor->currentFrame());
}

VectorImage* ScribbleArea::currentVectorImage(Layer* layer) const
{
    Q_ASSERT(layer->type() == Layer::VECTOR);
    auto vectorLayer = (static_cast<LayerVector*>(layer));
    return vectorLayer->getLastVectorImageAtFrame(mEditor->currentFrame(), 0);
}

//void ScribbleArea::drawCanvas(int frame, QRect rect)
//{
//    Object* object = mEditor->object();

//    CanvasPainterOptions o;
//    o.bPrevOnionSkin = mPrefs->isOn(SETTING::PREV_ONION);
//    o.bNextOnionSkin = mPrefs->isOn(SETTING::NEXT_ONION);
//    o.bColorizePrevOnion = mPrefs->isOn(SETTING::ONION_RED);
//    o.bColorizeNextOnion = mPrefs->isOn(SETTING::ONION_BLUE);
//    o.nPrevOnionSkinCount = mPrefs->getInt(SETTING::ONION_PREV_FRAMES_NUM);
//    o.nNextOnionSkinCount = mPrefs->getInt(SETTING::ONION_NEXT_FRAMES_NUM);
//    o.fOnionSkinMaxOpacity = mPrefs->getInt(SETTING::ONION_MAX_OPACITY);
//    o.fOnionSkinMinOpacity = mPrefs->getInt(SETTING::ONION_MIN_OPACITY);
//    o.bAntiAlias = mPrefs->isOn(SETTING::ANTIALIAS);
//    o.bGrid = mPrefs->isOn(SETTING::GRID);
//    o.nGridSizeW = mPrefs->getInt(SETTING::GRID_SIZE_W);
//    o.nGridSizeH = mPrefs->getInt(SETTING::GRID_SIZE_H);
//    o.bAxis = false;
//    o.bThinLines = mPrefs->isOn(SETTING::INVISIBLE_LINES);
//    o.bOutlines = mPrefs->isOn(SETTING::OUTLINES);
//    o.nShowAllLayers = mShowAllLayers;
//    o.bIsOnionAbsolute = (mPrefs->getString(SETTING::ONION_TYPE) == "absolute");
//    o.scaling = mEditor->view()->scaling();
//    o.onionWhilePlayback = mPrefs->getInt(SETTING::ONION_WHILE_PLAYBACK);
//    o.isPlaying = mEditor->playback()->isPlaying() ? true : false;
//    mCanvasPainter.setOptions(o);

//    mCanvasPainter.setCanvas(&mCanvas);

//    ViewManager* vm = mEditor->view();
//    mCanvasPainter.setViewTransform(vm->getView(), vm->getViewInverse());

//    mCanvasPainter.paint(object, mEditor->layers()->currentLayerIndex(), frame, rect);
//}

void ScribbleArea::setGaussianGradient(QGradient &gradient, QColor colour, qreal opacity, qreal offset)
{
    if (offset < 0) { offset = 0; }
    if (offset > 100) { offset = 100; }

    int r = colour.red();
    int g = colour.green();
    int b = colour.blue();
    qreal a = colour.alphaF();

    int mainColorAlpha = qRound(a * 255 * opacity);

    // the more feather (offset), the more softness (opacity)
    int alphaAdded = qRound((mainColorAlpha * offset) / 100);

    gradient.setColorAt(0.0, QColor(r, g, b, mainColorAlpha - alphaAdded));
    gradient.setColorAt(1.0, QColor(r, g, b, 0));
    gradient.setColorAt(1.0 - (offset / 100.0), QColor(r, g, b, mainColorAlpha - alphaAdded));
}

void ScribbleArea::loadMPBrush(const QByteArray &content)
{
    mMyPaint->loadBrush(content);
}

void ScribbleArea::newTileCreated(MPSurface *surface, MPTile *tile)
{
    updateTile(surface, tile);
}

void ScribbleArea::existingTileUpdated(MPSurface *surface, MPTile *tile)
{
    updateTile(surface, tile);
}

void ScribbleArea::onClearedSurface(MPSurface* surface)
{
    qDebug() << "surface cleared?";
}

void ScribbleArea::updateTile(MPSurface *surface, MPTile *tile)
{
    Q_UNUSED(surface);

    QPointF pos = tile->pos();

    MPTile *item = getTileFromPos(pos);
    item->setDirty(true);
    item->setImage(tile->image());

    item->update();
}

void ScribbleArea::clearSurfaceBuffer()
{
    QMutableHashIterator<QString, MPTile*> i(mTiles);
    while (i.hasNext()) {
        i.next();
        MPTile *tile = i.value();
        if (tile)
        {
            // Clear the content of the tile
            //
            tile->clear();

            // Removes blank tile from the scene for output optimization
            //
            // A tile without a scene is not re-created but onNewTile is
            // called when this tile is to be shown again.
            //
//            QGraphicsScene* scene = tile->scene();
//            if (scene) {
//                scene->removeItem(tile);
//                i.remove();
//            }
        }
    }
}

MPTile *ScribbleArea::getTileFromPos(QPointF point)
{
    QString posString = QString::number(point.x())+"_"+QString::number(point.y());
//    if (mTempTiles.contains(posString)) {
//        return mTempTiles.value(posString);
//    }
    if (mTiles.contains(posString)) {
        MPTile* tile = mTiles.value(posString);

        mTempTiles.insert(posString, tile);
        return tile;
    }
    else {
        qDebug() << "creating new tiles";
        QPixmap emptyImage = QPixmap(MYPAINT_TILE_SIZE, MYPAINT_TILE_SIZE);
        emptyImage.fill(Qt::transparent);

        MPTile *item = new MPTile(emptyImage);
        item->setPos(point);
//        item->setZValue(30);
//        mScene.addItem(item);
        mTiles.insert(posString, item);
        mTempTiles.insert(posString, item);


        return item;
    }
}

/************************************************************************************/
// Stroke Handling

void ScribbleArea::startStroke()
{
//    mBufferImg->clear();
//    mMyPaint->clearSurface();
//    Layer* layer = mEditor->layers()->currentLayer();
//    BitmapSurface* surfaceImage = currentBitmapSurfaceImage(layer);
//    mMyPaint->loadTiles(surfaceImage->pixmaps(), surfaceImage->tilePositions());

    // because we don't want to load mypaint
//    removeSurfaceBuffer();

    if (frameFirstLoad) {
        qDebug() << "frame first load";
        prepareForDrawing();
        frameFirstLoad = false;
    }
    mMyPaint->startStroke();
    isPainting = true;

}

void ScribbleArea::setBrushWidth(float width)
{
    mMyPaint->setBrushWidth(width);
}

void ScribbleArea::strokeTo(QPointF point, float pressure, float xtilt, float ytilt)
{
    point = mEditor->view()->mapScreenToCanvas(point);

    mMyPaint->strokeTo(point.x(), point.y(), pressure, xtilt, ytilt, mEditor->view()->scaling());

    // update dirty region
    updateDirtyTiles();
}

void ScribbleArea::updateDirtyTiles()
{
    QTransform v = mEditor->view()->getView();
    // update only dirty tiles
    for (MPTile* tile : mTempTiles) {
        if (tile->isDirty()) {
            QRectF mappedRect = v.mapRect(QRectF(tile->pos(), tile->boundingRect().size()));
            update(mappedRect.toRect());
            tile->setDirty(false);
        }
    }
}

void ScribbleArea::refreshSurface()
{
    clearSurfaceBuffer();
    mMyPaint->refreshSurface();
}

void ScribbleArea::endStroke()
{
    // clear the temp tiles buffer
    if (!mTempTiles.isEmpty()) {
        mTempTiles.clear();
    }
    isPainting = false;
}

//void ScribbleArea::drawPen(QPointF thePoint, qreal brushWidth, QColor fillColour, bool useAA)
//{
//    QRectF rectangle(thePoint.x() - 0.5 * brushWidth, thePoint.y() - 0.5 * brushWidth, brushWidth, brushWidth);

//    mBufferImg->drawEllipse(rectangle, Qt::NoPen, QBrush(fillColour, Qt::SolidPattern),
//                            QPainter::CompositionMode_Source, useAA);
//}

//void ScribbleArea::drawPencil(QPointF thePoint, qreal brushWidth, qreal fixedBrushFeather, QColor fillColour, qreal opacity)
//{
//    drawBrush(thePoint, brushWidth, fixedBrushFeather, fillColour, opacity, true);
//}

//void ScribbleArea::drawBrush(QPointF thePoint, qreal brushWidth, qreal mOffset, QColor fillColour, qreal opacity, bool usingFeather, int useAA)
//{
//    QRectF rectangle(thePoint.x() - 0.5 * brushWidth, thePoint.y() - 0.5 * brushWidth, brushWidth, brushWidth);

//    if (usingFeather)
//    {
//        QRadialGradient radialGrad(thePoint, 0.5 * brushWidth);
//        setGaussianGradient(radialGrad, fillColour, opacity, mOffset);

//        mBufferImg->drawEllipse(rectangle, Qt::NoPen, radialGrad,
//                                QPainter::CompositionMode_SourceOver, false);
//    }
//    else
//    {
//        mBufferImg->drawEllipse(rectangle, Qt::NoPen, QBrush(fillColour, Qt::SolidPattern),
//                                QPainter::CompositionMode_SourceOver, useAA);
//    }
//}

/**
 * @brief ScribbleArea::flipSelection
 * flip selection along the X or Y axis
*/
void ScribbleArea::flipSelection(bool flipVertical)
{
    int scaleX = myTempTransformedSelection.width() / mySelection.width();
    int scaleY = myTempTransformedSelection.height() / mySelection.height();
    QVector<QPoint> centerPoints = calcSelectionCenterPoints();

    QTransform translate = QTransform::fromTranslate(centerPoints[0].x(), centerPoints[0].y());
    QTransform _translate = QTransform::fromTranslate(-centerPoints[1].x(), -centerPoints[1].y());
    QTransform scale = QTransform::fromScale(-scaleX, scaleY);

    if (flipVertical)
    {
        scale = QTransform::fromScale(scaleX, -scaleY);
    }

    // reset transformation for vector selections
    selectionTransformation.reset();
    selectionTransformation *= _translate * scale * translate;

    paintTransformedSelection();
    applyTransformedSelection();
}

void ScribbleArea::blurBrush(BitmapImage *bmiSource_, QPointF srcPoint_, QPointF thePoint_, qreal brushWidth_, qreal mOffset_, qreal opacity_)
{
    QRadialGradient radialGrad(thePoint_, 0.5 * brushWidth_);
    setGaussianGradient(radialGrad, QColor(255, 255, 255, 127), opacity_, mOffset_);

    QRectF srcRect(srcPoint_.x() - 0.5 * brushWidth_, srcPoint_.y() - 0.5 * brushWidth_, brushWidth_, brushWidth_);
    QRectF trgRect(thePoint_.x() - 0.5 * brushWidth_, thePoint_.y() - 0.5 * brushWidth_, brushWidth_, brushWidth_);

    BitmapImage bmiSrcClip = bmiSource_->copy(srcRect.toRect());
    BitmapImage bmiTmpClip = bmiSrcClip; // TODO: find a shorter way

    bmiTmpClip.drawRect(srcRect, Qt::NoPen, radialGrad, QPainter::CompositionMode_Source, mPrefs->isOn(SETTING::ANTIALIAS));
    bmiSrcClip.bounds().moveTo(trgRect.topLeft().toPoint());
    bmiTmpClip.paste(&bmiSrcClip, QPainter::CompositionMode_SourceAtop);
    mBufferImg->paste(&bmiTmpClip);
}

void ScribbleArea::liquifyBrush(BitmapImage *bmiSource_, QPointF srcPoint_, QPointF thePoint_, qreal brushWidth_, qreal mOffset_, qreal opacity_)
{
    QPointF delta = (thePoint_ - srcPoint_); // increment vector
    QRectF trgRect(thePoint_.x() - 0.5 * brushWidth_, thePoint_.y() - 0.5 * brushWidth_, brushWidth_, brushWidth_);

    QRadialGradient radialGrad(thePoint_, 0.5 * brushWidth_);
    setGaussianGradient(radialGrad, QColor(255, 255, 255, 255), opacity_, mOffset_);

    // Create gradient brush
    BitmapImage bmiTmpClip;
    bmiTmpClip.drawRect(trgRect, Qt::NoPen, radialGrad, QPainter::CompositionMode_Source, mPrefs->isOn(SETTING::ANTIALIAS));

    // Slide texture/pixels of the source image
    qreal factor, factorGrad;

    for (int yb = bmiTmpClip.top(); yb < bmiTmpClip.bottom(); yb++)
    {
        for (int xb = bmiTmpClip.left(); xb < bmiTmpClip.right(); xb++)
        {
            QColor color;
            color.setRgba(bmiTmpClip.pixel(xb, yb));
            factorGrad = color.alphaF(); // any from r g b a is ok

            int xa = xb - factorGrad * delta.x();
            int ya = yb - factorGrad * delta.y();

            color.setRgba(bmiSource_->pixel(xa, ya));
            factor = color.alphaF();

            if (factor > 0.0)
            {
                color.setRed(color.red() / factor);
                color.setGreen(color.green() / factor);
                color.setBlue(color.blue() / factor);
                color.setAlpha(255); // Premultiplied color

                color.setRed(color.red()*factorGrad);
                color.setGreen(color.green()*factorGrad);
                color.setBlue(color.blue()*factorGrad);
                color.setAlpha(255 * factorGrad); // Premultiplied color

                bmiTmpClip.setPixel(xb, yb, color.rgba());
            }
            else
            {
                bmiTmpClip.setPixel(xb, yb, qRgba(255, 255, 255, 255));
            }
        }
    }
    mBufferImg->paste(&bmiTmpClip);
}

void ScribbleArea::drawPolyline(QPainterPath path, QPen pen, bool useAA)
{
    QRectF updateRect = mEditor->view()->mapCanvasToScreen(path.boundingRect().toRect()).adjusted(-1, -1, 1, 1);

    // Update region outside updateRect
    QRectF boundingRect = updateRect.adjusted(-width(), -height(), width(), height());
    mBufferImg->clear();
    mBufferImg->drawPath(path, pen, Qt::NoBrush, QPainter::CompositionMode_SourceOver, useAA);
    update(boundingRect.toRect());

}

/************************************************************************************/
// view handling

QRectF ScribbleArea::getCameraRect()
{
    return mCanvasPainter.getCameraRect();
}

QPointF ScribbleArea::getCentralPoint()
{
    return mEditor->view()->mapScreenToCanvas(QPointF(width() / 2, height() / 2));
}

/************************************************************************************/
// selection handling

void ScribbleArea::calculateSelectionRect()
{
    selectionTransformation.reset();
    Layer* layer = mEditor->layers()->currentLayer();
    if (layer == nullptr) { return; }
    if (layer->type() == Layer::VECTOR)
    {
        VectorImage *vectorImage = currentVectorImage(layer);
        vectorImage->calculateSelectionRect();
        setSelection(vectorImage->getSelectionRect());
    }
}

bool ScribbleArea::isSomethingSelected() const
{
    return mSomethingSelected;
}

void ScribbleArea::findMoveModeOfCornerInRange()
{
    const double marginInPixels = 15;
    const double scale = mEditor->view()->getView().inverted().m11();
    const double scaledMargin = fabs(marginInPixels * scale);

    // MAYBE: if this is broken, use myTransformedSelection
    QRectF transformRect = myTempTransformedSelection;
    QPointF lastPoint = currentTool()->getLastPoint();

    MoveMode mode;
    if (QLineF(lastPoint, transformRect.topLeft()).length() < scaledMargin)
    {
        mode = MoveMode::TOPLEFT;
    }
    else if (QLineF(lastPoint, transformRect.topRight()).length() < scaledMargin)
    {
        mode = MoveMode::TOPRIGHT;
    }
    else if (QLineF(lastPoint, transformRect.bottomLeft()).length() < scaledMargin)
    {
        mode = MoveMode::BOTTOMLEFT;

    }
    else if (QLineF(lastPoint, transformRect.bottomRight()).length() < scaledMargin)
    {
        mode = MoveMode::BOTTOMRIGHT;
    }
    else if (myTransformedSelection.contains(lastPoint))
    {
        mode = MoveMode::MIDDLE;
    }
    else {
        mode = MoveMode::NONE;
    }
    mMoveMode = mode;
}

MoveMode ScribbleArea::getMoveModeForSelectionAnchor()
{

    const double marginInPixels = 15;
    const double radius = marginInPixels / 2;
    const double scale = mEditor->view()->getView().inverted().m11();
    const double scaledMargin = fabs(marginInPixels * scale);

    if (mCurrentTransformSelection.isEmpty()) { return MoveMode::NONE; }

    QRectF topLeftCorner = mEditor->view()->mapScreenToCanvas(QRectF(mCurrentTransformSelection[0].x() - radius,
                                                                     mCurrentTransformSelection[0].y() - radius,
                                                                     marginInPixels, marginInPixels));

    QRectF topRightCorner = mEditor->view()->mapScreenToCanvas(QRectF(mCurrentTransformSelection[1].x() - radius,
                                                                      mCurrentTransformSelection[1].y() - radius,
                                                                      marginInPixels, marginInPixels));

    QRectF bottomRightCorner = mEditor->view()->mapScreenToCanvas(QRectF(mCurrentTransformSelection[2].x() - radius,
                                                                         mCurrentTransformSelection[2].y() - radius,
                                                                         marginInPixels, marginInPixels));

    QRectF bottomLeftCorner = mEditor->view()->mapScreenToCanvas(QRectF(mCurrentTransformSelection[3].x() - radius,
                                                                        mCurrentTransformSelection[3].y() - radius,
                                                                        marginInPixels, marginInPixels));

    QPointF currentPos = currentTool()->getCurrentPoint();

    if (QLineF(currentPos, topLeftCorner.center()).length() < scaledMargin)
    {
        return MoveMode::TOPLEFT;
    }
    else if (QLineF(currentPos, topRightCorner.center()).length() < scaledMargin)
    {
        return MoveMode::TOPRIGHT;
    }
    else if (QLineF(currentPos, bottomLeftCorner.center()).length() < scaledMargin)
    {
        return MoveMode::BOTTOMLEFT;

    }
    else if (QLineF(currentPos, bottomRightCorner.center()).length() < scaledMargin)
    {
        return MoveMode::BOTTOMRIGHT;
    }
    else if (myTempTransformedSelection.contains(currentPos))
    {
        return MoveMode::MIDDLE;
    }

    return MoveMode::NONE;
}

QPointF ScribbleArea::whichAnchorPoint(QPointF anchorPoint)
{
    MoveMode mode = getMoveModeForSelectionAnchor();
    if (mode == MoveMode::TOPLEFT)
    {
        anchorPoint = mySelection.bottomRight();
    }
    else if (mode == MoveMode::TOPRIGHT)
    {
        anchorPoint = mySelection.bottomLeft();
    }
    else if (mode == MoveMode::BOTTOMLEFT)
    {
        anchorPoint = mySelection.topRight();
    }
    else if (mode == MoveMode::BOTTOMRIGHT)
    {
        anchorPoint = mySelection.topLeft();
    }
    return anchorPoint;
}

void ScribbleArea::adjustSelection(float offsetX, float offsetY, qreal rotatedAngle)
{
    QRectF& transformedSelection = myTransformedSelection;

    switch (mMoveMode)
    {
    case MoveMode::MIDDLE:
    {
        myTempTransformedSelection =
            transformedSelection.translated(QPointF(offsetX, offsetY));

        break;
    }
    case MoveMode::TOPRIGHT:
    {
        myTempTransformedSelection =
            transformedSelection.adjusted(0, offsetY, offsetX, 0);

        break;
    }
    case MoveMode::TOPLEFT:
    {
        myTempTransformedSelection =
            transformedSelection.adjusted(offsetX, offsetY, 0, 0);

        break;
    }
    case MoveMode::BOTTOMLEFT:
    {
        myTempTransformedSelection =
            transformedSelection.adjusted(offsetX, 0, 0, offsetY);
        break;
    }
    case MoveMode::BOTTOMRIGHT:
    {
        myTempTransformedSelection =
            transformedSelection.adjusted(0, 0, offsetX, offsetY);
        break;

    }
    case MoveMode::ROTATION:
    {
        myTempTransformedSelection =
            transformedSelection; // @ necessary?
        myRotatedAngle = (currentTool()->getCurrentPixel().x() -
                          currentTool()->getLastPressPixel().x()) + rotatedAngle;
        break;
    }
    default:
        break;
    }
    update();
}

/**
 * @brief ScribbleArea::calculateSelectionCenter
 * @return QPoint of tempTransformSelection center at [0] and selection center at [1]
 */
QVector<QPoint> ScribbleArea::calcSelectionCenterPoints()
{
    QVector<QPoint> centerPoints;
    float selectionCenterX,
        selectionCenterY,
        tempSelectionCenterX,
        tempSelectionCenterY;

    tempSelectionCenterX = myTempTransformedSelection.center().x();
    tempSelectionCenterY = myTempTransformedSelection.center().y();
    selectionCenterX = mySelection.center().x();
    selectionCenterY = mySelection.center().y();
    centerPoints.append(QPoint(tempSelectionCenterX, tempSelectionCenterY));
    centerPoints.append(QPoint(selectionCenterX, selectionCenterY));
    return centerPoints;
}

void ScribbleArea::calculateSelectionTransformation()
{
    QVector<QPoint> centerPoints = calcSelectionCenterPoints();

    selectionTransformation.reset();

    selectionTransformation.translate(centerPoints[0].x(), centerPoints[0].y());
    selectionTransformation.rotate(myRotatedAngle);

    if (mySelection.width() > 0 && mySelection.height() > 0) // can't divide by 0
    {
        float scaleX = myTempTransformedSelection.width() / mySelection.width();
        float scaleY = myTempTransformedSelection.height() / mySelection.height();
        selectionTransformation.scale(scaleX, scaleY);
    }
    selectionTransformation.translate(-centerPoints[1].x(), -centerPoints[1].y());
}


void ScribbleArea::paintTransformedSelection()
{
    Layer* layer = mEditor->layers()->currentLayer();
    if (layer == nullptr)
    {
        return;
    }

    if (mSomethingSelected)    // there is something selected
    {
        if (layer->type() == Layer::BITMAP)
        {
            mCanvasPainter.setTransformedSelection(mySelection.toAlignedRect(), selectionTransformation);
        }
        else if (layer->type() == Layer::VECTOR)
        {
            // vector transformation
            VectorImage* vectorImage = currentVectorImage(layer);
            vectorImage->setSelectionTransformation(selectionTransformation);

        }
        setModified(mEditor->layers()->currentLayerIndex(), mEditor->currentFrame());
    }
//    update();
}

void ScribbleArea::applySelectionChanges()
{
    // we haven't applied our last modifications yet
    // therefore apply the transformed selection first.
    applyTransformedSelection();

    // make sure the current transformed selection is valid
    if (!myTempTransformedSelection.isValid())
    {
        myTempTransformedSelection = myTempTransformedSelection.normalized();
    }
    setSelection(myTempTransformedSelection);
    paintTransformedSelection();

    // Calculate the new transformation based on the new selection
    calculateSelectionTransformation();

    // apply the transformed selection to make the selection modification absolute.
    applyTransformedSelection();

}

void ScribbleArea::applyTransformedSelection()
{
    mCanvasPainter.ignoreTransformedSelection();

    Layer* layer = mEditor->layers()->currentLayer();
    if (layer == nullptr)
    {
        return;
    }

    if (mSomethingSelected)
    {
        if (mySelection.isEmpty()) { return; }

        if (layer->type() == Layer::BITMAP)
        {
            BitmapImage* bitmapImage = currentBitmapImage(layer);
            BitmapImage transformedImage = bitmapImage->transformed(mySelection.toRect(), selectionTransformation, true);

            bitmapImage->clear(mySelection);
            bitmapImage->paste(&transformedImage, QPainter::CompositionMode_SourceOver);
        }
        else if (layer->type() == Layer::VECTOR)
        {
            VectorImage* vectorImage = currentVectorImage(layer);
            vectorImage->applySelectionTransformation();

        }

        setModified(mEditor->layers()->currentLayerIndex(), mEditor->currentFrame());
    }

    updateCurrentFrame();
}

void ScribbleArea::cancelTransformedSelection()
{
    mCanvasPainter.ignoreTransformedSelection();

    if (mSomethingSelected)
    {
        Layer* layer = mEditor->layers()->currentLayer();
        if (layer == nullptr) { return; }

        if (layer->type() == Layer::VECTOR) {

            VectorImage* vectorImage = currentVectorImage(layer);
            vectorImage->setSelectionTransformation(QTransform());
        }

        setSelection(mySelection);

        selectionTransformation.reset();

        mOffset.setX(0);
        mOffset.setY(0);

        updateCurrentFrame();
    }
}

void ScribbleArea::setSelection(QRectF rect)
{
    Layer* layer = mEditor->layers()->currentLayer();

    if (layer->type() == Layer::BITMAP)
    {
        rect = rect.toAlignedRect();
    }
    mySelection = rect;
    myTransformedSelection = rect;
    myTempTransformedSelection = rect;
    mSomethingSelected = (mySelection.isNull() ? false : true);


    // Temporary disabled this as it breaks selection rotate key (ctrl) event.
    // displaySelectionProperties();
}

/**
 * @brief ScribbleArea::manageSelectionOrigin
 * switches anchor point when crossing threshold
 */
void ScribbleArea::manageSelectionOrigin(QPointF currentPoint, QPointF originPoint)
{
    int mouseX = currentPoint.x();
    int mouseY = currentPoint.y();

    QRectF selectRect;

    if (mouseX <= originPoint.x())
    {
        selectRect.setLeft(mouseX);
        selectRect.setRight(originPoint.x());
    }
    else
    {
        selectRect.setLeft(originPoint.x());
        selectRect.setRight(mouseX);
    }

    if (mouseY <= originPoint.y())
    {
        selectRect.setTop(mouseY);
        selectRect.setBottom(originPoint.y());
    }
    else
    {
        selectRect.setTop(originPoint.y());
        selectRect.setBottom(mouseY);
    }

    if (currentTool()->type() == ToolType::SELECT) {
        myTempTransformedSelection = selectRect;
    }

}

void ScribbleArea::displaySelectionProperties()
{
    Layer* layer = mEditor->layers()->currentLayer();
    if (layer == nullptr) { return; }
    if (layer->type() == Layer::VECTOR)
    {
        VectorImage* vectorImage = currentVectorImage(layer);
        //vectorImage->applySelectionTransformation();
        if (currentTool()->type() == MOVE)
        {
            int selectedCurve = vectorImage->getFirstSelectedCurve();
            if (selectedCurve != -1)
            {
                mEditor->tools()->setWidth(vectorImage->curve(selectedCurve).getWidth());
                mEditor->tools()->setFeather(vectorImage->curve(selectedCurve).getFeather());
                mEditor->tools()->setInvisibility(vectorImage->curve(selectedCurve).isInvisible());
                mEditor->tools()->setPressure(vectorImage->curve(selectedCurve).getVariableWidth());
                mEditor->color()->setColorNumber(vectorImage->curve(selectedCurve).getColourNumber());
            }

            int selectedArea = vectorImage->getFirstSelectedArea();
            if (selectedArea != -1)
            {
                mEditor->color()->setColorNumber(vectorImage->mArea[selectedArea].mColourNumber);
            }
        }
    }
}

void ScribbleArea::selectAll()
{
    mOffset.setX(0);
    mOffset.setY(0);
    Layer* layer = mEditor->layers()->currentLayer();

    Q_ASSERT(layer);
    if (layer == nullptr) { return; }

    if (layer->type() == Layer::BITMAP)
    {
        // Selects the drawn area (bigger or smaller than the screen). It may be more accurate to select all this way
        // as the drawing area is not limited
        BitmapImage *bitmapImage = currentBitmapImage(layer);
        setSelection(bitmapImage->bounds());
    }
    else if (layer->type() == Layer::VECTOR)
    {
        VectorImage *vectorImage = currentVectorImage(layer);
        vectorImage->selectAll();
        setSelection(vectorImage->getSelectionRect());
    }
    updateCurrentFrame();
}

/**
 * @brief ScribbleArea::resetSelectionProperties
 * should be used whenever translate, rotate, transform, scale
 * has been applied to a selection, but don't want to reset size nor position
 */
void ScribbleArea::resetSelectionProperties()
{
    mOffset = QPoint(0, 0);
    myRotatedAngle = 0;
    selectionTransformation.reset();
}

void ScribbleArea::deselectAll()
{
    resetSelectionProperties();
    mySelection = QRectF();
    myTransformedSelection = QRectF();
    myTempTransformedSelection = QRectF();

    Layer* layer = mEditor->layers()->currentLayer();
    if (layer == nullptr) { return; }
    if (layer->type() == Layer::VECTOR)
    {
        currentVectorImage(layer)->deselectAll();
    }
    mSomethingSelected = false;

    mBufferImg->clear();

    //mBitmapSelection.clear();
    vectorSelection.clear();

    // clear all the data tools may have accumulated
    editor()->tools()->cleanupAllToolsData();

    updateCurrentFrame();
}

void ScribbleArea::toggleThinLines()
{
    bool previousValue = mPrefs->isOn(SETTING::INVISIBLE_LINES);
    setEffect(SETTING::INVISIBLE_LINES, !previousValue);
}

void ScribbleArea::toggleOutlines()
{
    mIsSimplified = !mIsSimplified;
    setEffect(SETTING::OUTLINES, mIsSimplified);
}

void ScribbleArea::toggleShowAllLayers()
{
    mShowAllLayers++;
    if (mShowAllLayers == 3)
    {
        mShowAllLayers = 0;
    }
    updateAllFrames();
}

/************************************************************************************/
// tool handling

BaseTool* ScribbleArea::currentTool()
{
    return editor()->tools()->currentTool();
}

BaseTool* ScribbleArea::getTool(ToolType eToolType)
{
    return editor()->tools()->getTool(eToolType);
}

// TODO: check this method
void ScribbleArea::setCurrentTool(ToolType eToolMode)
{
    if (currentTool() != nullptr && eToolMode != currentTool()->type())
    {
        qDebug() << "Set Current Tool" << BaseTool::TypeName(eToolMode);
        if (BaseTool::TypeName(eToolMode) == "")
        {
            // tool does not exist
            //Q_ASSERT_X( false, "", "" );
            return;
        }

        if (currentTool()->type() == MOVE)
        {
            paintTransformedSelection();
            deselectAll();
        }
        else if (currentTool()->type() == POLYLINE)
        {
            deselectAll();
        }
    }

    mPrevToolType = currentTool()->type();

    // change cursor
    setCursor(currentTool()->cursor());
    updateCanvasCursor();
    //qDebug() << "fn: setCurrentTool " << "call: setCursor()" << "current tool" << currentTool()->typeName();
}

void ScribbleArea::setTemporaryTool(ToolType eToolMode)
{
    // Only switch to temporary tool if not already in this state
    // and temporary tool is not already the current tool.
    if (!mInstantTool && currentTool()->type() != eToolMode)
    {
        mInstantTool = true; // used to return to previous tool when finished (keyRelease).
        mPrevTemporalToolType = currentTool()->type();
        editor()->tools()->setCurrentTool(eToolMode);
    }
}

void ScribbleArea::deleteSelection()
{
    if (mSomethingSelected)      // there is something selected
    {
        Layer* layer = mEditor->layers()->currentLayer();
        if (layer == nullptr) { return; }

        mEditor->backup(tr("Delete Selection", "Undo Step: clear the selection area."));

        mClosestCurves.clear();
        if (layer->type() == Layer::VECTOR) { currentVectorImage(layer)->deleteSelection(); }
        if (layer->type() == Layer::BITMAP) { currentBitmapImage(layer)->clear(mySelection); }
        updateAllFrames();
    }
}

void ScribbleArea::clearCanvas()
{
    Layer* layer = mEditor->layers()->currentLayer();
    if (layer == nullptr) { return; }

    if (layer->type() == Layer::VECTOR)
    {
//        mEditor->backup(tr("Clear Image", "Undo step text"));

        currentVectorImage(layer)->clear();
        mClosestCurves.clear();
        mClosestVertices.clear();
    }
    else if (layer->type() == Layer::BITMAP)
    {
//        mEditor->backup(tr("Clear Image", "Undo step text"));
        currentBitmapSurfaceImage(layer)->clear();
        mMyPaint->clearSurface();
        refreshSurface();
        update();
    }
    else
    {
        return; // skip updates when nothing changes
    }
    setModified(mEditor->layers()->currentLayerIndex(), mEditor->currentFrame());
}

void ScribbleArea::setPrevTool()
{
    editor()->tools()->setCurrentTool(mPrevTemporalToolType);
    mInstantTool = false;
}

// TODO: move somewhere else... scribblearea is bloated enough already.
void ScribbleArea::paletteColorChanged(QColor color)
{
    mMyPaint->setBrushColor(color);
    updateAllVectorLayersAtCurrentFrame();
}


void ScribbleArea::floodFillError(int errorType)
{
    QString message, error;
    if (errorType == 1) { message = tr("There is a gap in your drawing (or maybe you have zoomed too much)."); }
    if (errorType == 2 || errorType == 3) message = tr("Sorry! This doesn't always work."
                                                       "Please try again (zoom a bit, click at another location... )<br>"
                                                       "if it doesn't work, zoom a bit and check that your paths are connected by pressing F1.).");

    if (errorType == 1) { error = tr("Out of bound."); }
    if (errorType == 2) { error = tr("Could not find a closed path."); }
    if (errorType == 3) { error = tr("Could not find the root index."); }
    QMessageBox::warning(this, tr("Flood fill error"), tr("%1<br><br>Error: %2").arg(message).arg(error), QMessageBox::Ok, QMessageBox::Ok);
    deselectAll();
}
