/*

Pencil2D - Traditional Animation Software
Copyright (C) 2012-2020 Matthew Chiawen Chang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/
#include "catch.hpp"

#include <QDir>
#include <QFile>
#include <QImage>

#include "imageexporter.h"
#include "object.h"
#include "layerbitmap.h"
#include "layercamera.h"
#include "bitmapimage.h"

namespace
{
const QRgb exportRed = qPremultiply(QColor(255, 0, 0).rgba());

// An object with a camera and a bitmap layer carrying a red 5x5 block
// around the canvas origin.
Object* makeExportScene(LayerCamera** cameraOut, LayerBitmap** bitmapOut)
{
    Object* object = new Object;
    object->init();
    LayerCamera* camera = object->addNewCameraLayer();
    LayerBitmap* bitmap = object->addNewBitmapLayer();
    bitmap->setName("Bitmap");

    BitmapImage* image = bitmap->getBitmapImageAtFrame(1);
    for (int y = 0; y < 5; y++)
        for (int x = 0; x < 5; x++)
            image->setPixel(x, y, exportRed);

    *cameraOut = camera;
    *bitmapOut = bitmap;
    return object;
}
} // namespace

TEST_CASE("ImageExporter writes a frame as a PNG image")
{
    LayerCamera* camera = nullptr;
    LayerBitmap* bitmap = nullptr;
    Object* object = makeExportScene(&camera, &bitmap);

    const QString path = QDir::temp().filePath("pencil2d_test_export.png");
    const QSize camSize = camera->getViewSize();

    Status st = ImageExporter::exportImage(object, 1, camera->getViewAtFrame(1), camSize,
                                           camSize, path, "PNG", false, false);
    REQUIRE(st.ok());

    QImage exported(path);
    REQUIRE_FALSE(exported.isNull());
    REQUIRE(exported.size() == camSize);

    // Canvas origin lands at the image center; the block extends right/down.
    const QPoint center(camSize.width() / 2, camSize.height() / 2);
    REQUIRE(exported.pixelColor(center + QPoint(2, 2)) == QColor(255, 0, 0));
    // Background is opaque white (transparency was off).
    REQUIRE(exported.pixelColor(10, 10) == QColor(255, 255, 255));

    delete object;
    QFile::remove(path);
}

TEST_CASE("ImageExporter exports a numbered sequence and reports progress")
{
    LayerCamera* camera = nullptr;
    LayerBitmap* bitmap = nullptr;
    Object* object = makeExportScene(&camera, &bitmap);

    const QString prefix = QDir::temp().filePath("pencil2d_test_seq");

    QList<int> progressValues;
    Status st = ImageExporter::exportFrames(object, 1, 3, camera, QSize(80, 60),
                                            prefix, "PNG", false, false, "Bitmap", false,
                                            [&progressValues](int value) {
                                                progressValues.append(value);
                                                return true;
                                            },
                                            100);
    REQUIRE(st.ok());
    REQUIRE(progressValues.size() == 3);
    REQUIRE(progressValues.last() == 100);

    for (int i = 1; i <= 3; i++)
    {
        const QString file = prefix + QString("000%1.png").arg(i);
        REQUIRE(QFile::exists(file));
        QFile::remove(file);
    }

    delete object;
}

TEST_CASE("ImageExporter stops when the progress callback cancels")
{
    LayerCamera* camera = nullptr;
    LayerBitmap* bitmap = nullptr;
    Object* object = makeExportScene(&camera, &bitmap);

    const QString prefix = QDir::temp().filePath("pencil2d_test_cancelled");

    int calls = 0;
    Status st = ImageExporter::exportFrames(object, 1, 5, camera, QSize(80, 60),
                                            prefix, "PNG", false, false, "Bitmap", false,
                                            [&calls](int) {
                                                calls++;
                                                return false; // cancel immediately
                                            },
                                            100);
    REQUIRE(st.ok()); // cancellation is not an error
    REQUIRE(calls == 1);
    REQUIRE_FALSE(QFile::exists(prefix + "0001.png"));

    delete object;
}
