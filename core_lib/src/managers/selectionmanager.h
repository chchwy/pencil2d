#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include "basemanager.h"
#include "movemode.h"
#include "vertexref.h"
#include "vectorselection.h"

#include <QPointF>
#include <QRectF>
#include <QPolygonF>
#include <QTransform>

class Editor;

class SelectionManager : public BaseManager
{
    Q_OBJECT
public:
    explicit SelectionManager(Editor* editor);
    ~SelectionManager() override;

    bool init() override;
    Status load(Object*) override;
    Status save(Object*) override;
    void workingLayerChanged(Layer*) override;

    QVector<QPoint> calcSelectionCenterPoints();

    void update();

    QRectF mappedSelection();

    QPointF whichAnchorPoint(QPointF anchorPoint);
    QPointF getTransformOffset() { return mOffset; }
    QPointF getCurrentOffset();
    QPointF maintainAspectRatio(qreal offsetX, qreal offsetY);

    void flipSelection(bool flipVertical);
    void selectAll();
    void deselectAll();

    void setSelection(QRectF rect);
    void controlOffsetOrigin(QPointF currentPoint, QPointF anchorPoint);

    MoveMode getMoveModeForSelectionAnchor();
    MoveMode getMoveMode() const { return mMoveMode; }
    void setMoveMode(MoveMode moveMode) { mMoveMode = moveMode; }

    bool somethingSelected() const { return mSomethingSelected; }

    void calculateSelectionTransformation();
    void calculateSelectionRect();
    void adjustSelection(float offsetX, float offsetY, qreal rotatedAngle);
    void manageSelectionOrigin(QPointF currentPoint, QPointF originPoint);
    void findMoveModeOfCornerInRange();
    void setCurves(QList<int> curves) { mClosestCurves = curves; }
    QList<int> closestCurves() { return mClosestCurves; }
    QList<VertexRef> closestVertices() { return mClosestVertices; }

    QTransform selectionTransform() { return mSelectionTransform; }
    void setSelectionTransform(QTransform transform) { mSelectionTransform = transform; }
    void resetSelectionTransform();

    inline bool transformHasBeenModified() { return (mySelection != myTempTransformedSelection) || myRotatedAngle != 0; }

    /** @brief TransformProxyTool::resetSelectionProperties
     * should be used whenever translate, rotate, transform, scale
     * has been applied to a selection, but don't want to reset size nor position
     */
    void resetSelectionProperties();
    void deleteSelection();

    bool shouldDeselect();


    QPolygonF currentSelectionPolygonF() const { return mCurrentSelectionPolygonF; }
    QPolygonF lastSelectionPolygonF() const { return mLastSelectionPolygonF; }

    QRectF mySelection;
    QRectF myTempTransformedSelection;
    QRectF myTransformedSelection;

    int myRotatedAngle;
    qreal selectionTolerance = 8.0;

    VectorSelection vectorSelection;


private:

    bool mSomethingSelected;
    QPolygonF mLastSelectionPolygonF;
    QPolygonF mCurrentSelectionPolygonF;
    QPointF mOffset;

    QList<int> mClosestCurves;
    QList<VertexRef> mClosestVertices;

    MoveMode mMoveMode;

    QTransform mSelectionTransform;
};

#endif // SELECTIONMANAGER_H
