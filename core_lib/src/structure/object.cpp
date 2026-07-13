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
#include "object.h"

#include "paletteio.h"
#include "workingdirectory.h"

#include <QDomDocument>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QRegularExpression>

#include "layer.h"
#include "layerbitmap.h"
#include "layervector.h"
#include "layersound.h"
#include "layercamera.h"

#include "util.h"
#include "bitmapimage.h"
#include "vectorimage.h"
#include "fileformat.h"
#include "activeframepool.h"


Object::Object()
{
    mActiveFramePool.reset(new ActiveFramePool);
}

Object::~Object()
{
    mActiveFramePool->clear();

    for (Layer* layer : mLayers)
        delete layer;
    mLayers.clear();

    deleteWorkingDir();
}

void Object::init()
{
    createWorkingDir();

    // default palette
    loadDefaultPalette();
}

QDomElement Object::saveXML(QDomDocument& doc) const
{
    QDomElement objectTag = doc.createElement("object");

    for (Layer* layer : mLayers)
    {
        QDomElement layerTag = layer->createDomElement(doc);
        objectTag.appendChild(layerTag);
    }
    return objectTag;
}

bool Object::loadXML(const QDomElement& docElem, ProgressCallback progressForward)
{
    if (docElem.isNull())
    {
        return false;
    }

    const QString dataDirPath = dataDir();

    for (QDomNode node = docElem.firstChild(); !node.isNull(); node = node.nextSibling())
    {
        QDomElement element = node.toElement(); // try to convert the node to an element.
        if (element.tagName() != "layer")
        {
            continue;
        }

        Layer* newLayer;
        switch (element.attribute("type").toInt())
        {
        case Layer::BITMAP:
            newLayer = new LayerBitmap(getUniqueLayerID());
            break;
        case Layer::VECTOR:
            newLayer = new LayerVector(getUniqueLayerID());
            break;
        case Layer::SOUND:
            newLayer = new LayerSound(getUniqueLayerID());
            break;
        case Layer::CAMERA:
            newLayer = new LayerCamera(getUniqueLayerID());
            break;
        default:
            Q_UNREACHABLE();
        }
        mLayers.append(newLayer);
        newLayer->loadDomElement(element, dataDirPath, progressForward);
    }
    return true;
}

LayerBitmap* Object::addNewBitmapLayer()
{
    LayerBitmap* layerBitmap = new LayerBitmap(getUniqueLayerID());
    mLayers.append(layerBitmap);

    layerBitmap->addNewKeyFrameAt(1);

    return layerBitmap;
}

LayerVector* Object::addNewVectorLayer()
{
    LayerVector* layerVector = new LayerVector(getUniqueLayerID());
    mLayers.append(layerVector);

    layerVector->addNewKeyFrameAt(1);

    return layerVector;
}

LayerSound* Object::addNewSoundLayer()
{
    LayerSound* layerSound = new LayerSound(getUniqueLayerID());
    mLayers.append(layerSound);

    // No default keyFrame at position 1 for Sound layer.

    return layerSound;
}

LayerCamera* Object::addNewCameraLayer()
{
    LayerCamera* layerCamera = new LayerCamera(getUniqueLayerID());
    mLayers.append(layerCamera);

    layerCamera->addNewKeyFrameAt(1);

    return layerCamera;
}

void Object::createWorkingDir()
{
    QString projectName;
    if (mFilePath.isEmpty())
    {
        projectName = "Default";
    }
    else
    {
        QFileInfo fileInfo(mFilePath);
        projectName = fileInfo.completeBaseName();
    }

    mWorkingDir.create(projectName);
}

void Object::deleteWorkingDir() const
{
    mWorkingDir.remove();
}

void Object::setWorkingDir(const QString& path)
{
    mWorkingDir.setPath(path);
}

int Object::getMaxLayerID()
{
    int maxId = 0;
    for (Layer* iLayer : mLayers)
    {
        if (iLayer->id() > maxId)
        {
            maxId = iLayer->id();
        }
    }
    return maxId;
}

