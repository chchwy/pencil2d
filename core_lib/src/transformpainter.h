#ifndef TRANSFORMPAINTER_H
#define TRANSFORMPAINTER_H

#include <QRectF>
#include <QPolygonF>
#include <QTransform>
#include "vectorselection.h"

class QPainter;
class Object;
class BaseTool;
class Layer;
class VectorImage;

struct TransformParameters
{
    QPolygonF lastSelectionPolygonF;
    QPolygonF currentSelectionPolygonF;

    TransformParameters&operator =(TransformParameters& t)
    {
        lastSelectionPolygonF = t.lastSelectionPolygonF;
        currentSelectionPolygonF = t.currentSelectionPolygonF;
        return *this;
    }
};

struct VectorPainterParameters
{
    Object* object;
    int layerIndex;
    VectorSelection vectorSelection;
    VectorImage* vectorImage;
    QList<VertexRef> vertices;
    QList<int> curves;
    QTransform viewTransform;

};

class TransformPainter
{
public:
    TransformPainter();

    void paint(QPainter& painter, const Object* object, const int layerIndex, BaseTool* tool, TransformParameters& transformParameters);
    void paint(QPainter& painter, VectorPainterParameters& vectorPainterParameters);

private:

    void paintBorderAnchors(QPainter& painter);
    void paintStrokeAnchors(QPainter& painter);

    void paintSelectedVectorVertices(QPainter& painter);
    void paintVertices(QPainter& painter);

    int mLayerIndex;
    const Object* mObject;
    TransformParameters mParams;
    BaseTool* mBaseTool;

    VectorSelection mVectorSelection;
    VectorImage* mVectorImage;
    QTransform mViewTransform;
    QList<VertexRef> mClosestVertices;
    QList<int> mClosestCurves;
};

#endif // TRANSFORMPAINTER_H
