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
#include "selectionmanager.h"

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
    deltaTimer.invalidate();
}

bool ScribbleArea::init()
{
    mPrefs = mEditor->preference();
    mDoubleClickTimer = new QTimer(this);
    mMyPaint = MPHandler::handler();

    connect(mMyPaint, &MPHandler::newTile, this, &ScribbleArea::updateTile);
    connect(mMyPaint, &MPHandler::updateTile, this, &ScribbleArea::updateTile);
    connect(mMyPaint, &MPHandler::clearedSurface, this, &ScribbleArea::onClearedSurface);

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

    connect(mEditor->select(), &SelectionManager::selectionChanged, this, &ScribbleArea::updateCurrentFrame);
    connect(mEditor->select(), &SelectionManager::needPaintAndApply, this, &ScribbleArea::applySelectionChanges);
    connect(mEditor->select(), &SelectionManager::needDeleteSelection, this, &ScribbleArea::deleteSelection);

    mDoubleClickTimer->setInterval(50);

    const int curveSmoothingLevel = mPrefs->getInt(SETTING::CURVE_SMOOTHING);
    mCurveSmoothingLevel = curveSmoothingLevel / 20.0; // default value is 1.0

    mQuickSizing = mPrefs->isOn(SETTING::QUICK_SIZING);
    mMakeInvisible = false;

    mIsSimplified = mPrefs->isOn(SETTING::OUTLINES);
    mMultiLayerOnionSkin = mPrefs->isOn(SETTING::MULTILAYER_ONION);

    mBufferImg = new BitmapImage;

    updateCanvasCursor();

    setMouseTracking(true); // reacts to mouse move events, even if the button is not pressed

#if QT_VERSION >= 0x50900

    // tablet tracking first added in 5.9
    setTabletTracking(true);

#endif

    mDebugRect = QRectF(0, 0, 0, 0);

    QPixmapCache::setCacheLimit(100 * 1024); // unit is kb, so it's 100MB cache

    int nLength = mEditor->layers()->animationLength();
    mPixmapCacheKeys.resize(static_cast<unsigned>(std::max(nLength, 240)));

    mNeedUpdateAll = false;
    deltaTimer.start();

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

void ScribbleArea::prepareForDrawing()
{
    qDebug() << "prepare for drawing";

    // This loads an image into mypaint backend
    // should be used with caution, highly dependent on surface since.
    // if possible only load when frame changes.
    Layer* layer = mEditor->layers()->currentLayer();

    switch(layer->type()) {
        case Layer::BITMAP:
        {
            BitmapImage* bitmapImage = currentBitmapImage(layer);
            const QImage* image = bitmapImage->image();
            mMyPaint->loadImage(*image, bitmapImage->topLeft());
            break;
        }
        case Layer::VECTOR:
        {
            Q_ASSERT("Not implemented for vector");
            break;
        }
        default:
            break;
    }
}

void ScribbleArea::showBitmapFrame(Layer* layer)
{
//    BitmapImage* bitmapImage = currentBitmapImage(layer);

//    if (bitmapImage->isModified()) {
//        qDebug() << "keyframe has been modified";
//        bitmapImage->setModified(false);

//    } else {
//        frameFirstLoad = true;
//    }

    drawCanvas(mEditor->currentFrame());
}

void ScribbleArea::showCurrentFrame()
{
    Layer* layer = mEditor->layers()->currentLayer();

    mMyPaint->clearSurface();

    switch (layer->type())
    {
        case Layer::BITMAP:
        {
            showBitmapFrame(layer);
            break;
        }
        case Layer::VECTOR:
        {
            break;
        }
        default:
        {
            break;
        }
    }
    update();
}

void ScribbleArea::drawCanvas(int frame)
{
    Object* object = mEditor->object();

    QPainter painter(this);
    painter.setClipping(true);
    painter.setClipRect(this->rect());

    QHash<QString, MPTile*> tilesToBeRendered;

    if (mIsPainting) {
        tilesToBeRendered = mBufferTiles;
    }

//    qDebug() << "bufferTiles " << mBufferTiles.size();
//    qDebug() << tilesToBeRendered.size();

    mCanvasPainter.setOptions( getRenderOptions() );
    mCanvasPainter.setCanvas( &mCanvas );
    mCanvasPainter.setViewTransform( mEditor->view()->getView());
    mCanvasPainter.paint(painter, object, mEditor->layers()->currentLayerIndex(), frame, tilesToBeRendered.values(), mIsPainting);

    // Cache current frame for faster render
    //
    QString cachedFrameKey = getCachedFrameKey( frame );

    QPixmap pm;
    if (QPixmapCache::find(cachedFrameKey, &pm)) {
        QPixmapCache::remove( cachedFrameKey );
    }

    QPixmapCache::insert( cachedFrameKey, mCanvas );
}

CanvasPainterOptions ScribbleArea::getRenderOptions()
{
    CanvasPainterOptions o;
    o.prevOnionSkinEnabled = mPrefs->isOn(SETTING::PREV_ONION);
    o.nextOnionSkinEnabled = mPrefs->isOn(SETTING::NEXT_ONION);
    o.prevColoredOnionSkinEnabled = mPrefs->isOn(SETTING::ONION_RED);
    o.nextColoredOnionSkinEnabled = mPrefs->isOn(SETTING::ONION_BLUE);
    o.prevOnionSkinCount = mPrefs->getInt(SETTING::ONION_PREV_FRAMES_NUM);
    o.nextOnionSkinCount = mPrefs->getInt(SETTING::ONION_NEXT_FRAMES_NUM);
    o.onionSkinMaxOpacity = mPrefs->getInt(SETTING::ONION_MAX_OPACITY);
    o.onionSkinMinOpacity = mPrefs->getInt(SETTING::ONION_MIN_OPACITY);
    o.antiAliasingEnabled = mPrefs->isOn(SETTING::ANTIALIAS);
    o.gridEnabld = mPrefs->isOn(SETTING::GRID);
    o.gridSizeW = mPrefs->getInt(SETTING::GRID_SIZE_W);
    o.gridSizeH = mPrefs->getInt(SETTING::GRID_SIZE_H);
    o.axisEnabled = false;
    o.vectorOptions.thinLinesEnabled = mPrefs->isOn(SETTING::INVISIBLE_LINES);
    o.vectorOptions.outlineEnabled = mPrefs->isOn(SETTING::OUTLINES);
    o.showLayersCount = mShowAllLayers;
    o.onionSkinAbsoluteEnabled = (mPrefs->getString(SETTING::ONION_TYPE) == "absolute");
    o.zoomLevel = mEditor->view()->scaling();
    o.onionWhilePlayback = mPrefs->getInt(SETTING::ONION_WHILE_PLAYBACK);
    o.isPlaying = mEditor->playback()->isPlaying() ? true : false;

    return o;
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
        showCurrentFrame();
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

    auto selectMan = mEditor->select();
    bool isSomethingSelected = selectMan->somethingSelected();
    if (isSomethingSelected) {
        keyEventForSelection(event);
    } else {
        keyEvent(event);
    }
}

void ScribbleArea::keyEventForSelection(QKeyEvent* event)
{
    auto selectMan = mEditor->select();
    switch (event->key())
    {
    case Qt::Key_Right:
        selectMan->translate(QPointF(1, 0));
        paintTransformedSelection();
        break;
    case Qt::Key_Left:
        selectMan->translate(QPointF(-1, 0));
        paintTransformedSelection();
        break;
    case Qt::Key_Up:
        selectMan->translate(QPointF(0, -1));
        paintTransformedSelection();
        break;
    case Qt::Key_Down:
        selectMan->translate(QPointF(0, 1));
        paintTransformedSelection();
        break;
    case Qt::Key_Return:
        applyTransformedSelection();
        mEditor->deselectAll();
        break;
    case Qt::Key_Escape:
        mEditor->deselectAll();
        cancelTransformedSelection();
        break;
    case Qt::Key_Backspace:
        deleteSelection();
        mEditor->deselectAll();
        break;
    case Qt::Key_Space:
        setTemporaryTool(HAND); // just call "setTemporaryTool()" to activate temporarily any tool
        break;
    default:
        event->ignore();
    }
}

void ScribbleArea::keyEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Right:
        mEditor->scrubForward();
        event->ignore();
        break;
    case Qt::Key_Left:
        mEditor->scrubBackward();
        event->ignore();
        break;
    case Qt::Key_Up:
        mEditor->layers()->gotoNextLayer();
        event->ignore();
        break;
    case Qt::Key_Down:
        mEditor->layers()->gotoPreviouslayer();
        event->ignore();
        break;
    case Qt::Key_Return:
        event->ignore();
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

    Layer* layer = mEditor->layers()->currentLayer();
    if (layer->type() == Layer::CAMERA && !layer->visible())
    {
        showLayerNotVisibleWarning(); // FIXME: crash when using tablets
        return;
    }

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

    event->accept();

    update();
//    updateCanvasCursor();
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
    // set deltatime per press to avoid last stroke bleeding...
    deltaTime = 1.f;

    if (event->button() == Qt::RightButton)
    {
        mMouseRightButtonInUse = true;
        getTool(HAND)->pointerPressEvent(event);
        return;
    }

    bool isCameraLayer = mEditor->layers()->currentLayer()->type() == Layer::CAMERA;
    if ((currentTool()->type() != HAND || isCameraLayer) && (event->button() != Qt::RightButton) && (event->button() != Qt::MidButton || isCameraLayer))
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

    if (mEditor->view()->transformUpdated()) {
        mEditor->view()->transformUpdatedState(false);
        update();
    }

    if (event->buttons() & (Qt::LeftButton | Qt::RightButton))
    {

        // --- use SHIFT + drag to resize WIDTH / use CTRL + drag to resize FEATHER ---
        if (currentTool()->isAdjusting())
        {
            currentTool()->adjustCursor(event->modifiers());
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
    updateCanvasCursor();
}

void ScribbleArea::pointerReleaseEvent(PointerEvent* event)
{
    if (currentTool()->isAdjusting())
    {
        currentTool()->stopAdjusting();
        mEditor->tools()->setWidth(static_cast<float>(currentTool()->properties.width));
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
    Layer* layer = mEditor->layers()->currentLayer();
    if (layer == nullptr) { return false; }

    return layer->type() == Layer::BITMAP || layer->type() == Layer::VECTOR;
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
    mMouseInUse = (e->buttons() & Qt::RightButton) || (e->buttons() & Qt::LeftButton);
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

    mMyPaint->setSurfaceSize(newSize);
    mEditor->view()->setCanvasSize( newSize );

    QWidget::resizeEvent( event );
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

void ScribbleArea::showLayerNotVisibleWarning()
{
    QMessageBox::warning(this, tr("Warning"),
                         tr("You are trying to modify a hidden layer! Please select another layer (or make the current layer visible)."),
                         QMessageBox::Ok,
                         QMessageBox::Ok);
}

void ScribbleArea::paintBitmapBuffer()
{
    LayerBitmap* layer = static_cast<LayerBitmap*>(mEditor->layers()->currentLayer());
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
    BitmapImage* targetImage = currentBitmapImage(layer);
    if (targetImage == nullptr) { return; }

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

    // adds content from canvas and saves to bitmapimage
    for (MPTile* item : mBufferTiles.values()) {
        QPixmap tilePixmap = item->pixmap();
        targetImage->paste(tilePixmap, item->pos(), cm);
    }

    layer->setModified(frameNumber, true);
    emit modification(frameNumber);
}

void ScribbleArea::clearBitmapBuffer()
{
    mBufferImg->clear();
}

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

void ScribbleArea::paintCanvasCursor()
{
    QPainter painter(this);

    QTransform view = mEditor->view()->getView();
    QPointF mousePos = currentTool()->getCurrentPoint();
    int centerCal = mCursorImg.width() / 2;

    mTransformedCursorPos = view.map(mousePos);

    // reset matrix
//    view.reset();

//    painter.setTransform(view);
    mCursorCenterPos.setX(centerCal);
    mCursorCenterPos.setY(centerCal);

    painter.drawPixmap(QPoint(static_cast<int>(mTransformedCursorPos.x() - mCursorCenterPos.x()),
                              static_cast<int>(mTransformedCursorPos.y() - mCursorCenterPos.y())),
                       mCursorImg);

//    painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);

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
        mCursorImg = currentTool()->quickSizeCursor(static_cast<float>(brushWidth), static_cast<float>(brushFeather), scalingFac);
    }
    else if (mEditor->preference()->isOn(SETTING::DOTTED_CURSOR))
    {
        bool useFeather = currentTool()->properties.useFeather;
        mCursorImg = currentTool()->canvasCursor(static_cast<float>(brushWidth), static_cast<float>(brushFeather), useFeather, scalingFac, width());
    }
    else
    {
        mCursorImg = QPixmap(); // if above does not comply, deallocate image
    }

    // update cursor rect
    QPoint translatedPos = QPoint(static_cast<int>(mTransformedCursorPos.x() - mCursorCenterPos.x()),
                                  static_cast<int>(mTransformedCursorPos.y() - mCursorCenterPos.y()));

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
        case KEEP_DRAWING_ON_PREVIOUS_KEY:
        {
            if (previousKeyFrame == nullptr) {
                mEditor->addNewKey();
            }
            break;
        }
        case DUPLICATE_PREVIOUS_KEY:
        {
            if (previousKeyFrame)
            {
                KeyFrame* dupKey = previousKeyFrame->clone();
                layer->addKeyFrame(frameNumber, dupKey);
                mEditor->scrubTo(frameNumber);
                break;
            }
            // if the previous keyframe doesn't exist,
            // fallthrough and create empty keyframe
        }
        case CREATE_NEW_KEY:
            mEditor->addNewKey();

            // Refresh canvas
            drawCanvas(frameNumber);
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


//    paintCanvasCursor();
    drawCanvas(mEditor->currentFrame());
    paintSelectionAnchors();

//    paintCanvasCursor(painter);

//    painter.setCompositionMode(-QPainter::CompositionMode_DestinationOut);
//    updateCanvasCursor();

    //        // paints the selection outline
    //        if (mSomethingSelected && !myTempTransformedSelection.isNull())
    //        {
    //        }

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

void ScribbleArea::paintSelectionAnchors()
{
    QPainter painter(this);

    Object* object = mEditor->object();

    auto selectMan = mEditor->select();
    selectMan->updatePolygons();

    if (!selectMan->somethingSelected()) { return; }
    if (selectMan->currentSelectionPolygonF().isEmpty()) { return; }
    if (selectMan->currentSelectionPolygonF().count() < 4) { return; }

    QPolygonF lastSelectionPolygon = editor()->view()->mapPolygonToScreen(selectMan->lastSelectionPolygonF());
    QPolygonF currentSelectionPolygon = editor()->view()->mapPolygonToScreen(selectMan->currentSelectionPolygonF());

    TransformParameters params = { lastSelectionPolygon, currentSelectionPolygon };
    mSelectionPainter.paint(painter, object, mEditor->currentLayerIndex(), currentTool(), params);
}

//void ScribbleArea::paintVectorAnchors()
//{
//    QPainter painter(this);

//    Object* object = mEditor->object();

//    VectorPainterParameters params;

////    mTransformPainter.paint(painter, object, mEditor->currentLayerIndex(), currentTool(), params);
//}

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
    mMyPaint->refreshSurface();
}

void ScribbleArea::newTileCreated(MPSurface *, MPTile *)
{
//    updateTile(surface, tile);
}

void ScribbleArea::existingTileUpdated(MPSurface *, MPTile *)
{
//    updateTile(surface, tile);
}

void ScribbleArea::onClearedSurface(MPSurface* surface)
{
    Q_UNUSED(surface)
//    qDebug() << "surface cleared?";
}

void ScribbleArea::updateTile(MPSurface *surface, MPTile *tile)
{
    Q_UNUSED(surface)

    QPointF pos = tile->pos();

    tile->setDirty(true);
    mBufferTiles.insert(QString::number(pos.x())+"_"+QString::number(pos.y()), tile);
}

/************************************************************************************/
// Stroke Handling

void ScribbleArea::startStroke()
{

//    if (frameFirstLoad) {
//        qDebug() << "frame first load";
        prepareForDrawing();
        frameFirstLoad = false;
//    }
    mMyPaint->startStroke();
    mIsPainting = true;

    qDebug() << "start stroke";

}

void ScribbleArea::setBrushWidth(float width)
{
    mMyPaint->setBrushWidth(width);
}

void ScribbleArea::strokeTo(QPointF point, float pressure, float xtilt, float ytilt)
{
    point = mEditor->view()->mapScreenToCanvas(point);

    mMyPaint->strokeTo(static_cast<float>(point.x()), static_cast<float>(point.y()), pressure, xtilt, ytilt, calculateDeltaTime());

    // update dirty region
    updateDirtyTiles();
}

QColor ScribbleArea::pickColorFromSurface(QPointF point, int radius)
{

    return mMyPaint->getSurfaceColor(static_cast<float>(point.x()), static_cast<float>(point.y()), radius);
}

void ScribbleArea::updateDirtyTiles()
{
    QTransform v = mEditor->view()->getView();
    QHashIterator<QString, MPTile*> i(mBufferTiles);
    while (i.hasNext()) {
        i.next();
        MPTile* tile = i.value();
        const QPointF& tilePos = tile->pos();
        const QSizeF& tileSize = tile->boundingRect().size();
        if (tile->isDirty()) {

            const QRectF& mappedRect = v.mapRect(QRectF(tilePos, tileSize));
            update(mappedRect.toRect());

            tile->setDirty(false);
            mBufferTiles.insert(i.key(), i.value());
        }
    }
}

void ScribbleArea::refreshSurface()
{
    mMyPaint->refreshSurface();
}

void ScribbleArea::endStroke()
{
    // clear the temp tiles buffer
    if (!mBufferTiles.isEmpty()) {
        mBufferTiles.clear();
    }
    mIsPainting = false;
    mMyPaint->endStroke();
    qDebug() << "end stroke";
}

void ScribbleArea::flipSelection(bool flipVertical)
{
    mEditor->select()->flipSelection(flipVertical);
    paintTransformedSelection();
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

void ScribbleArea::paintTransformedSelection()
{
    Layer* layer = mEditor->layers()->currentLayer();
    if (layer == nullptr)
    {
        return;
    }

    auto selectMan = mEditor->select();
    if (selectMan->somethingSelected())    // there is something selected
    {
        if (layer->type() == Layer::BITMAP)
        {
            mCanvasPainter.setTransformedSelection(selectMan->mySelectionRect().toRect(), selectMan->myTempTransformedSelectionRect().toRect(), selectMan->selectionTransform());
        }
        else if (layer->type() == Layer::VECTOR)
        {
            // vector transformation
            VectorImage* vectorImage = currentVectorImage(layer);
            vectorImage->setSelectionTransformation(selectMan->selectionTransform());

        }
        setModified(mEditor->layers()->currentLayerIndex(), mEditor->currentFrame());
    }
    update();
}

void ScribbleArea::applySelectionChanges()
{
    // we haven't applied our last modifications yet
    // therefore apply the transformed selection first.
    applyTransformedSelection();


    //// might not be needed anymore...
    auto selectMan = mEditor->select();

    // make sure the current transformed selection is valid
    if (!selectMan->myTempTransformedSelectionRect().isValid())
    {
        const QRectF& normalizedRect = selectMan->myTempTransformedSelectionRect().normalized();
        selectMan->setTempTransformedSelectionRect(normalizedRect);
    }
    selectMan->setSelection(selectMan->myTempTransformedSelectionRect());
    paintTransformedSelection();

    // Calculate the new transformation based on the new selection
    selectMan->calculateSelectionTransformation();

    // apply the transformed selection to make the selection modification absolute.
    applyTransformedSelection();
    //// end

}

void ScribbleArea::applyTransformedSelection()
{
    mCanvasPainter.ignoreTransformedSelection();

    Layer* layer = mEditor->layers()->currentLayer();
    if (layer == nullptr)
    {
        return;
    }

    auto selectMan = mEditor->select();
    if (selectMan->somethingSelected())
    {
        if (selectMan->mySelectionRect().isEmpty()) { return; }

        if (layer->type() == Layer::BITMAP)
        {
            BitmapImage* bitmapImage = currentBitmapImage(layer);
            bitmapImage->moveSelectionTransform(selectMan->mySelectionRect().toRect(),
                                   selectMan->selectionTransform());
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

    auto selectMan = mEditor->select();
    if (selectMan->somethingSelected())
    {
        Layer* layer = mEditor->layers()->currentLayer();
        if (layer == nullptr) { return; }

        if (layer->type() == Layer::VECTOR) {

            VectorImage* vectorImage = currentVectorImage(layer);
            vectorImage->setSelectionTransformation(QTransform());
        }

        mEditor->select()->setSelection(selectMan->mySelectionRect());

        selectMan->resetSelectionProperties();

        updateCurrentFrame();
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
            mEditor->deselectAll();
        }
        else if (currentTool()->type() == POLYLINE)
        {
            mEditor->deselectAll();
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
    auto selectMan = mEditor->select();
    if (selectMan->somethingSelected())      // there is something selected
    {
        Layer* layer = mEditor->layers()->currentLayer();
        if (layer == nullptr) { return; }

////        mEditor->backup(tr("Delete Selection", "Undo Step: clear the selection area."));

        selectMan->clearCurves();
        if (layer->type() == Layer::VECTOR) { currentVectorImage(layer)->deleteSelection(); }
        if (layer->type() == Layer::BITMAP) { currentBitmapImage(layer)->clear(selectMan->mySelectionRect()); }
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
        mEditor->select()->clearCurves();
        mEditor->select()->clearVertices();
    }
    else if (layer->type() == Layer::BITMAP)
    {
//        mEditor->backup(tr("Clear Image", "Undo step text"));
        currentBitmapImage(layer)->clear();
    }
    else
    {
        return; // skip updates when nothing changes
    }

    setModified(mEditor->layers()->currentLayerIndex(), mEditor->currentFrame());
    mMyPaint->clearSurface();
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

//void ScribbleArea::brushSettingChanged(MyPaintBrushSetting setting)
//{
//    mMyPaint->setB
//}

qreal ScribbleArea::calculateDeltaTime()
{
    deltaTime = deltaTimer.nsecsElapsed() * 1e-9;
    deltaTimer.restart();
    return deltaTime;
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
    mEditor->deselectAll();
}