int Object::getUniqueLayerID()
{
    return 1 + getMaxLayerID();
}

Layer* Object::getLayer(int i) const
{
    if (i < 0 || i >= getLayerCount())
    {
        return nullptr;
    }

    return mLayers.at(i);
}

Layer* Object::getLayerBelow(int i, Layer::LAYER_TYPE type) const
{
    for (; i >= 0; --i)
    {
        Layer* layerCheck = getLayer(i);
        Q_ASSERT(layerCheck);
        if (layerCheck->type() == type)
        {
            return layerCheck;
        }
    }

    return nullptr;
}

Layer* Object::findLayerById(int layerId) const
{
    for(Layer* layer : mLayers)
    {
        if (layer->id() == layerId)
        {
            return layer;
        }
    }
    return nullptr;
}

Layer* Object::findLayerByName(const QString& strName, Layer::LAYER_TYPE type) const
{
    bool bCheckType = (type != Layer::UNDEFINED);
    for (Layer* layer : mLayers)
    {
        bool isTypeMatch = (bCheckType) ? (type == layer->type()) : true;
        if (isTypeMatch && layer->name() == strName)
        {
            return layer;
        }
    }
    return nullptr;
}

Layer* Object::takeLayer(int layerId)
{
    // Removes the layer from this Object and returns it
    // The ownership of this layer has been transfer to the caller
    int index = -1;
    for (int i = 0; i< mLayers.length(); ++i)
    {
        Layer* layer = mLayers[i];
        if (layer->id() == layerId)
        {
            index = i;
            break;
        }
    }

    if (index == -1) { return nullptr; }

    Layer* layer = mLayers.takeAt(index);
    return layer;
}

bool Object::swapLayers(int i, int j)
{
    bool canSwap = canSwapLayers(i, j);
    if (!canSwap) { return false; }

    if (i != j)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
        mLayers.swapItemsAt(i, j);
#else
        mLayers.swap(i, j);
#endif
    }
    return true;
}

bool Object::canSwapLayers(int layerIndexLeft, int layerIndexRight) const
{
    if (layerIndexLeft < 0 || layerIndexLeft >= mLayers.size())
    {
        return false;
    }

    if (layerIndexRight < 0 || layerIndexRight >= mLayers.size())
    {
        return false;
    }

    Layer* firstLayer = mLayers.first();
    Layer* leftLayer = mLayers.at(layerIndexLeft);
    Layer* rightLayer = mLayers.at(layerIndexRight);

    // The bottom layer can't be swapped!
    if ((leftLayer->type() == Layer::CAMERA ||
         rightLayer->type() == Layer::CAMERA) &&
         (firstLayer == leftLayer || firstLayer == rightLayer)) {
        return false;
    }
    return true;
}

bool Object::canDeleteLayer(int index) const
{
    // We expect the first camera layer to be at the bottom and this layer must not be deleted!
    if (index == 0) {
        return false;
    }

    if (mLayers.at(index) == nullptr)
    {
        return false;
    }

    return true;
}

void Object::deleteLayer(int i)
{
    if (i > -1 && i < mLayers.size())
    {
        delete mLayers.takeAt(i);
    }
}

void Object::deleteLayer(Layer* layer)
{
    auto it = std::find(mLayers.begin(), mLayers.end(), layer);

    if (it != mLayers.end())
    {
        delete layer;
        mLayers.erase(it);
    }
}

bool Object::addLayer(Layer* layer)
{
    if (layer == nullptr || mLayers.contains(layer))
    {
        return false;
    }
    layer->setId(getUniqueLayerID());
    mLayers.append(layer);
    return true;
}

bool Object::insertLayer(int index, Layer* layer)
{
    if (layer == nullptr || mLayers.contains(layer))
    {
        return false;
    }
    index = qBound(0, index, static_cast<int>(mLayers.size()));
    mLayers.insert(index, layer);
    return true;
}

ColorRef Object::getColor(int index) const
{
    ColorRef result(Qt::white, tr("error"));
    if (index > -1 && index < mPalette.size())
    {
        result = mPalette.at(index);
    }
    return result;
}

