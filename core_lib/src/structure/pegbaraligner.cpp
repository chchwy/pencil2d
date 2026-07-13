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
#include "pegbaraligner.h"

#include <QDebug>
#include <pencilerror.h>

#include <bitmapimage.h>
#include <layerbitmap.h>
#include <object.h>

PegStatus::PegStatus(ErrorCode code, QPoint point)
    : Status(code), point(point)
{
}

PegBarAligner::PegBarAligner(Object* object, QRect searchRect) :
    mObject(object), mPegSearchRect(searchRect)
{
}


Status PegBarAligner::align(const BitmapImage& referenceImage, const QStringList& layers,
                            QList<int>* alignedFrames)
{
    PegStatus result = findPoint(referenceImage);

    if (!result.ok())
    {
        return Status(Status::FAIL, tr("Peg hole not found!\nCheck selection, and please try again.", "PegBar error message"));
    }

    const int pegX = result.point.x();
    const int pegY = result.point.y();

    for (int i = 0; i < layers.count(); i++)
    {
        LayerBitmap* layerBitmap = static_cast<LayerBitmap*>(mObject->findLayerByName(layers.at(i)));
        for (int k = layerBitmap->firstKeyFramePosition(); k <= layerBitmap->getMaxKeyFramePosition(); k++)
        {
            if (!layerBitmap->keyExists(k)) { continue; }

            BitmapImage* img = layerBitmap->getBitmapImageAtFrame(k);
            img->enableAutoCrop(false);

            result = findPoint(*img);
            if (!result.ok())
            {
                const QString errorDescription = tr("Peg bar not found at %2, %1").arg(k).arg(layerBitmap->name());
                return Status(result.code(), errorDescription);
            }
            img->moveTopLeft(QPoint(img->left() + (pegX - result.point.x()), img->top() + (pegY - result.point.y())));

            if (alignedFrames != nullptr)
            {
                alignedFrames->append(img->pos());
            }
        }
    }

    return Status::OK;
}


PegStatus PegBarAligner::findPoint(const BitmapImage& image) const
{
    QPoint point;
    const int left = mPegSearchRect.left();
    const int right = mPegSearchRect.right();
    const int top = mPegSearchRect.top();
    const int bottom = mPegSearchRect.bottom();
    const int grayValue = mGrayThreshold;

    bool foundX = false;

    for (int x = left; x <= right && !foundX; x++)
    {
        for (int y = top; y <= bottom; y++)
        {
            const QRgb& scan = image.constScanLine(x,y);
            if (qAlpha(scan) == 255 && qGray(scan) < grayValue)
            {
                foundX = true;
                point.setX(x);

                break;
            }
        }
    }

    bool foundY = false;
    for (int y = top; y <= bottom && !foundY; y++)
    {
        for (int x = left; x <= right; x++)
        {
            const QRgb& scan = image.constScanLine(x,y);
            if (qAlpha(scan) == 255 && qGray(scan) < grayValue)
            {
                foundY = true;
                point.setY(y);

                break;
            }
        }
    }

    if (foundX && foundY) {
        return PegStatus(Status::OK, point);
    }
    return Status::FAIL;
}
