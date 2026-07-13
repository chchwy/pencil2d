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
#ifndef IMAGEEXPORTER_H
#define IMAGEEXPORTER_H

#include <functional>

#include <QSize>
#include <QString>
#include <QTransform>

#include "pencilerror.h"

class Object;
class LayerCamera;

/** Exports frames of an Object as still images (PNG/JPG/TIFF/BMP/WEBP),
 *  independent of any UI. */
namespace ImageExporter
{
    /** Receives progress in the 0..progressMax range; return false to
     *  cancel the export. */
    using ProgressFn = std::function<bool(int)>;

    /** Exports the frame range as an image sequence, one file per frame,
     *  numbered <filePath>NNNN.<ext>. */
    Status exportFrames(const Object* object,
                        int frameStart, int frameEnd,
                        const LayerCamera* cameraLayer,
                        QSize exportSize,
                        QString filePath,
                        QString format,
                        bool transparency,
                        bool exportKeyframesOnly,
                        const QString& layerName,
                        bool antialiasing,
                        const ProgressFn& progress = nullptr,
                        int progressMax = 50);

    /** Exports a single frame to the given path. */
    Status exportImage(const Object* object,
                       int frame,
                       const QTransform& view,
                       QSize cameraSize,
                       QSize exportSize,
                       const QString& filePath,
                       const QString& format,
                       bool antialiasing,
                       bool transparency);
}

#endif // IMAGEEXPORTER_H