void Object::setColor(int index, const QColor& newColor)
{
    Q_ASSERT(index >= 0);

    mPalette[index].color = newColor;
}

void Object::setColorRef(int index, const ColorRef& newColorRef)
{
    mPalette[index] = newColorRef;
}

void Object::movePaletteColor(int start, int end)
{
    mPalette.move(start, end);
}

void Object::moveVectorColor(int start, int end)
{
    for (Layer* layer : mLayers)
    {
        if (layer->type() == Layer::VECTOR)
        {
            static_cast<LayerVector*>(layer)->moveColor(start, end);
        }
    }
}

void Object::addColorAtIndex(int index, const ColorRef& newColor)
{
    mPalette.insert(index, newColor);
}

bool Object::isColorInUse(int index) const
{
    for (Layer* layer : mLayers)
    {
        if (layer->type() == Layer::VECTOR)
        {
            LayerVector* layerVector = static_cast<LayerVector*>(layer);

            if (layerVector->usesColor(index))
            {
                return true;
            }
        }
    }
    return false;
}

void Object::removeColor(int index)
{
    for (Layer* layer : mLayers)
    {
        if (layer->type() == Layer::VECTOR)
        {
            LayerVector* layerVector = static_cast<LayerVector*>(layer);
            layerVector->removeColor(index);
        }
    }

    mPalette.removeAt(index);

    // update the vector pictures using that color!
}

void Object::renameColor(int i, const QString& text)
{
    mPalette[i].name = text;
}

QString Object::savePalette(const QString& dataFolder) const
{
    QString fullPath = QDir(dataFolder).filePath(PFF_PALETTE_FILE);
    bool ok = exportPalette(fullPath);
    if (ok)
        return fullPath;
    return "";
}

bool Object::exportPalette(const QString& filePath) const
{
    return PaletteIO::exportPalette(filePath, mPalette);
}

void Object::openPalette(const QString& filePath)
{
    if (!QFile::exists(filePath))
    {
        return;
    }

    mPalette.clear();
    importPalette(filePath);
}

/*
 * Imports palette, e.g. appends to palette
*/
bool Object::importPalette(const QString& filePath)
{
    return PaletteIO::importPalette(filePath, mPalette);
}


void Object::loadDefaultPalette()
{
    mPalette.clear();
    addColor(ColorRef(QColor(Qt::black), tr("Black")));
    addColor(ColorRef(QColor(Qt::red), tr("Red")));
    addColor(ColorRef(QColor(Qt::darkRed), tr("Dark Red")));
    addColor(ColorRef(QColor(255, 128, 0), tr("Orange")));
    addColor(ColorRef(QColor(128, 64, 0), tr("Dark Orange")));
    addColor(ColorRef(QColor(Qt::yellow), tr("Yellow")));
    addColor(ColorRef(QColor(Qt::darkYellow), tr("Dark Yellow")));
    addColor(ColorRef(QColor(Qt::green), tr("Green")));
    addColor(ColorRef(QColor(Qt::darkGreen), tr("Dark Green")));
    addColor(ColorRef(QColor(Qt::cyan), tr("Cyan")));
    addColor(ColorRef(QColor(Qt::darkCyan), tr("Dark Cyan")));
    addColor(ColorRef(QColor(Qt::blue), tr("Blue")));
    addColor(ColorRef(QColor(Qt::darkBlue), tr("Dark Blue")));
    addColor(ColorRef(QColor(255, 255, 255), tr("White")));
    addColor(ColorRef(QColor(220, 220, 229), tr("Very Light Grey")));
    addColor(ColorRef(QColor(Qt::lightGray), tr("Light Grey")));
    addColor(ColorRef(QColor(Qt::gray), tr("Grey")));
    addColor(ColorRef(QColor(Qt::darkGray), tr("Dark Grey")));
    addColor(ColorRef(QColor(255, 227, 187), tr("Pale Orange Yellow")));
    addColor(ColorRef(QColor(221, 196, 161), tr("Pale Grayish Orange Yellow")));
    addColor(ColorRef(QColor(255, 214, 156), tr("Orange Yellow ")));
    addColor(ColorRef(QColor(207, 174, 127), tr("Grayish Orange Yellow")));
    addColor(ColorRef(QColor(255, 198, 116), tr("Light Orange Yellow")));
    addColor(ColorRef(QColor(227, 177, 105), tr("Light Grayish Orange Yellow")));
}

