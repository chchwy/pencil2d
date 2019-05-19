#ifndef TRANSFORMPAINTER_H
#define TRANSFORMPAINTER_H

#include "QRectF"
#include "QPolygonF"

class QPainter;
class Object;
class BaseTool;

struct TransformParameters
{
    QPolygonF lastSelectionPolygonF;
    QPolygonF currentSelectionPolygonF;
};

class TransformPainter
{
public:
    TransformPainter();

    void paint(QPainter& painter, const Object* object, int layerIndex, BaseTool* tool, TransformParameters& transformParameters);
};

#endif // TRANSFORMPAINTER_H
