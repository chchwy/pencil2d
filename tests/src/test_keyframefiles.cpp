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

#include <map>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTextStream>
#include "filemanager.h"
#include "object.h"
#include "layerbitmap.h"
#include "bitmapimage.h"

// Keyframe files keep their name for life, however the keyframe moves (#1953).

namespace
{
    // Writes a .pcl project the way older versions did: one bitmap layer (id 2)
    // whose keyframe files are named after their position, LLL.PPP.png.
    // frames: position -> side of a red square.
    QString writeOldStyleProject(const QTemporaryDir& dir, const std::map<int, int>& frames)
    {
        const QString pclPath = dir.filePath("old.pcl");
        const QString dataPath = pclPath + ".data";
        QDir().mkpath(dataPath);

        QString xml = "<!DOCTYPE PencilDocument><document><object>"
                      "<layer name='Bitmap' id='2' visibility='1' type='1'>";
        for (const auto& [pos, side] : frames)
        {
            const QString name = QString::asprintf("002.%03d.png", pos);
            QImage image(side, side, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::red);
            REQUIRE(image.save(QDir(dataPath).filePath(name)));
            xml += QString("<image frame='%1' topLeftX='0' topLeftY='0' src='%2'/>").arg(pos).arg(name);
        }
        xml += "</layer></object></document>";

        QFile file(pclPath);
        REQUIRE(file.open(QIODevice::WriteOnly));
        QTextStream(&file) << xml;
        return pclPath;
    }

    LayerBitmap* bitmapLayer(Object* o)
    {
        for (int i = 0; i < o->getLayerCount(); ++i)
        {
            if (auto layer = dynamic_cast<LayerBitmap*>(o->getLayer(i)))
                return layer;
        }
        return nullptr;
    }

    void moveKeyFrame(LayerBitmap* layer, int from, int to)
    {
        layer->deselectAll();
        layer->setFrameSelected(from, true);
        REQUIRE(layer->moveSelectedFrames(to - from));
        layer->deselectAll();
        REQUIRE(layer->keyExists(to));
    }

    // Width of the image at a keyframe; -1 when there is no keyframe there.
    int widthAt(LayerBitmap* layer, int pos)
    {
        BitmapImage* b = layer->getBitmapImageAtFrame(pos);
        return b ? b->image()->width() : -1;
    }

    int drawSquare(LayerBitmap* layer, int pos, int side)
    {
        BitmapImage* b = layer->getBitmapImageAtFrame(pos);
        b->image(); // the app only draws on the frame it shows, which is loaded
        b->drawRect(QRectF(0, 0, side, side), QPen(Qt::NoPen), QBrush(Qt::blue),
                    QPainter::CompositionMode_SourceOver, false);
        return b->image()->width(); // drawRect pads the bounds a little
    }

    QString baseName(LayerBitmap* layer, int pos)
    {
        return QFileInfo(layer->getBitmapImageAtFrame(pos)->fileName()).fileName();
    }
}

TEST_CASE("Moving a keyframe doesn't touch its file")
{
    QTemporaryDir dir;
    FileManager fm;
    Object* o = fm.load(writeOldStyleProject(dir, { { 2, 10 }, { 3, 20 } }));
    REQUIRE(o != nullptr);
    LayerBitmap* layer = bitmapLayer(o);

    const QString file2 = layer->getBitmapImageAtFrame(2)->fileName();
    const QString file3 = layer->getBitmapImageAtFrame(3)->fileName();
    moveKeyFrame(layer, 3, 13);
    moveKeyFrame(layer, 2, 12);

    const QString pclx = dir.filePath("moved.pclx");
    REQUIRE(fm.save(o, pclx).ok());

    CHECK(layer->getBitmapImageAtFrame(12)->fileName() == file2);
    CHECK(layer->getBitmapImageAtFrame(13)->fileName() == file3);
    delete o;

    Object* reloaded = fm.load(pclx);
    REQUIRE(reloaded != nullptr);
    layer = bitmapLayer(reloaded);
    CHECK(widthAt(layer, 12) == 10);
    CHECK(widthAt(layer, 13) == 20);
    CHECK_FALSE(layer->keyExists(2));
    CHECK_FALSE(layer->keyExists(3));
    delete reloaded;
}

TEST_CASE("A new keyframe in a moved keyframe's old slot gets its own file")
{
    QTemporaryDir dir;
    FileManager fm;
    Object* o = fm.load(writeOldStyleProject(dir, { { 2, 10 } }));
    REQUIRE(o != nullptr);
    LayerBitmap* layer = bitmapLayer(o);

    moveKeyFrame(layer, 2, 5);
    REQUIRE(layer->addNewKeyFrameAt(2));

    const QString pclx = dir.filePath("slot.pclx");

    SECTION("A drawn keyframe")
    {
        const int drawnWidth = drawSquare(layer, 2, 30);
        REQUIRE(fm.save(o, pclx).ok());
        CHECK(layer->getBitmapImageAtFrame(2)->fileName() != layer->getBitmapImageAtFrame(5)->fileName());
        delete o;

        Object* reloaded = fm.load(pclx);
        REQUIRE(reloaded != nullptr);
        layer = bitmapLayer(reloaded);
        CHECK(widthAt(layer, 5) == 10);
        CHECK(widthAt(layer, 2) == drawnWidth);
        delete reloaded;
    }

    SECTION("An empty keyframe stays empty")
    {
        // The moved keyframe keeps its positional file name 002.002.png. The
        // empty keyframe now at frame 2 must not pick that file up on reload.
        REQUIRE(fm.save(o, pclx).ok());
        delete o;

        Object* reloaded = fm.load(pclx);
        REQUIRE(reloaded != nullptr);
        layer = bitmapLayer(reloaded);
        CHECK(widthAt(layer, 5) == 10);
        REQUIRE(layer->keyExists(2));
        CHECK(widthAt(layer, 2) == 0);
        delete reloaded;
    }
}