namespace
{
// Preload window used while painting: keep a few frames on either side
// warm so nearby onion-skin and scrub accesses don't hit the disk.
const int PAINT_PRELOAD_FRAMES_BEHIND = 3;
const int PAINT_PRELOAD_FRAMES_AHEAD = 4;
}

void Object::paintImage(QPainter& painter,int frameNumber,
                        bool background,
                        bool antialiasing) const
{
    updateActiveFrames(frameNumber, PAINT_PRELOAD_FRAMES_BEHIND, PAINT_PRELOAD_FRAMES_AHEAD);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    // paints the background
    if (background)
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        painter.setWorldMatrixEnabled(false);
        painter.drawRect(QRect(0, 0, painter.device()->width(), painter.device()->height()));
        painter.setWorldMatrixEnabled(true);
    }

    for (Layer* layer : mLayers)
    {
        if (!layer->visible())
        {
            continue;
        }

        painter.setOpacity(1.0);

        if (layer->type() == Layer::BITMAP)
        {

            LayerBitmap* layerBitmap = static_cast<LayerBitmap*>(layer);
            BitmapImage* bitmap = layerBitmap->getLastBitmapImageAtFrame(frameNumber);
            if (bitmap)
            {
                painter.setOpacity(bitmap->getOpacity());
                bitmap->paintImage(painter);
            }

        }
        // paints the vector images
        if (layer->type() == Layer::VECTOR)
        {
            LayerVector* layerVector = static_cast<LayerVector*>(layer);
            VectorImage* vec = layerVector->getLastVectorImageAtFrame(frameNumber);
            if (vec)
            {
                painter.setOpacity(vec->getOpacity());
                vec->paintImage(painter, *this, false, false, antialiasing);
            }
        }
    }
}

QString Object::copyFileToDataFolder(const QString& strFilePath)
{
    if (!QFile::exists(strFilePath))
    {
        qDebug() << "[Object] sound file doesn't exist: " << strFilePath;
        return "";
    }

    QString sNewFileName = "sound_";
    sNewFileName += QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz.");
    sNewFileName += QFileInfo(strFilePath).suffix();

    QString destFile = QDir(dataDir()).filePath(sNewFileName);

    if (QFile::exists(destFile))
    {
        QFile::remove(destFile);
    }

    bool bCopyOK = QFile::copy(strFilePath, destFile);
    if (!bCopyOK)
    {
        qDebug() << "[Object] couldn't copy sound file to data folder: " << strFilePath;
        return "";
    }

    return destFile;
}

int Object::getLayerCount() const
{
    return mLayers.size();
}

void Object::setData(const ObjectData& d)
{
    mData = d;
}

int Object::totalKeyFrameCount() const
{
    int sum = 0;
    for (const Layer* layer : mLayers)
    {
        sum += layer->keyFrameCount();
    }
    return sum;
}

void Object::updateActiveFrames(int frame, int framesBehind, int framesAhead) const
{
    const int beginFrame = std::max(frame - framesBehind, 1);
    const int endFrame = frame + framesAhead;

    const int minFrameCount = getLayerCount() * (endFrame - beginFrame);
    mActiveFramePool->setMinFrameCount(minFrameCount);

    for (Layer* layer : mLayers)
    {
        if (layer->visible())
        {
            for (int k = beginFrame; k < endFrame; ++k)
            {
                KeyFrame* key = layer->getKeyFrameAt(k);
                mActiveFramePool->put(key);
            }
        }
    }
}

void Object::setActiveFramePoolSize(int sizeInMB)
{
    // convert MB to Byte
    mActiveFramePool->resize(qint64(sizeInMB) * 1024 * 1024);
}
