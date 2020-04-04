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

#include "stroketool.h"

#include "scribblearea.h"
#include "strokemanager.h"
#include "viewmanager.h"
#include "layermanager.h"
#include "colormanager.h"
#include "selectionmanager.h"

#include "beziercurve.h"
#include "vectorimage.h"


#include "editor.h"
#include "qmath.h"

#ifdef Q_OS_MAC
extern "C" {
    void detectWhichOSX();
    void disableCoalescing();
    void enableCoalescing();
}
#else
extern "C" {
    void detectWhichOSX() {}
    void disableCoalescing() {}
    void enableCoalescing() {}
}
#endif

StrokeTool::StrokeTool(QObject* parent) : BaseTool(parent)
{
    detectWhichOSX();
}

void StrokeTool::startStroke()
{
    mEditor->backup(typeName());
    if (emptyFrameActionEnabled())
    {
        mScribbleArea->handleDrawingOnEmptyFrame();
    }

    mScribbleArea->startStroke();
    mScribbleArea->setBrushWidth(static_cast<float>(qLn(properties.width)));

    mFirstDraw = true;
    mLastPixel = getCurrentPixel();

    mStrokePoints.clear();

    //Experimental
    QPointF startStrokes = strokeManager()->interpolateStart(mLastPixel);
    mStrokePoints << mEditor->view()->mapScreenToCanvas(startStrokes);

    mStrokePressures.clear();
    mStrokePressures << strokeManager()->getPressure();

    disableCoalescing();
}

bool StrokeTool::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Alt:
        mScribbleArea->setTemporaryTool(EYEDROPPER);
        return true;
    case Qt::Key_Space:
        mScribbleArea->setTemporaryTool(HAND); // just call "setTemporaryTool()" to activate temporarily any tool
        return true;
    }
    return false;
}

bool StrokeTool::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
    return true;
}

bool StrokeTool::emptyFrameActionEnabled()
{
    return true;
}

void StrokeTool::endStroke()
{
    Layer* layer = mEditor->layers()->currentLayer();
    qreal distance = QLineF(getCurrentPixel(), mMouseDownPoint).length();
    if (distance < 1) { isBrushDab = true; } else { isBrushDab = false; }

    if (layer->type() == Layer::BITMAP)
        paintBitmapStroke();
    else if (layer->type() == Layer::VECTOR)
        paintVectorStroke();

    strokeManager()->interpolateEnd();
    mStrokePressures << strokeManager()->getPressure();
    mStrokePoints.clear();
    mStrokePressures.clear();

    enableCoalescing();

    mScribbleArea->endStroke();
}

void StrokeTool::drawStroke(const QPointF pos)
{
    const QPointF pixel = pos;

    const float pressure = static_cast<float>(mCurrentPressure);
    mScribbleArea->strokeTo(pixel, pressure, mCurrentXTilt,  mCurrentYTilt);

    if ( pixel != mLastPixel || !mFirstDraw )
    {
        mLastPixel = pixel;
        mStrokePoints << mEditor->view()->mapScreenToCanvas( pixel );
        mStrokePressures << strokeManager()->getPressure();
    }
    else
    {
        mFirstDraw = false;
    }
}

void StrokeTool::drawStroke()
{
    QPointF pixel = getCurrentPoint();

    drawStroke(pixel);
}


void StrokeTool::paintBitmapStroke()
{
    mScribbleArea->paintBitmapBuffer(QPainter::CompositionMode_Source);
    mScribbleArea->setAllDirty();
    mScribbleArea->clearBitmapBuffer();
}

// This function uses the points from DrawStroke
// and turns them into vector lines.
void StrokeTool::paintVectorStroke()
{
    if (mStrokePoints.empty())
        return;

    Layer* layer = mEditor->layers()->currentLayer();

    if (layer->type() == Layer::VECTOR && mStrokePoints.size() > -1)
    {
        // Clear the temporary pixel path
        mScribbleArea->clearBitmapBuffer();
        qreal tol = mScribbleArea->getCurveSmoothing() / mEditor->view()->scaling();

        BezierCurve curve(mStrokePoints, mStrokePressures, tol);
        curve.setWidth(properties.width);
        curve.setFeather(properties.feather);
        curve.setFilled(false);
        curve.setInvisibility(properties.invisibility);
        curve.setVariableWidth(properties.pressure);
        curve.setColourNumber(mEditor->color()->frontColorNumber());

        VectorImage* vectorImage = static_cast<VectorImage*>(layer->getLastKeyFrameAtPosition(mEditor->currentFrame()));
        vectorImage->addCurve(curve, mEditor->view()->scaling(), false);

        if (vectorImage->isAnyCurveSelected() || mEditor->select()->somethingSelected())
        {
            mEditor->deselectAll();
        }

        vectorImage->setSelected(vectorImage->getLastCurveNumber(), true);

        mScribbleArea->setModified(mEditor->layers()->currentLayerIndex(), mEditor->currentFrame());
        mScribbleArea->setAllDirty();
    }
}
