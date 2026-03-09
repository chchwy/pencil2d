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

#include "layerbitmap.h"
#include "bitmapimage.h"
#include "util.h"

#include <memory>
#include <QDir>
#include <QDomElement>
#include <QTemporaryDir>

// Test fixture class to access protected/private members
class LayerBitmapTest
{
public:
    static KeyFrame* callCreateKeyFrame(LayerBitmap& layer, int position)
    {
        return layer.createKeyFrame(position);
    }

    static Status callSaveKeyFrameFile(LayerBitmap& layer, KeyFrame* frame, QString path)
    {
        return layer.saveKeyFrameFile(frame, path);
    }

    static bool callNeedSaveFrame(LayerBitmap& layer, KeyFrame* key, const QString& path)
    {
        return layer.needSaveFrame(key, path);
    }

    static Status callPresave(LayerBitmap& layer, const QString& dataFolder)
    {
        return layer.presave(dataFolder);
    }
};

TEST_CASE("Load bitmap layer from XML")
{
    std::unique_ptr<Layer> bitmapLayer(new LayerBitmap(1));
    QTemporaryDir dataDir;
    REQUIRE(dataDir.isValid());
    QDomDocument doc;
    doc.setContent(QString("<layer id='1' name='Bitmap Layer' visibility='1'></layer>"));
    QDomElement layerElem = doc.documentElement();
    ProgressCallback nullCallback = []() {};

    auto createFrame = [&layerElem, &doc](QString src = "001.001.png", int frame = 1, int topLeftX = 0, int topLeftY = 0)
    {
        QDomElement frameElem = doc.createElement("image");
        frameElem.setAttribute("src", src);
        frameElem.setAttribute("frame", frame);
        frameElem.setAttribute("topLeftX", topLeftX);
        frameElem.setAttribute("topLeftY", topLeftY);
        layerElem.appendChild(frameElem);
    };

    SECTION("No frames")
    {
        bitmapLayer->loadDomElement(layerElem, dataDir.path(), []() {});

        REQUIRE(bitmapLayer->keyFrameCount() == 0);
    }

    SECTION("Single frame")
    {
        createFrame("001.001.png", 1, 0, 0);

        bitmapLayer->loadDomElement(layerElem, dataDir.path(), nullCallback);

        REQUIRE(bitmapLayer->keyFrameCount() == 1);
        BitmapImage* frame = static_cast<BitmapImage*>(bitmapLayer->getKeyFrameAt(1));
        REQUIRE(frame != nullptr);
        REQUIRE(frame->top() == 0);
        REQUIRE(frame->left() == 0);
        REQUIRE(closestCanonicalPath(frame->fileName()) == closestCanonicalPath(dataDir.filePath("001.001.png")));
    }

    SECTION("Multiple frames")
    {
        createFrame("001.001.png", 1);
        createFrame("001.002.png", 2);

        bitmapLayer->loadDomElement(layerElem, dataDir.path(), nullCallback);

        REQUIRE(bitmapLayer->keyFrameCount() == 2);
        for (int i = 1; i <= 2; i++)
        {
            BitmapImage* frame = static_cast<BitmapImage*>(bitmapLayer->getKeyFrameAt(i));
            REQUIRE(frame != nullptr);
            REQUIRE(frame->top() == 0);
            REQUIRE(frame->left() == 0);
            REQUIRE(closestCanonicalPath(frame->fileName()) == closestCanonicalPath(dataDir.filePath(QString("001.%1.png").arg(QString::number(i), 3, QChar('0')))));
        }
    }

    SECTION("Frame with absolute src")
    {
        createFrame(QDir(dataDir.filePath("001.001.png")).absolutePath());

        bitmapLayer->loadDomElement(layerElem, dataDir.path(), nullCallback);

        REQUIRE(bitmapLayer->keyFrameCount() == 0);
    }

    SECTION("Frame src outside of data dir")
    {
        QTemporaryDir otherDir;
        createFrame(QDir(dataDir.path()).relativeFilePath(QDir(otherDir.filePath("001.001.png")).absolutePath()));

        bitmapLayer->loadDomElement(layerElem, dataDir.path(), nullCallback);

        REQUIRE(bitmapLayer->keyFrameCount() == 0);
    }

    SECTION("Frame src nested in data dir")
    {
        createFrame("subdir/001.001.png");

        bitmapLayer->loadDomElement(layerElem, dataDir.path(), nullCallback);

        REQUIRE(bitmapLayer->keyFrameCount() == 1);
        BitmapImage* frame = static_cast<BitmapImage*>(bitmapLayer->getKeyFrameAt(1));
        REQUIRE(frame != nullptr);
        REQUIRE(frame->top() == 0);
        REQUIRE(frame->left() == 0);
        REQUIRE(closestCanonicalPath(frame->fileName()) == closestCanonicalPath(dataDir.filePath("subdir/001.001.png")));
    }
}

