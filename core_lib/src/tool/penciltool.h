/*

Pencil2D - Traditional Animation Software
Copyright (C) 2005-2007 Patrick Corrieri & Pascal Naidon
Copyright (C) 2012-2020 Matthew Chiawen Chang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/

#ifndef PENCILTOOL_H
#define PENCILTOOL_H

#include "stroketool.h"
#include <QColor>

class Layer;

class PencilTool : public StrokeTool
{
    Q_OBJECT
public:
    explicit PencilTool(QObject* parent);
    ToolType type() override { return PENCIL; }
    void loadSettings() override;
    void saveSettings() override;
    QCursor cursor() override;
    void resetToDefault() override;

    void pointerPressEvent(PointerEvent*) override;
    void pointerMoveEvent(PointerEvent*) override;
    void pointerReleaseEvent(PointerEvent*) override;

    void drawStroke();
    void paintAt(QPointF point);
    void paintVectorStroke(Layer* layer);

    void setWidth(const qreal width) override;
    void setFeather(const qreal feather) override;
    void setUseFeather(const bool useFeather) override;
    void setInvisibility(const bool invisibility) override;
    void setPressure(const bool pressure) override;
    void setPreserveAlpha(const bool preserveAlpha) override;
    void setStabilizerLevel(const int level) override;
    void setUseFillContour(const bool useFillContour) override;

private:
    QPointF mLastBrushPoint{ 0, 0 };
    QPointF mMouseDownPoint;
};

#endif // PENCILTOOL_H
