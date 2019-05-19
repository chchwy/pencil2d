#include "selectionmanager.h"
#include "editor.h"

#include "layermanager.h"
#include "basetool.h"
#include "toolmanager.h"
#include "viewmanager.h"
#include "scribblearea.h"

#include "qpainter.h"

//#ifdef QT_DEBUG
#include <QDebug>
//#endif


SelectionManager::SelectionManager(Editor* editor) : BaseManager(editor),
    mSomethingSelected(false), mySelection(QRectF()), myTempTransformedSelection(QRectF()), myTransformedSelection(QRectF()),
    mLastSelectionPolygonF(QPolygonF()),mCurrentSelectionPolygonF(QPolygonF()),
    mOffset(QPointF()), myRotatedAngle(0), mMoveMode(MoveMode::NONE)
{
}

SelectionManager::~SelectionManager()
{
}

bool SelectionManager::init()
{
    return true;
}

Status SelectionManager::load(Object*)
{
    return Status::OK;
}

Status SelectionManager::save(Object*)
{
    return Status::OK;
}

void SelectionManager::workingLayerChanged(Layer *)
{
}

void SelectionManager::resetSelectionProperties()
{
    mOffset = QPoint(0, 0);
    myRotatedAngle = 0;
    mSelectionTransform.reset();
}


void SelectionManager::update()
{
    mOffset = getCurrentOffset();
    mCurrentSelectionPolygonF = editor()->view()->mapPolygonToScreen(myTempTransformedSelection.toRect());
    mLastSelectionPolygonF = editor()->view()->mapPolygonToScreen(myTransformedSelection.toRect());
}

void SelectionManager::resetSelectionTransform()
{
    mSelectionTransform.reset();
}

QPointF SelectionManager::getCurrentOffset()
{
    BaseTool* tool = editor()->tools()->currentTool();
    return tool->getCurrentPoint() - tool->getCurrentPressPoint();
}

bool SelectionManager::shouldDeselect()
{
    BaseTool* tool = editor()->tools()->currentTool();
    return (!myTransformedSelection.contains(tool->getCurrentPoint())
            && getMoveMode() == MoveMode::NONE);
}

void SelectionManager::deleteSelection()
{
    ScribbleArea* scribbleArea = editor()->getScribbleArea();
    scribbleArea->deleteSelection();
}

void SelectionManager::findMoveModeOfCornerInRange()
{
    BaseTool* currentTool = editor()->tools()->currentTool();
    const double marginInPixels = 15;
    const double scale = editor()->view()->getView().inverted().m11();
    const double scaledMargin = qAbs(marginInPixels * scale);

    // MAYBE: if this is broken, use myTransformedSelection
    QRectF transformRect = myTempTransformedSelection;
    QPointF lastPoint = currentTool->getLastPoint();

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

//    qDebug("%i", mode);
    mMoveMode = mode;
}

void SelectionManager::controlOffsetOrigin(QPointF currentPoint, QPointF anchorPoint)
{
    QPointF offset = mOffset;
    if (getMoveMode() != MoveMode::NONE)
    {
        if (editor()->layers()->currentLayer()->type() == Layer::BITMAP) {
            offset = QPointF(mOffset).toPoint();
        }

        adjustSelection(mOffset.x(), mOffset.y(), myRotatedAngle);
    }
    else
    {
        // when the selection is none, manage the selection Origin
        manageSelectionOrigin(currentPoint, anchorPoint);
    }
}

MoveMode SelectionManager::getMoveModeForSelectionAnchor()
{

    BaseTool* currentTool = editor()->tools()->currentTool();
    const double marginInPixels = 15;
    const double radius = marginInPixels / 2;
    const double scale = editor()->view()->getView().inverted().m11();
    const double scaledMargin = qAbs(marginInPixels * scale);

    qDebug() << mCurrentSelectionPolygonF;
    if (mCurrentSelectionPolygonF.count() < 4) { return MoveMode::NONE; }


    QRectF topLeftCorner = editor()->view()->mapScreenToCanvas(QRectF(mCurrentSelectionPolygonF[0].x() - radius,
                                                                     mCurrentSelectionPolygonF[0].y() - radius,
                                                                     marginInPixels, marginInPixels));

    QRectF topRightCorner = editor()->view()->mapScreenToCanvas(QRectF(mCurrentSelectionPolygonF[1].x() - radius,
                                                                      mCurrentSelectionPolygonF[1].y() - radius,
                                                                      marginInPixels, marginInPixels));

    QRectF bottomRightCorner = editor()->view()->mapScreenToCanvas(QRectF(mCurrentSelectionPolygonF[2].x() - radius,
                                                                         mCurrentSelectionPolygonF[2].y() - radius,
                                                                         marginInPixels, marginInPixels));

    QRectF bottomLeftCorner = editor()->view()->mapScreenToCanvas(QRectF(mCurrentSelectionPolygonF[3].x() - radius,
                                                                        mCurrentSelectionPolygonF[3].y() - radius,
                                                                        marginInPixels, marginInPixels));

    QPointF currentPos = currentTool->getCurrentPoint();

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

QPointF SelectionManager::whichAnchorPoint(QPointF anchorPoint)
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

void SelectionManager::adjustSelection(float offsetX, float offsetY, qreal rotatedAngle)
{

    qDebug() << offsetX;
    qDebug() << offsetY;
    BaseTool* currentTool = editor()->tools()->currentTool();
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
        myRotatedAngle = (currentTool->getCurrentPixel().x() -
                          currentTool->getLastPressPixel().x()) + rotatedAngle;
        break;
    }
    default:
        break;
    }
}


