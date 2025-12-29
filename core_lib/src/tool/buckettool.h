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

#ifndef BUCKETTOOL_H
#define BUCKETTOOL_H

#include "stroketool.h"

#include "bitmapimage.h"
#include "bitmapbucket.h"

class Layer;
class VectorImage;

class BucketTool : public BaseTool
{
    Q_OBJECT
public:
    explicit BucketTool(QObject* parent = nullptr);

    QCursor cursor() override;
    ToolType type() const override { return BUCKET; }
    ToolCategory category() const override { return BASETOOL; }

    void loadSettings() override;

    void pointerPressEvent(PointerEvent*) override;
    void pointerMoveEvent(PointerEvent*) override;
    void pointerReleaseEvent(PointerEvent*) override;

    void paintBitmap();
    void paintVector(Layer* layer);

    void applyChanges();

    void setStrokeThickness(qreal width);
    void setColorTolerance(int tolerance);
    void setColorToleranceEnabled(bool enabled);
    void setFillExpand(int fillExpandValue);
    void setFillExpandEnabled(bool enabled);
    void setFillReferenceMode(int referenceMode);
    void setFillMode(int mode);

    QPointF getCurrentPoint() const;
    QPointF getCurrentPixel() const;

    qreal fillThickness() const { return settings()->getInfo(BUCKET_FILLTHICKNESS_VALUE).realValue(); }
    int tolerance() const { return settings()->getInfo(BUCKET_COLORTOLERANCE_VALUE).intValue(); }
    int fillExpandAmount() const { return settings()->getInfo(BUCKET_FILLEXPAND_VALUE).intValue(); }
    int fillReferenceMode() const { return settings()->getInfo(BUCKET_FILLLAYERREFERENCEMODE_VALUE).intValue(); }
    int fillMode() const { return settings()->getInfo(BUCKET_FILLMODE_VALUE).intValue(); }
    bool colorToleranceEnabled() const { return settings()->getInfo(BUCKET_COLORTOLERANCE_ENABLED).boolValue(); }
    bool fillExpandEnabled() const { return settings()->getInfo(BUCKET_FILLEXPAND_ENABLED).boolValue(); }
    
signals:
    void fillModeChanged(int mode);
    void fillReferenceModeChanged(int referenceMode);
    void fillExpandEnabledChanged(bool isON);
    void fillExpandChanged(int fillExpandValue);
    void toleranceEnabledChanged(bool isON);
    void toleranceChanged(int width);
    void strokeThicknessChanged(qreal width);

private:

    BitmapBucket mBitmapBucket;
    VectorImage* vectorImage = nullptr;

    bool mFilledOnMove = false;

    StrokeInterpolator mInterpolator;
    const UndoSaveState* mUndoSaveState = nullptr;
};

#endif // BUCKETTOOL_H