TEST_CASE("LayerBitmap::needSaveFrame - Bug #2 Regression Tests")
{
    QTemporaryDir dataDir;
    REQUIRE(dataDir.isValid());

    LayerBitmap layer(1);
    ProgressCallback nullCallback = []() {};

    SECTION("Unmodified frame with existing file should not need save")
    {
        // Create a frame and simulate it has been saved
        BitmapImage* frame = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 1));
        layer.addKeyFrame(1, frame);

        // Create actual PNG file to simulate previous save
        QString filePath = dataDir.filePath("001.001.png");
        QImage testImage(100, 100, QImage::Format_ARGB32_Premultiplied);
        testImage.fill(Qt::red);
        REQUIRE(testImage.save(filePath));

        // Set frame's filename to match the saved file
        frame->setFileName(filePath);
        frame->setModified(false);

        // Verify file exists
        REQUIRE(QFile::exists(filePath));

        // Test: needSaveFrame should return false (frame doesn't need saving)
        bool needsSave = LayerBitmapTest::callNeedSaveFrame(layer, frame, filePath);
        REQUIRE(needsSave == false);
    }

    SECTION("Modified frame should always need save")
    {
        BitmapImage* frame = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 1));
        layer.addKeyFrame(1, frame);

        QString filePath = dataDir.filePath("001.001.png");
        frame->setFileName(filePath);
        frame->setModified(true);  // Mark as modified

        // Test: modified frame always needs save
        bool needsSave = LayerBitmapTest::callNeedSaveFrame(layer, frame, filePath);
        REQUIRE(needsSave == true);
    }

    SECTION("Frame without existing file should need save")
    {
        BitmapImage* frame = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 1));
        layer.addKeyFrame(1, frame);

        QString filePath = dataDir.filePath("001.001.png");
        frame->setFileName(filePath);
        frame->setModified(false);

        // Verify file does NOT exist
        REQUIRE_FALSE(QFile::exists(filePath));

        // Test: frame without file needs save
        bool needsSave = LayerBitmapTest::callNeedSaveFrame(layer, frame, filePath);
        REQUIRE(needsSave == true);
    }

    SECTION("Frame with empty fileName should need save")
    {
        BitmapImage* frame = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 1));
        layer.addKeyFrame(1, frame);

        QString filePath = dataDir.filePath("001.001.png");
        frame->setFileName("");  // Empty filename
        frame->setModified(false);

        // Test: frame with empty filename needs save
        bool needsSave = LayerBitmapTest::callNeedSaveFrame(layer, frame, filePath);
        REQUIRE(needsSave == true);
    }

    SECTION("BUG #2 FIX: Moved frame should need save even if unmodified")
    {
        BitmapImage* frame = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 5));
        layer.addKeyFrame(5, frame);

        // Create file at original location (frame 5)
        QString oldFilePath = dataDir.filePath("001.005.png");
        QImage testImage(100, 100, QImage::Format_ARGB32_Premultiplied);
        testImage.fill(Qt::blue);
        REQUIRE(testImage.save(oldFilePath));

        // Simulate frame was previously saved at position 5
        frame->setFileName(oldFilePath);
        frame->setModified(false);

        // Verify original file exists
        REQUIRE(QFile::exists(oldFilePath));

        // Now simulate frame moved to position 10
        QString newFilePath = dataDir.filePath("001.010.png");
        frame->setPos(10);  // Move to position 10

        // Test: Even though frame is not modified, it should need save because path changed
        bool needsSave = LayerBitmapTest::callNeedSaveFrame(layer, frame, newFilePath);
        REQUIRE(needsSave == true);

        // Verify the check works: old path != new path
        REQUIRE(oldFilePath != newFilePath);
        REQUIRE(frame->fileName() == oldFilePath);
    }

    SECTION("BUG #2 FIX: Frame at same position with same path should not need save")
    {
        BitmapImage* frame = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 5));
        layer.addKeyFrame(5, frame);

        QString filePath = dataDir.filePath("001.005.png");
        QImage testImage(100, 100, QImage::Format_ARGB32_Premultiplied);
        testImage.fill(Qt::green);
        REQUIRE(testImage.save(filePath));

        frame->setFileName(filePath);
        frame->setModified(false);

        // Test: Frame hasn't moved, same path, unmodified -> no save needed
        bool needsSave = LayerBitmapTest::callNeedSaveFrame(layer, frame, filePath);
        REQUIRE(needsSave == false);
    }

    SECTION("BUG #2 FIX: Multiple moves scenario")
    {
        // Simulate: Frame A moves 1->2, Frame B moves 2->3
        BitmapImage* frameA = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 1));
        BitmapImage* frameB = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 2));
        layer.addKeyFrame(1, frameA);
        layer.addKeyFrame(2, frameB);

        // Create files at original positions
        QString pathA_old = dataDir.filePath("001.001.png");
        QString pathB_old = dataDir.filePath("001.002.png");

        QImage imgA(50, 50, QImage::Format_ARGB32_Premultiplied);
        imgA.fill(Qt::red);
        REQUIRE(imgA.save(pathA_old));

        QImage imgB(50, 50, QImage::Format_ARGB32_Premultiplied);
        imgB.fill(Qt::blue);
        REQUIRE(imgB.save(pathB_old));

        frameA->setFileName(pathA_old);
        frameB->setFileName(pathB_old);
        frameA->setModified(false);
        frameB->setModified(false);

        // Simulate moves: A: 1->2, B: 2->3
        frameA->setPos(2);
        frameB->setPos(3);

        QString pathA_new = dataDir.filePath("001.002.png");
        QString pathB_new = dataDir.filePath("001.003.png");

        // Both frames need to be saved because paths changed
        REQUIRE(LayerBitmapTest::callNeedSaveFrame(layer, frameA, pathA_new) == true);
        REQUIRE(LayerBitmapTest::callNeedSaveFrame(layer, frameB, pathB_new) == true);
    }
}

