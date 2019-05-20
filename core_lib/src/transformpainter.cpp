#include "transformpainter.h"

#include "object.h"
#include "qpainter.h"
#include "vectorimage.h"
#include "vectorselection.h"
#include "layermanager.h"
#include "basetool.h"


TransformPainter::TransformPainter() : mLayerIndex(0), mObject(nullptr), mBaseTool(nullptr)
{

}

void TransformPainter::paint(QPainter& painter,
                             const Object* object,
                             const int layerIndex,
                             BaseTool* tool,
                             TransformParameters& tParams)
{

    mObject = object;
    mLayerIndex = layerIndex;
    mBaseTool = tool;
    mParams = tParams;
    paintBorderAnchors(painter);
}

void TransformPainter::paint(QPainter& painter, VectorPainterParameters& vPP)
{
    mObject = vPP.object;
    mLayerIndex = vPP.layerIndex;
    mVectorImage = vPP.vectorImage;
    mVectorSelection = vPP.vectorSelection;
    mClosestCurves = vPP.curves;
    mClosestVertices = vPP.vertices;
    paintStrokeAnchors(painter);
}

void TransformPainter::paintBorderAnchors(QPainter& painter)
{
    Layer* layer = mObject->getLayer(mLayerIndex);

    if (layer == nullptr) { return; }

    if (layer->type() == Layer::BITMAP)
    {
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(Qt::DashLine));

        // Draw previous selection
        painter.drawPolygon(mParams.lastSelectionPolygonF.toPolygon());

        // Draw current selection
        painter.drawPolygon(mParams.currentSelectionPolygonF.toPolygon());

    }
    if (layer->type() == Layer::VECTOR)
    {
        painter.setBrush(QColor(0, 0, 0, 20));
        painter.setPen(Qt::gray);
        painter.drawPolygon(mParams.currentSelectionPolygonF);
    }

    if (layer->type() != Layer::VECTOR || mBaseTool->type() != SELECT)
    {
        painter.setPen(Qt::SolidLine);
        painter.setBrush(QBrush(Qt::gray));
        int width = 6;
        int radius = width / 2;

        const QRectF topLeftCorner = QRectF(mParams.currentSelectionPolygonF[0].x() - radius,
                                            mParams.currentSelectionPolygonF[0].y() - radius,
                                            width, width);
        painter.drawRect(topLeftCorner);

        const QRectF topRightCorner = QRectF(mParams.currentSelectionPolygonF[1].x() - radius,
                                             mParams.currentSelectionPolygonF[1].y() - radius,
                                             width, width);
        painter.drawRect(topRightCorner);

        const QRectF bottomRightCorner = QRectF(mParams.currentSelectionPolygonF[2].x() - radius,
                                                mParams.currentSelectionPolygonF[2].y() - radius,
                                                width, width);
        painter.drawRect(bottomRightCorner);

        const QRectF bottomLeftCorner = QRectF(mParams.currentSelectionPolygonF[3].x() - radius,
                                               mParams.currentSelectionPolygonF[3].y() - radius,
                                               width, width);
        painter.drawRect(bottomLeftCorner);

        painter.setBrush(QColor(0, 255, 0, 50));
        painter.setPen(Qt::green);
    }
}

void TransformPainter::paintStrokeAnchors(QPainter& painter)
{
    Layer* layer = mObject->getLayer(mLayerIndex);
    if (layer->type() == Layer::VECTOR)
    {
        painter.save();
        painter.setWorldMatrixEnabled(false);
        painter.setRenderHint(QPainter::Antialiasing, false);
        // ----- paints the edited elements
        QPen pen2(Qt::black, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen2);
        QColor colour;
        // ------------ vertices of the edited curves
        colour = QColor(200, 200, 200);
        painter.setBrush(colour);

        paintVertices(painter);
        // ------------ selected vertices of the edited curves
        paintSelectedVectorVertices(painter);
        // ----- paints the closest vertices
        colour = QColor(255, 0, 0);
        painter.setBrush(colour);
        if (mVectorSelection.curve.size() > 0)
        {
            for (int k = 0; k < mClosestVertices.size(); k++)
            {
                VertexRef vertexRef = mClosestVertices.at(k);
                QPointF vertexPoint = mVectorImage->getVertex(vertexRef);

                QRectF rectangle = QRectF(mViewTransform.map(vertexPoint) - QPointF(3.0, 3.0), QSizeF(7, 7));
                painter.drawRect(rectangle);
            }
        }
        painter.restore();
    }

        // end siwtch
}

void TransformPainter::paintVertices(QPainter& painter)
{
    for (int k = 0; k < mVectorSelection.curve.size(); k++)
    {
        int curveNumber = mVectorSelection.curve.at(k);

        for (int vertexNumber = -1; vertexNumber < mVectorImage->getCurveSize(curveNumber); vertexNumber++)
        {
            QPointF vertexPoint = mVectorImage->getVertex(curveNumber, vertexNumber);
            QRectF rectangle(mViewTransform.map(vertexPoint) - QPointF(3.0, 3.0), QSizeF(7, 7));
            if (painter.viewport().contains(mViewTransform.map(vertexPoint).toPoint()))
            {
                painter.drawRect(rectangle);
            }
        }
    }
}

void TransformPainter::paintSelectedVectorVertices(QPainter& painter)
{
    QColor colour = QColor(100, 100, 255);
    painter.setBrush(colour);
    for (int k = 0; k < mVectorSelection.vertex.size(); k++)
    {
        VertexRef vertexRef = mVectorSelection.vertex.at(k);
        QPointF vertexPoint = mVectorImage->getVertex(vertexRef);
        QRectF rectangle0 = QRectF(mViewTransform.map(vertexPoint) - QPointF(3.0, 3.0), QSizeF(7, 7));
        painter.drawRect(rectangle0);
    }
}
