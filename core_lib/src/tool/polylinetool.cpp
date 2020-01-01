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

#include "polylinetool.h"


#include "editor.h"
#include "scribblearea.h"
#include "mphandler.h"

#include "strokemanager.h"
#include "layermanager.h"
#include "colormanager.h"
#include "viewmanager.h"
#include "pointerevent.h"
#include "layervector.h"
#include "layerbitmap.h"
#include "vectorimage.h"


PolylineTool::PolylineTool(QObject* parent) : StrokeTool(parent)
{
}

ToolType PolylineTool::type()
{
    return POLYLINE;
}

void PolylineTool::loadSettings()
{
    mPropertyEnabled[WIDTH] = true;
    mPropertyEnabled[BEZIER] = true;
    mPropertyEnabled[ANTI_ALIASING] = true;

    QSettings settings(PENCIL2D, PENCIL2D);

    properties.width = settings.value("polyLineWidth", 8.0).toDouble();
    properties.feather = -1;
    properties.pressure = false;
    properties.invisibility = OFF;
    properties.preserveAlpha = OFF;
    properties.useAA = settings.value("brushAA").toBool();
    properties.stabilizerLevel = -1;
}

void PolylineTool::resetToDefault()
{
    setWidth(8.0);
    setBezier(false);
}

void PolylineTool::setWidth(const qreal width)
{
    // Set current property
    properties.width = width;

    // Update settings
    QSettings settings(PENCIL2D, PENCIL2D);
    settings.setValue("polyLineWidth", width);
    settings.sync();
}

void PolylineTool::setFeather(const qreal feather)
{
    Q_UNUSED(feather)
    properties.feather = -1;
}

void PolylineTool::setAA(const int AA)
{
    // Set current property
    properties.useAA = AA;

    // Update settings
    QSettings settings(PENCIL2D, PENCIL2D);
    settings.setValue("brushAA", AA);
    settings.sync();
}

QCursor PolylineTool::cursor()
{
    return Qt::CrossCursor;
}

void PolylineTool::clearToolData()
{
    mPoints.clear();
}

void PolylineTool::pointerPressEvent(PointerEvent* event)
{
    Layer* layer = mEditor->layers()->currentLayer();

    mScribbleArea->mMyPaint->clearSurface();
    mEditor->backup(typeName());

    if (event->button() == Qt::LeftButton)
    {
        if (layer->type() == Layer::BITMAP || layer->type() == Layer::VECTOR)
        {
            mScribbleArea->handleDrawingOnEmptyFrame();

            if (layer->type() == Layer::VECTOR)
            {
                ((LayerVector *)layer)->getLastVectorImageAtFrame(mEditor->currentFrame(), 0)->deselectAll();
                if (mScribbleArea->makeInvisible() && !mEditor->preference()->isOn(SETTING::INVISIBLE_LINES))
                {
                    mScribbleArea->toggleThinLines();
                }
            }

            if (!mPoints.isEmpty()) {

                for(int i=0; i < mPoints.size(); i++) {
                    drawStroke(mPoints[i]);
                }

                mPoints.takeAt(0);
            }

            if (previousPoint.isNull()) {
                mPoints << getCurrentPoint();
            } else {
                mPoints << previousPoint;
            }
        }
    }

    mScribbleArea->mMyPaint->startStroke();
    mScribbleArea->setIsPainting(true);

    mScribbleArea->paintBitmapBuffer(QPainter::CompositionMode_SourceOver);

    mScribbleArea->clearTilesBuffer();
}

void PolylineTool::pointerMoveEvent(PointerEvent*)
{
    Layer* layer = mEditor->layers()->currentLayer();

    if (layer->type() == Layer::BITMAP || layer->type() == Layer::VECTOR)
    {
        drawPolyline(mPoints, getCurrentPoint());
    }
    previousPoint = getCurrentPoint();
}

void PolylineTool::pointerReleaseEvent(PointerEvent *)
{
}

void PolylineTool::pointerDoubleClickEvent(PointerEvent*)
{
    // include the current point before ending the line.
    mPoints << getCurrentPoint();

    endPolyline(mPoints);
    clearToolData();
}


bool PolylineTool::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Return:
        if (mPoints.size() > 0)
        {
            endPolyline(mPoints);
            clearToolData();
            return true;
        }
        break;

    case Qt::Key_Escape:
        if (mPoints.size() > 0)
        {
            cancelPolyline();
            clearToolData();
            return true;
        }
        break;

    default:
        return false;
    }

    return false;
}

void PolylineTool::drawPolyline(QList<QPointF> points, QPointF endPoint)
{
    if (points.size() > 0)
    {
        mScribbleArea->mMyPaint->clearSurface();
        mScribbleArea->mMyPaint->startStroke();
        for(int i=0; i<points.size(); i++) {
            drawStroke(points[i]);
        }
        drawStroke(endPoint);
        mScribbleArea->updateCurrentFrame();
        mScribbleArea->mMyPaint->endStroke();


        // Vector otherwise
//        if (layer->type() == Layer::VECTOR)
//        {
//            if (mEditor->layers()->currentLayer()->type() == Layer::VECTOR)
//            {
//                tempPath = mEditor->view()->mapCanvasToScreen(tempPath);
//                if (mScribbleArea->makeInvisible() == true)
//                {
//                    pen.setWidth(0);
//                    pen.setStyle(Qt::DotLine);
//                }
//                else
//                {
//                    pen.setWidth(properties.width * mEditor->view()->scaling());
//                }
//            }
//        }

//        mScribbleArea->drawPolyline(tempPath, pen, properties.useAA);
    }
}


void PolylineTool::cancelPolyline()
{
    // Clear the in-progress polyline from the bitmap buffer.
    mScribbleArea->clearBitmapBuffer();
    mScribbleArea->updateCurrentFrame();
}

void PolylineTool::endPolyline(QList<QPointF> points)
{
    Layer* layer = mEditor->layers()->currentLayer();

    if (layer->type() == Layer::VECTOR)
    {
        BezierCurve curve = BezierCurve(points);
        if (mScribbleArea->makeInvisible() == true)
        {
            curve.setWidth(0);
        }
        else
        {
            curve.setWidth(properties.width);
        }
        curve.setColourNumber(mEditor->color()->frontColorNumber());
        curve.setVariableWidth(false);
        curve.setInvisibility(mScribbleArea->makeInvisible());

        ((LayerVector *)layer)->getLastVectorImageAtFrame(mEditor->currentFrame(), 0)->addCurve(curve, mEditor->view()->scaling());
    }
    if (layer->type() == Layer::BITMAP)
    {
        drawPolyline(points, points.last());
        mScribbleArea->paintBitmapBuffer(QPainter::CompositionMode_SourceOver);
    }
    mScribbleArea->setModified(mEditor->layers()->currentLayerIndex(), mEditor->currentFrame());

    mScribbleArea->prepareForDrawing();
    mScribbleArea->clearBitmapBuffer();
    mScribbleArea->endStroke();
}