TEST_CASE("LayerBitmap::saveKeyFrameFile - Integration Test for Bug #2")
{
    QTemporaryDir dataDir;
    REQUIRE(dataDir.isValid());

    LayerBitmap layer(1);
    ProgressCallback nullCallback = []() {};

    SECTION("Save moved unmodified frame")
    {
        // Create frame at position 5
        BitmapImage* frame = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 5));
        layer.addKeyFrame(5, frame);

        // Create actual image data
        QImage img(100, 100, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::cyan);
        frame->setImage(&img);

        // Save at original position
        QString oldPath = dataDir.filePath("001.005.png");
        Status st = LayerBitmapTest::callSaveKeyFrameFile(layer, frame, dataDir.path());
        REQUIRE(st.ok());
        REQUIRE(QFile::exists(oldPath));

        // Verify frame is marked as unmodified after save
        REQUIRE_FALSE(frame->isModified());

        // Move frame to position 10 (without modifying image)
        frame->setPos(10);
        QString newPath = dataDir.filePath("001.010.png");

        // BUG #2 FIX TEST: This should save the frame to new location
        st = LayerBitmapTest::callSaveKeyFrameFile(layer, frame, dataDir.path());
        REQUIRE(st.ok());

        // Verify new file exists
        REQUIRE(QFile::exists(newPath));

        // Verify frame's filename is updated
        REQUIRE(closestCanonicalPath(frame->fileName()) == closestCanonicalPath(newPath));

        // Verify frame is marked unmodified after save
        REQUIRE_FALSE(frame->isModified());
    }

    SECTION("Unmodified frame at same location should return SAFE")
    {
        BitmapImage* frame = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 3));
        layer.addKeyFrame(3, frame);

        QImage img(100, 100, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::yellow);
        frame->setImage(&img);

        // First save
        Status st = LayerBitmapTest::callSaveKeyFrameFile(layer, frame, dataDir.path());
        REQUIRE(st.ok());

        QString filePath = dataDir.filePath("001.003.png");
        REQUIRE(QFile::exists(filePath));

        // Second save without modification should return SAFE (skip save)
        st = LayerBitmapTest::callSaveKeyFrameFile(layer, frame, dataDir.path());
        REQUIRE(st.ok());
        REQUIRE(st.code() == Status::SAFE);
    }
}