void SelectionManager::setSelection(QRectF rect)
{
    Layer* layer = editor()->layers()->currentLayer();

    if (layer->type() == Layer::BITMAP)
    {
        rect = rect.toRect();
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
void SelectionManager::manageSelectionOrigin(QPointF currentPoint, QPointF originPoint)
{
    BaseTool* currentTool = editor()->tools()->currentTool();
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

    if (currentTool->type() == ToolType::SELECT) {
        myTempTransformedSelection = selectRect;
    }
}

void SelectionManager::calculateSelectionTransformation()
{
    QVector<QPoint> centerPoints = calcSelectionCenterPoints();

    mSelectionTransform.reset();

    mSelectionTransform.translate(centerPoints[0].x(), centerPoints[0].y());
    mSelectionTransform.rotate(myRotatedAngle);

    if (mySelection.width() > 0 && mySelection.height() > 0) // can't divide by 0
    {
        float scaleX = myTempTransformedSelection.width() / mySelection.width();
        float scaleY = myTempTransformedSelection.height() / mySelection.height();
        mSelectionTransform.scale(scaleX, scaleY);
    }
    mSelectionTransform.translate(-centerPoints[1].x(), -centerPoints[1].y());
}

QVector<QPoint> SelectionManager::calcSelectionCenterPoints()
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


QPointF SelectionManager::maintainAspectRatio(qreal offsetX, qreal offsetY)
{
    qreal factor = myTransformedSelection.width() / myTransformedSelection.height();

    if (mMoveMode == MoveMode::TOPLEFT || mMoveMode == MoveMode::BOTTOMRIGHT)
    {
        offsetY = offsetX / factor;
    }
    else if (mMoveMode == MoveMode::TOPRIGHT || mMoveMode == MoveMode::BOTTOMLEFT)
    {
        offsetY = -(offsetX / factor);
    }
    else if (mMoveMode == MoveMode::MIDDLE)
    {
        qreal absX = offsetX;
        if (absX < 0) { absX = -absX; }

        qreal absY = offsetY;
        if (absY < 0) { absY = -absY; }

        if (absX > absY) { offsetY = 0; }
        if (absY > absX) { offsetX = 0; }
    }
    return QPointF(offsetX, offsetY);
}

void SelectionManager::deselectAll()
{
    resetSelectionProperties();
    mySelection = QRectF();
    myTransformedSelection = QRectF();
    myTempTransformedSelection = QRectF();
    mCurrentSelectionPolygonF = QPolygonF();
    mLastSelectionPolygonF = QPolygonF();

    Layer* layer = editor()->layers()->currentLayer();
    if (layer == nullptr) { return; }
//    if (layer->type() == Layer::VECTOR)
//    {
//        currentVectorImage(layer)->deselectAll();
//    }
    mSomethingSelected = false;

//    mBufferImg->clear();

    //mBitmapSelection.clear();
//    vectorSelection.clear();

    // clear all the data tools may have accumulated
    editor()->tools()->cleanupAllToolsData();

    // Update cursor
//    setCursor(currentTool()->cursor());

//    updateCurrentFrame();
}

/**
 * @brief ScribbleArea::flipSelection
 * flip selection along the X or Y axis
*/
void SelectionManager::flipSelection(bool flipVertical)
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
    mSelectionTransform.reset();
    mSelectionTransform *= _translate * scale * translate;

//    paintTransformedSelection();
//    applyTransformedSelection();
}

void SelectionManager::calculateSelectionRect()
{
    mSelectionTransform.reset();
    Layer* layer = editor()->layers()->currentLayer();
    if (layer == nullptr) { return; }
    if (layer->type() == Layer::VECTOR)
    {
//        VectorImage *vectorImage = currentVectorImage(layer);
//        vectorImage->calculateSelectionRect();
//        setSelection(vectorImage->getSelectionRect());
    }
}

void SelectionManager::selectAll()
{
    mOffset.setX(0);
    mOffset.setY(0);
    Layer* layer = editor()->layers()->currentLayer();

    Q_ASSERT(layer);
    if (layer == nullptr) { return; }

    if (layer->type() == Layer::BITMAP)
    {
        // Selects the drawn area (bigger or smaller than the screen). It may be more accurate to select all this way
        // as the drawing area is not limited
//        BitmapImage *bitmapImage = currentBitmapImage(layer);
//        setSelection(bitmapImage->bounds());
    }
    else if (layer->type() == Layer::VECTOR)
    {
//        VectorImage *vectorImage = currentVectorImage(layer);
//        vectorImage->selectAll();
//        setSelection(vectorImage->getSelectionRect());
    }
//    updateCurrentFrame();
}

