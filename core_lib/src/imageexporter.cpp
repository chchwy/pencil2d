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
#include "imageexporter.h"

#include <QDebug>
#include <QImage>
#include <QImageWriter>
#include <QPainter>

#include "layercamera.h"
#include "object.h"

Status ImageExporter::exportFrames(const Object* object,
                                   int frameStart, int frameEnd,
                                   const LayerCamera* cameraLayer,
                                   QSize exportSize,
                                   QString filePath,
                                   QString format,
                                   bool transparency,
                                   bool exportKeyframesOnly,
                                   const QString& layerName,
                                   bool antialiasing,
                                   const ProgressFn& progress,
                                   int progressMax)
{
    Q_ASSERT(cameraLayer);

    QString extension = "";
    QString formatStr = format;
    if (formatStr == "PNG" || formatStr == "png")
    {
        format = "PNG";
        extension = ".png";
    }
    if (formatStr == "JPG" || formatStr == "jpg" || formatStr == "JPEG" || formatStr == "jpeg")
    {
        format = "JPG";
        extension = ".jpg";
        transparency = false; // JPG doesn't support transparency, so we have to include the background
    }
    if (formatStr == "TIFF" || formatStr == "tiff" || formatStr == "TIF" || formatStr == "tif")
    {
        format = "TIFF";
        extension = ".tiff";
    }
    if (formatStr == "BMP" || formatStr == "bmp")
    {
        format = "BMP";
        extension = ".bmp";
        transparency = false;
    }
    if (formatStr == "WEBP" || formatStr == "webp") {
        format = "WEBP";
        extension = ".webp";
    }
    if (filePath.endsWith(extension, Qt::CaseInsensitive))
    {
        filePath.chop(extension.size());
    }

    qDebug() << "Exporting frames from "
        << frameStart << "to"
        << frameEnd
        << "at size " << exportSize;

    DebugDetails dd;
    dd << "\n[Export frames diagnostics]\n";
    bool ok = true;

    for (int currentFrame = frameStart; currentFrame <= frameEnd; currentFrame++)
    {
        if (progress != nullptr)
        {
            int totalFramesToExport = (frameEnd - frameStart) + 1;
            bool keepGoing = true;
            if (totalFramesToExport != 0) // Avoid dividing by zero.
            {
                keepGoing = progress((currentFrame - frameStart + 1) * progressMax / totalFramesToExport);
            }

            if (!keepGoing)
            {
                break;
            }
        }

        QTransform view = cameraLayer->getViewAtFrame(currentFrame);
        QSize camSize = cameraLayer->getViewSize();

        QString frameNumberString = QString::number(currentFrame);
        while (frameNumberString.length() < 4)
        {
            frameNumberString.prepend("0");
        }
        QString sFileName = filePath + frameNumberString + extension;
        Layer* layer = object->findLayerByName(layerName);
        Status st = Status::SAFE;
        if (exportKeyframesOnly)
        {
            if (layer->keyExists(currentFrame))
            {
                st = exportImage(object, currentFrame, view, camSize, exportSize, sFileName, format, antialiasing, transparency);
            }
        }
        else
        {
            st = exportImage(object, currentFrame, view, camSize, exportSize, sFileName, format, antialiasing, transparency);
        }

        if (!st.ok())
        {
            ok = false;
            dd.collect(st.details());
        }
    }

    if (!ok)
    {
        dd << "\nError: Failed to export one or more frames";
        return Status(Status::FAIL, dd);
    }

    return Status::OK;
}

Status ImageExporter::exportImage(const Object* object,
                                  int frame,
                                  const QTransform& view,
                                  QSize cameraSize,
                                  QSize exportSize,
                                  const QString& filePath,
                                  const QString& format,
                                  bool antialiasing,
                                  bool transparency)
{
    QImage imageToExport(exportSize, QImage::Format_ARGB32_Premultiplied);

    QColor bgColor = Qt::white;
    if (transparency)
        bgColor.setAlpha(0);
    imageToExport.fill(bgColor);

    QTransform centralizeCamera;
    centralizeCamera.translate(cameraSize.width() / 2, cameraSize.height() / 2);

    QPainter painter(&imageToExport);
    painter.setWorldTransform(view * centralizeCamera);
    painter.setWindow(QRect(0, 0, cameraSize.width(), cameraSize.height()));

    object->paintImage(painter, frame, false, antialiasing);

    QImageWriter writer(filePath, format.toStdString().c_str());
    bool b = writer.write(imageToExport);
    if (b) {
        return Status::OK;
    } else {
        DebugDetails dd;
        dd << "ImageExporter::exportImage";
        dd << QString("&nbsp;&nbsp;filePath: ").append(filePath);
        dd << QString("&nbsp;&nbsp;Error: %1 (code %2)").arg(writer.errorString()).arg(static_cast<int>(writer.error()));
        return Status(Status::FAIL, dd);
    }
}
