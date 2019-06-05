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
#include "layerbitmapsurface.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include "keyframe.h"
#include "bitmapsurface.h"

LayerBitmapSurface::LayerBitmapSurface(Object* object) : Layer(object, Layer::BITMAP)
{
    setName(tr("Bitmap Layer"));
}

LayerBitmapSurface::~LayerBitmapSurface()
{
}

BitmapSurface* LayerBitmapSurface::getBitmapImageAtFrame(int frameNumber)
{
    Q_ASSERT(frameNumber >= 1);
    return static_cast<BitmapSurface*>(getKeyFrameAt(frameNumber));
}

BitmapSurface* LayerBitmapSurface::getLastBitmapImageAtFrame(int frameNumber, int increment)
{
    Q_ASSERT(frameNumber >= 1);
    return static_cast<BitmapSurface*>(getLastKeyFrameAtPosition(frameNumber + increment));
}

void LayerBitmapSurface::loadImageAtFrame(QString path, QPoint topLeft, int frameNumber)
{
    BitmapSurface* pKeyFrame = new BitmapSurface();
    pKeyFrame->createNewSurfaceFromImage(path, topLeft);
    pKeyFrame->setPos(frameNumber);
    loadKey(pKeyFrame);
}

Status LayerBitmapSurface::saveKeyFrameFile(KeyFrame* keyframe, QString path)
{
    QString strFilePath = filePath(keyframe, QDir(path));

    BitmapSurface* bitmapSurface = static_cast<BitmapSurface*>(keyframe);

    bool needSave = needSaveFrame(keyframe, strFilePath);
    if (!needSave)
    {
        return Status::SAFE;
    }

    bitmapSurface->setFileName(strFilePath);

    Status st = bitmapSurface->writeFile(strFilePath);
    if (!st.ok())
    {
        bitmapSurface->setFileName("");

        DebugDetails dd;
        dd << "LayerBitmapSurface::saveKeyFrame";
        dd << QString("  KeyFrame.pos() = %1").arg(keyframe->pos());
        dd << QString("  strFilePath = %1").arg(strFilePath);
        dd << QString("BitmapImage could not be saved");
        dd.collect(st.details());
        return Status(Status::FAIL, dd);
    }

    bitmapSurface->setModified(false);
    return Status::OK;
}

KeyFrame* LayerBitmapSurface::createKeyFrame(int position, Object*)
{
    BitmapSurface* b = new BitmapSurface();
    b->setPos(position);
    return b;
}

Status LayerBitmapSurface::presave(const QString& sDataFolder)
{
    QDir dataFolder(sDataFolder);
    // Handles keys that have been moved but not modified
    std::vector<BitmapSurface*> movedOnlyBitmaps;
    foreachKeyFrame([&movedOnlyBitmaps, &dataFolder,this](KeyFrame* key)
    {
        auto bitmap = static_cast<BitmapSurface*>(key);
        // (b->fileName() != fileName(b) && !modified => the keyframe has been moved, but users didn't draw on it.
        if (!bitmap->fileName().isEmpty()
            && !bitmap->isModified()
            && bitmap->fileName() != filePath(bitmap, dataFolder))
        {
            movedOnlyBitmaps.push_back(bitmap);
        }
    });

    for (BitmapSurface* b : movedOnlyBitmaps)
    {
        // Move to temporary locations first to avoid overwritting anything we shouldn't be
        // Ex: Frame A moves from 1 -> 2, Frame B moves from 2 -> 3. Make sure A does not overwrite B
        QString tmpName = QString::asprintf("t_%03d.%03d.png", id(), b->pos());
        QDir sA, sB;
        if ((sA=QFileInfo(b->fileName()).dir()) != (sB=dataFolder)) {
            // Copy instead of move if the data folder itself has changed
            QFile::copy(b->fileName(), tmpName);
        }
        else {
            QFile::rename(b->fileName(), tmpName);
        }
        b->setFileName(tmpName);
    }

    for (BitmapSurface* b : movedOnlyBitmaps)
    {
        QString dest = filePath(b, dataFolder);
        QFile::remove(dest);

        QFile::rename(b->fileName(), dest);
        b->setFileName(dest);
    }

    return Status::OK;
}

QString LayerBitmapSurface::filePath(KeyFrame* key, const QDir& dataFolder) const
{
    return dataFolder.filePath(fileName(key));
}

QString LayerBitmapSurface::fileName(KeyFrame* key) const
{
    return QString::asprintf("%03d.%03d.png", id(), key->pos());
}

bool LayerBitmapSurface::needSaveFrame(KeyFrame* key, const QString& savePath)
{
    if (key->isModified()) // keyframe was modified
        return true;
    if (QFile::exists(savePath) == false) // hasn't been saved before
        return true;
    if (key->fileName().isEmpty())
        return true;
    return false;
}

QDomElement LayerBitmapSurface::createDomElement(QDomDocument& doc)
{
    QDomElement layerTag = doc.createElement("layer");
    layerTag.setAttribute("id", id());
    layerTag.setAttribute("name", name());
    layerTag.setAttribute("visibility", visible());
    layerTag.setAttribute("type", type());

    foreachKeyFrame([&](KeyFrame* pKeyFrame)
    {
        BitmapSurface* pImg = static_cast<BitmapSurface*>(pKeyFrame);

        QDomElement imageTag = doc.createElement("image");
        imageTag.setAttribute("frame", pKeyFrame->pos());
        imageTag.setAttribute("src", fileName(pKeyFrame));
        imageTag.setAttribute("topLeftX", pImg->bounds().x());
        imageTag.setAttribute("topLeftY", pImg->bounds().y());
        layerTag.appendChild(imageTag);

        Q_ASSERT(QFileInfo(pKeyFrame->fileName()).fileName() == fileName(pKeyFrame));
    });

    return layerTag;
}

void LayerBitmapSurface::loadDomElement(QDomElement element, QString dataDirPath, ProgressCallback progressStep)
{
    if (!element.attribute("id").isNull())
    {
        int id = element.attribute("id").toInt();
        setId(id);
    }
    setName(element.attribute("name"));
    setVisible(element.attribute("visibility").toInt() == 1);

    QDomNode imageTag = element.firstChild();
    while (!imageTag.isNull())
    {
        QDomElement imageElement = imageTag.toElement();
        if (!imageElement.isNull())
        {
            if (imageElement.tagName() == "image")
            {
                QString path = dataDirPath + "/" + imageElement.attribute("src"); // the file is supposed to be in the data directory
                QFileInfo fi(path);
                if (!fi.exists()) path = imageElement.attribute("src");
                int position = imageElement.attribute("frame").toInt();
                int x = imageElement.attribute("topLeftX").toInt();
                int y = imageElement.attribute("topLeftY").toInt();
                loadImageAtFrame(path, QPoint(x, y), position);

                progressStep();
            }
        }
        imageTag = imageTag.nextSibling();
    }
}
