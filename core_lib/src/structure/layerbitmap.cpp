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
#include "layerbitmap.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include "keyframe.h"
#include "bitmapimage.h"
#include "util/util.h"

LayerBitmap::LayerBitmap(int id) : Layer(id, Layer::BITMAP)
{
    setName(tr("Bitmap Layer"));
}

LayerBitmap::~LayerBitmap()
{
}

BitmapImage* LayerBitmap::getBitmapImageAtFrame(int frameNumber)
{
    Q_ASSERT(frameNumber >= 1);
    return static_cast<BitmapImage*>(getKeyFrameAt(frameNumber));
}

BitmapImage* LayerBitmap::getLastBitmapImageAtFrame(int frameNumber)
{
    Q_ASSERT(frameNumber >= 1);
    return static_cast<BitmapImage*>(getLastKeyFrameAtPosition(frameNumber));
}

void LayerBitmap::replaceKeyFrame(const KeyFrame* bitmapImage)
{
    *getBitmapImageAtFrame(bitmapImage->pos()) = *static_cast<const BitmapImage*>(bitmapImage);
}

void LayerBitmap::repositionFrame(QPoint point, int frame)
{
    BitmapImage* image = getBitmapImageAtFrame(frame);
    Q_ASSERT(image);
    image->moveTopLeft(point);
}

QRect LayerBitmap::getFrameBounds(int frame)
{
    BitmapImage* image = getBitmapImageAtFrame(frame);
    Q_ASSERT(image);
    return image->bounds();
}

void LayerBitmap::loadImageAtFrame(QString path, QPoint topLeft, int frameNumber, qreal opacity)
{
    // An empty keyframe has no file. Don't keep a name that points at nothing,
    // or a later save can't tell an empty keyframe from a lost file.
    if (!QFile::exists(path))
    {
        path.clear();
    }
    BitmapImage* pKeyFrame = new BitmapImage(topLeft, path);
    pKeyFrame->enableAutoCrop(true);
    pKeyFrame->setPos(frameNumber);
    pKeyFrame->setOpacity(opacity);
    loadKey(pKeyFrame);
}

Status LayerBitmap::saveKeyFrameFile(KeyFrame* keyframe, QString path)
{
    BitmapImage* bitmapImage = static_cast<BitmapImage*>(keyframe);
    const QString currentFile = bitmapImage->fileName();
    const QString strFilePath = keyFrameSavePath(keyframe, path, "png");

    if (!bitmapImage->isModified() && strFilePath == currentFile && QFile::exists(currentFile))
    {
        return Status::SAFE; // already on disk, under the name it keeps
    }

    DebugDetails dd;
    dd << "LayerBitmap::saveKeyFrame";
    dd << QString("&nbsp;&nbsp;KeyFrame.pos() = %1").arg(keyframe->pos());
    dd << QString("&nbsp;&nbsp;strFilePath = %1").arg(strFilePath);

    // The keyframe's file name only changes once its image is safely in the new
    // file. An image that isn't loaded in memory has no other copy.
    if (!bitmapImage->isModified() && !currentFile.isEmpty() && !bitmapImage->isLoaded())
    {
        if (!QFile::exists(currentFile))
        {
            dd << QString("Error: The keyframe's file is missing: %1").arg(currentFile);
            return Status(Status::FAIL, dd);
        }
        if (!QFile::copy(currentFile, strFilePath))
        {
            dd << QString("Error: Failed to copy %1").arg(currentFile);
            return Status(Status::FAIL, dd);
        }
        bitmapImage->setFileName(strFilePath);
        return Status::OK;
    }

    Status st = bitmapImage->writeFile(strFilePath);
    if (!st.ok())
    {
        dd << QString("Error: Failed to save BitmapImage");
        dd.collect(st.details());
        return Status(Status::FAIL, dd);
    }
    if (st == Status::OK) // SAFE means the image is empty and has no file
    {
        bitmapImage->setFileName(strFilePath);
    }
    bitmapImage->setModified(false);
    return Status::OK;
}

KeyFrame* LayerBitmap::createKeyFrame(int position)
{
    BitmapImage* b = new BitmapImage;
    b->setPos(position);
    b->enableAutoCrop(true);
    return b;
}

QString LayerBitmap::srcFileName(const KeyFrame* key) const
{
    if (!key->fileName().isEmpty())
    {
        return QFileInfo(key->fileName()).fileName();
    }
    // An empty keyframe has no file. Its src must still not name a file that
    // exists: an old positional name like LLL.PPP.png may belong to another
    // keyframe that has moved away from this position.
    return QString::asprintf("%03d.%03d.empty.png", id(), key->pos());
}

QDomElement LayerBitmap::createDomElement(QDomDocument& doc) const
{
    QDomElement layerElem = createBaseDomElement(doc);

    foreachKeyFrame([&](KeyFrame* pKeyFrame)
    {
        BitmapImage* pImg = static_cast<BitmapImage*>(pKeyFrame);

        QDomElement imageTag = doc.createElement("image");
        imageTag.setAttribute("frame", pKeyFrame->pos());
        imageTag.setAttribute("src", srcFileName(pKeyFrame));
        imageTag.setAttribute("topLeftX", pImg->topLeft().x());
        imageTag.setAttribute("topLeftY", pImg->topLeft().y());
        imageTag.setAttribute("opacity", pImg->getOpacity());
        layerElem.appendChild(imageTag);
    });

    return layerElem;
}

void LayerBitmap::loadDomElement(const QDomElement& element, QString dataDirPath, ProgressCallback progressStep)
{
    this->loadBaseDomElement(element);

    QDomNode imageTag = element.firstChild();
    while (!imageTag.isNull())
    {
        QDomElement imageElement = imageTag.toElement();
        if (!imageElement.isNull() && imageElement.tagName() == "image")
        {
            QString path = validateDataPath(imageElement.attribute("src"), dataDirPath);
            if (!path.isEmpty())
            {
                int position = imageElement.attribute("frame").toInt();
                int x = imageElement.attribute("topLeftX").toInt();
                int y = imageElement.attribute("topLeftY").toInt();
                qreal opacity = imageElement.attribute("opacity", "1.0").toDouble();
                loadImageAtFrame(path, QPoint(x, y), position, opacity);
            }

            progressStep();
        }
        imageTag = imageTag.nextSibling();
    }
}