TEST_CASE("LayerBitmap::presave - Bug #3 Error Handling Tests")
{
    QTemporaryDir dataDir;
    REQUIRE(dataDir.isValid());

    LayerBitmap layer(1);
    ProgressCallback nullCallback = []() {};

    SECTION("presave() succeeds when no frames need moving")
    {
        // Create frame at position 1
        BitmapImage* frame = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 1));
        layer.addKeyFrame(1, frame);

        // Create and save image
        QImage img(50, 50, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::red);
        frame->setImage(&img);

        QString filePath = dataDir.filePath("001.001.png");
        REQUIRE(img.save(filePath));
        frame->setFileName(filePath);
        frame->setModified(false);

        // presave should succeed (no moves needed)
        Status st = LayerBitmapTest::callPresave(layer, dataDir.path());
        REQUIRE(st.ok());
    }

    SECTION("presave() successfully moves single frame")
    {
        // Create frame at position 5
        BitmapImage* frame = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 5));
        layer.addKeyFrame(5, frame);

        // Create file at original position
        QString oldPath = dataDir.filePath("001.005.png");
        QImage img(50, 50, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::blue);
        REQUIRE(img.save(oldPath));

        frame->setFileName(oldPath);
        frame->setModified(false);

        // Move frame to position 10
        layer.removeKeyFrame(5);
        frame->setPos(10);
        layer.addKeyFrame(10, frame);

        // presave should succeed and move the file
        Status st = LayerBitmapTest::callPresave(layer, dataDir.path());
        REQUIRE(st.ok());

        // Verify file was moved
        QString newPath = dataDir.filePath("001.010.png");
        REQUIRE(QFile::exists(newPath));
        REQUIRE(closestCanonicalPath(frame->fileName()) == closestCanonicalPath(newPath));
    }

    SECTION("presave() successfully handles multiple frame moves")
    {
        // Create three frames at positions 1, 2, 3
        BitmapImage* frame1 = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 1));
        BitmapImage* frame2 = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 2));
        BitmapImage* frame3 = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 3));

        layer.addKeyFrame(1, frame1);
        layer.addKeyFrame(2, frame2);
        layer.addKeyFrame(3, frame3);

        // Create files at original positions
        QString path1 = dataDir.filePath("001.001.png");
        QString path2 = dataDir.filePath("001.002.png");
        QString path3 = dataDir.filePath("001.003.png");

        QImage img1(50, 50, QImage::Format_ARGB32_Premultiplied);
        img1.fill(Qt::red);
        REQUIRE(img1.save(path1));

        QImage img2(50, 50, QImage::Format_ARGB32_Premultiplied);
        img2.fill(Qt::green);
        REQUIRE(img2.save(path2));

        QImage img3(50, 50, QImage::Format_ARGB32_Premultiplied);
        img3.fill(Qt::blue);
        REQUIRE(img3.save(path3));

        frame1->setFileName(path1);
        frame2->setFileName(path2);
        frame3->setFileName(path3);
        frame1->setModified(false);
        frame2->setModified(false);
        frame3->setModified(false);

        // Move all frames: 1→5, 2→6, 3→7
        layer.removeKeyFrame(1);
        layer.removeKeyFrame(2);
        layer.removeKeyFrame(3);

        frame1->setPos(5);
        frame2->setPos(6);
        frame3->setPos(7);

        layer.addKeyFrame(5, frame1);
        layer.addKeyFrame(6, frame2);
        layer.addKeyFrame(7, frame3);

        // presave should successfully move all frames
        Status st = LayerBitmapTest::callPresave(layer, dataDir.path());
        REQUIRE(st.ok());

        // Verify all files were moved
        QString newPath1 = dataDir.filePath("001.005.png");
        QString newPath2 = dataDir.filePath("001.006.png");
        QString newPath3 = dataDir.filePath("001.007.png");

        REQUIRE(QFile::exists(newPath1));
        REQUIRE(QFile::exists(newPath2));
        REQUIRE(QFile::exists(newPath3));

        REQUIRE(closestCanonicalPath(frame1->fileName()) == closestCanonicalPath(newPath1));
        REQUIRE(closestCanonicalPath(frame2->fileName()) == closestCanonicalPath(newPath2));
        REQUIRE(closestCanonicalPath(frame3->fileName()) == closestCanonicalPath(newPath3));
    }

    SECTION("BUG #3 FIX: presave() fails gracefully when source file is missing")
    {
        // Create frame at position 5
        BitmapImage* frame = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 5));
        layer.addKeyFrame(5, frame);

        // Set filename to non-existent file
        QString fakePath = dataDir.filePath("001.005.png");
        frame->setFileName(fakePath);
        frame->setModified(false);

        // Move frame to position 10
        layer.removeKeyFrame(5);
        frame->setPos(10);
        layer.addKeyFrame(10, frame);

        // presave should fail because source file doesn't exist
        Status st = LayerBitmapTest::callPresave(layer, dataDir.path());
        REQUIRE_FALSE(st.ok());
        REQUIRE(st.code() == Status::FAIL);
    }

    SECTION("BUG #3 FIX: presave() fails when destination cannot be removed")
    {
        // Create frame at position 5
        BitmapImage* frame = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 5));
        layer.addKeyFrame(5, frame);

        // Create file at original position
        QString oldPath = dataDir.filePath("001.005.png");
        QImage img(50, 50, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::cyan);
        REQUIRE(img.save(oldPath));

        frame->setFileName(oldPath);
        frame->setModified(false);

        // Move frame to position 10
        layer.removeKeyFrame(5);
        frame->setPos(10);
        layer.addKeyFrame(10, frame);

        // Create a read-only file at destination to simulate failure
        QString destPath = dataDir.filePath("001.010.png");
        QImage blockingImg(50, 50, QImage::Format_ARGB32_Premultiplied);
        blockingImg.fill(Qt::yellow);
        REQUIRE(blockingImg.save(destPath));

        // Make destination read-only
        QFile destFile(destPath);
        REQUIRE(destFile.setPermissions(QFile::ReadOwner | QFile::ReadUser));

        // presave should fail because it can't remove the read-only destination
        Status st = LayerBitmapTest::callPresave(layer, dataDir.path());

        // Restore permissions for cleanup
        destFile.setPermissions(QFile::WriteOwner | QFile::ReadOwner);

        // Verify presave failed
        REQUIRE_FALSE(st.ok());
        REQUIRE(st.code() == Status::FAIL);
    }

    SECTION("BUG #3 FIX: presave() handles chain moves (A→B, B→C)")
    {
        // This tests the critical scenario where frames are shuffled
        // Frame A: 1→2, Frame B: 2→3
        // The temporary file strategy prevents overwrites

        BitmapImage* frameA = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 1));
        BitmapImage* frameB = static_cast<BitmapImage*>(LayerBitmapTest::callCreateKeyFrame(layer, 2));

        layer.addKeyFrame(1, frameA);
        layer.addKeyFrame(2, frameB);

        // Create distinct files
        QString pathA = dataDir.filePath("001.001.png");
        QString pathB = dataDir.filePath("001.002.png");

        QImage imgA(50, 50, QImage::Format_ARGB32_Premultiplied);
        imgA.fill(Qt::red);
        REQUIRE(imgA.save(pathA));

        QImage imgB(50, 50, QImage::Format_ARGB32_Premultiplied);
        imgB.fill(Qt::blue);
        REQUIRE(imgB.save(pathB));

        frameA->setFileName(pathA);
        frameB->setFileName(pathB);
        frameA->setModified(false);
        frameB->setModified(false);

        // Move: A: 1→2, B: 2→3
        layer.removeKeyFrame(1);
        layer.removeKeyFrame(2);

        frameA->setPos(2);
        frameB->setPos(3);

        layer.addKeyFrame(2, frameA);
        layer.addKeyFrame(3, frameB);

        // presave should successfully handle the chain move
        Status st = LayerBitmapTest::callPresave(layer, dataDir.path());
        REQUIRE(st.ok());

        // Verify both files are at new locations
        QString newPathA = dataDir.filePath("001.002.png");
        QString newPathB = dataDir.filePath("001.003.png");

        REQUIRE(QFile::exists(newPathA));
        REQUIRE(QFile::exists(newPathB));

        // Verify the images are still correct (red at 2, blue at 3)
        QImage verifyA(newPathA);
        QImage verifyB(newPathB);
        REQUIRE(verifyA.pixel(25, 25) == qRgba(255, 0, 0, 255));  // Red
        REQUIRE(verifyB.pixel(25, 25) == qRgba(0, 0, 255, 255));  // Blue
    }
}
