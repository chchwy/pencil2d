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
#include "selecttool.h"
#include "pointerevent.h"
#include "vectorimage.h"
#include "editor.h"
#include "strokemanager.h"
#include "layervector.h"
#include "scribblearea.h"
#include "layermanager.h"
#include "toolmanager.h"
#include "selectionmanager.h"

SelectTool::SelectTool(QObject* parent) : BaseTool(parent)
{
}

void SelectTool::loadSettings()
{
    properties.width = -1;
    properties.feather = -1;
    properties.stabilizerLevel = -1;
    properties.useAA = -1;
}

QCursor SelectTool::cursor()
{
    MoveMode mode = mEditor->select()->getMoveModeForSelectionAnchor();
    BaseTool* tool = mEditor->tools()->currentTool();
    return tool->selectMoveCursor(mode, type());
}

void SelectTool::beginSelection()
{
    // Store original click position for help with selection rectangle.
    mAnchorOriginPoint = getLastPoint();

    auto selectMan = mEditor->select();
    selectMan->calculateSelectionTransformation();

    // paint and apply the transformation
    mScribbleArea->paintTransformedSelection();
    mScribbleArea->applyTransformedSelection();

    if (selectMan->somethingSelected()) // there is something selected
    {
        if (mCurrentLayer->type() == Layer::VECTOR)
        {
            static_cast<LayerVector*>(mCurrentLayer)->getLastVectorImageAtFrame(mEditor->currentFrame(), 0)->deselectAll();
        }

        selectMan->findMoveModeOfCornerInRange();
        mAnchorOriginPoint = selectMan->whichAnchorPoint(mAnchorOriginPoint);

        // the user did not click on one of the corners
        if (selectMan->getMoveMode() == MoveMode::NONE)
        {
            selectMan->mySelection.setTopLeft(getLastPoint());
            selectMan->mySelection.setBottomRight(getLastPoint());
        }
    }
    else
    {
        selectMan->setSelection(QRectF(getCurrentPoint().x(), getCurrentPoint().y(),1,1));
    }
    mScribbleArea->update();
}

void SelectTool::pointerPressEvent(PointerEvent* event)
{
    mCurrentLayer = mEditor->layers()->currentLayer();
    if (mCurrentLayer == nullptr) return;
    if (!mCurrentLayer->isPaintable()) { return; }
    if (event->button() != Qt::LeftButton) { return; }
    auto selectMan = mEditor->select();

    selectMan->update();

    beginSelection();
}

void SelectTool::pointerMoveEvent(PointerEvent* event)
{
    mCurrentLayer = mEditor->layers()->currentLayer();
    if (mCurrentLayer == nullptr) { return; }
    if (!mCurrentLayer->isPaintable()) { return; }
    auto selectMan = mEditor->select();

    if (!selectMan->somethingSelected()) { return; }

    selectMan->update();

    mScribbleArea->updateToolCursor();

    if (mScribbleArea->isPointerInUse())
    {
        selectMan->controlOffsetOrigin(getCurrentPoint(), mAnchorOriginPoint);

        if (mCurrentLayer->type() == Layer::VECTOR)
        {
            static_cast<LayerVector*>(mCurrentLayer)->
                    getLastVectorImageAtFrame(mEditor->currentFrame(), 0)->
                    select(selectMan->myTempTransformedSelection);
        }
    }

    mScribbleArea->updateCurrentFrame();
}

void SelectTool::pointerReleaseEvent(PointerEvent* event)
{
    mCurrentLayer = mEditor->layers()->currentLayer();
    if (mCurrentLayer == nullptr) return;
    if (event->button() != Qt::LeftButton) return;
    auto selectMan = mEditor->select();

    // if there's a small very small distance between current and last point
    // discard the selection...
    // TODO: improve by adding a timer to check if the user is deliberately selecting
    if (QLineF(mAnchorOriginPoint, getCurrentPoint()).length() < 5.0)
    {
        selectMan->deselectAll();
    }
    if (maybeDeselect())
    {
        selectMan->deselectAll();
    }
    else
    {
        keepSelection();
    }

    selectMan->update();

    mScribbleArea->updateToolCursor();
    mScribbleArea->updateCurrentFrame();
//    mScribbleArea->setAllDirty();
}

bool SelectTool::maybeDeselect()
{
    return (!isSelectionPointValid() && mEditor->select()->getMoveMode() == MoveMode::NONE);
}

/**
 * @brief SelectTool::keepSelection
 * Keep selection rect and normalize if invalid
 */
void SelectTool::keepSelection()
{
    auto selectMan = mEditor->select();
    if (mCurrentLayer->type() == Layer::BITMAP) {
        if (!selectMan->myTempTransformedSelection.isValid())
        {
            selectMan->setSelection(selectMan->myTempTransformedSelection.normalized());
        }
        else
        {
            selectMan->setSelection(selectMan->myTempTransformedSelection);
        }
    }
    else if (mCurrentLayer->type() == Layer::VECTOR)
    {
        VectorImage* vectorImage = static_cast<LayerVector*>(mCurrentLayer)->getLastVectorImageAtFrame(mEditor->currentFrame(), 0);
        selectMan->setSelection(vectorImage->getSelectionRect());
    }
}

bool SelectTool::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Alt:
        mScribbleArea->setTemporaryTool(MOVE);
        break;
    default:
        break;
    }

    // Follow the generic behaviour anyway
    return false;
}