TEST_CASE("Swapping two keyframes keeps both images")
{
    QTemporaryDir dir;
    FileManager fm;
    Object* o = fm.load(writeOldStyleProject(dir, { { 2, 10 }, { 3, 20 } }));
    REQUIRE(o != nullptr);
    LayerBitmap* layer = bitmapLayer(o);

    moveKeyFrame(layer, 3, 10);
    moveKeyFrame(layer, 2, 3);
    moveKeyFrame(layer, 10, 2);

    const QString pclx = dir.filePath("swapped.pclx");
    REQUIRE(fm.save(o, pclx).ok());
    delete o;

    Object* reloaded = fm.load(pclx);
    REQUIRE(reloaded != nullptr);
    layer = bitmapLayer(reloaded);
    CHECK(widthAt(layer, 2) == 20);
    CHECK(widthAt(layer, 3) == 10);
    delete reloaded;
}

TEST_CASE("A failed keyframe write keeps every keyframe's file")
{
    QTemporaryDir dir;
    FileManager fm;
    Object* o = fm.load(writeOldStyleProject(dir, { { 2, 10 }, { 3, 20 } }));
    REQUIRE(o != nullptr);
    LayerBitmap* layer = bitmapLayer(o);

    drawSquare(layer, 2, 5); // needs writing; frame 3 is untouched and unloaded
    const QString file2 = layer->getBitmapImageAtFrame(2)->fileName();
    const QString file3 = layer->getBitmapImageAtFrame(3)->fileName();

    // Saving into a folder that doesn't exist fails for every keyframe.
    QStringList written;
    REQUIRE_FALSE(layer->save(dir.filePath("no/such/folder"), written, [] {}).ok());

    CHECK(layer->getBitmapImageAtFrame(2)->fileName() == file2);
    CHECK(layer->getBitmapImageAtFrame(3)->fileName() == file3);
    CHECK(widthAt(layer, 3) == 20);
    delete o;
}

TEST_CASE("Old positional names are kept until the keyframe is redrawn")
{
    QTemporaryDir dir;
    FileManager fm;
    Object* o = fm.load(writeOldStyleProject(dir, { { 2, 10 }, { 3, 20 } }));
    REQUIRE(o != nullptr);
    LayerBitmap* layer = bitmapLayer(o);

    drawSquare(layer, 3, 5);
    const QString pclx = dir.filePath("redrawn.pclx");
    REQUIRE(fm.save(o, pclx).ok());

    CHECK(baseName(layer, 2) == "002.002.png");
    CHECK(QRegularExpression("^002\\.003\\.[0-9a-z]{8}\\.png$").match(baseName(layer, 3)).hasMatch());
    delete o;

    Object* reloaded = fm.load(pclx);
    REQUIRE(reloaded != nullptr);
    layer = bitmapLayer(reloaded);
    CHECK(widthAt(layer, 2) == 10);
    CHECK(widthAt(layer, 3) >= 20);
    delete reloaded;
}

TEST_CASE("Saving an old .pcl project to another folder copies moved keyframes")
{
    QTemporaryDir dir;
    FileManager fm;
    Object* o = fm.load(writeOldStyleProject(dir, { { 2, 10 }, { 3, 20 } }));
    REQUIRE(o != nullptr);
    LayerBitmap* layer = bitmapLayer(o);
    moveKeyFrame(layer, 2, 4);

    QDir(dir.path()).mkpath("elsewhere");
    const QString pcl = dir.filePath("elsewhere/copy.pcl");
    REQUIRE(fm.save(o, pcl).ok());
    delete o;

    Object* reloaded = fm.load(pcl);
    REQUIRE(reloaded != nullptr);
    layer = bitmapLayer(reloaded);
    CHECK(widthAt(layer, 4) == 10);
    CHECK(widthAt(layer, 3) == 20);
    delete reloaded;
}

TEST_CASE("Saving fails when an unloaded keyframe's file has gone missing")
{
    QTemporaryDir dir;
    FileManager fm;
    Object* o = fm.load(writeOldStyleProject(dir, { { 2, 10 }, { 3, 20 } }));
    REQUIRE(o != nullptr);
    LayerBitmap* layer = bitmapLayer(o);

    // Something outside Pencil2D (a temp-folder cleaner, say) removed it.
    const QString file3 = layer->getBitmapImageAtFrame(3)->fileName();
    REQUIRE(QFile::remove(file3));

    CHECK_FALSE(fm.save(o, dir.filePath("missing.pclx")).ok());
    CHECK(layer->getBitmapImageAtFrame(3)->fileName() == file3);
    delete o;
}

TEST_CASE("A clone of an unloaded keyframe is independent of the original")
{
    QTemporaryDir dir;
    FileManager fm;
    Object* o = fm.load(writeOldStyleProject(dir, { { 2, 10 } }));
    REQUIRE(o != nullptr);
    LayerBitmap* layer = bitmapLayer(o);

    BitmapImage* original = layer->getBitmapImageAtFrame(2);
    BitmapImage* copy = original->clone();
    REQUIRE(copy->fileName() != original->fileName());

    copy->drawRect(QRectF(0, 0, 40, 40), QPen(Qt::NoPen), QBrush(Qt::blue),
                   QPainter::CompositionMode_SourceOver, false);
    CHECK(copy->image()->width() > 10);
    CHECK(original->image()->width() == 10);
    delete copy;
    delete o;
}
