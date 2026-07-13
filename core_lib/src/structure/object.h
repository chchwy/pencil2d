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
#ifndef OBJECT_H
#define OBJECT_H

#include <memory>
#include <QCoreApplication>
#include <QObject>
#include <QList>
#include <QColor>
#include "layer.h"
#include "colorref.h"
#include "pencilerror.h"
#include "pencildef.h"
#include "objectdata.h"
#include "workingdirectory.h"

class QFile;
class LayerBitmap;
class LayerVector;
class LayerCamera;
class LayerSound;
class ObjectData;
class ActiveFramePool;


class Object final
{
    Q_DECLARE_TR_FUNCTIONS(Object)
public:
    explicit Object();
    ~Object();

    Object(Object const&) = delete;
    Object(Object&&) = delete;
    Object& operator=(Object const&) = delete;
    Object& operator=(Object&&) = delete;

    void init();
    void createWorkingDir();
    void deleteWorkingDir() const;
    void setWorkingDir(const QString& path); // used by crash recovery

    QString filePath() const { return mFilePath; }
    void    setFilePath(const QString& strFileName) { mFilePath = strFileName; }

    QString workingDir() const { return mWorkingDir.path(); }

    QString dataDir() const { return mWorkingDir.dataPath(); }
    void    setDataDir(const QString& dirPath) { mWorkingDir.setDataPath(dirPath); }

    QString mainXMLFile() const { return mMainXMLFile; }
    void    setMainXMLFile(const QString& file) { mMainXMLFile = file; }

    QDomElement saveXML(QDomDocument& doc) const;
    bool loadXML(const QDomElement& element, ProgressCallback progressForward);

    void paintImage(QPainter& painter, int frameNumber, bool background, bool antialiasing) const;

    QString copyFileToDataFolder(const QString& strFilePath);

    // Color palette
    ColorRef getColor(int index) const;
    void setColor(int index, const QColor& newColor);
    void setColorRef(int index, const ColorRef& newColorRef);
    void movePaletteColor(int start, int end);
    void moveVectorColor(int start, int end);

    void addColor(const ColorRef& newColor) { mPalette.append(newColor); }
    void addColorAtIndex(int index, const ColorRef& newColor);
    void removeColor(int index);
    bool isColorInUse(int index) const;
    void renameColor(int i, const QString& text);
    int getColorCount() { return mPalette.size(); }
    bool importPalette(const QString& filePath);
    void openPalette(const QString& filePath);

    bool exportPalette(const QString& filePath) const;
    QString savePalette(const QString& filePath) const;

    void loadDefaultPalette();

    LayerBitmap* addNewBitmapLayer();
    LayerVector* addNewVectorLayer();
    LayerSound* addNewSoundLayer();
    LayerCamera* addNewCameraLayer();

    int  getLayerCount() const;
    Layer* getLayer(int i) const;
    Layer* getLayerBelow(int i, Layer::LAYER_TYPE type) const;
    Layer* findLayerByName(const QString& strName, Layer::LAYER_TYPE type = Layer::UNDEFINED) const;
    Layer* findLayerById(int layerId) const;
    Layer* takeLayer(int layerId); // Note: transfer ownership of the layer

    bool swapLayers(int i, int j);

    /** Allows you to check whether two layers can be swappped, before doing the actual operation
     *
     *  @param[in] layerIndexLeft The first layer to compare
     *  @param[in] layerIndexRight The second layer to compare
     *
     *  @return true if layers can be swapped, otherwise false
    */
    bool canSwapLayers(int layerIndexLeft, int layerIndexRight) const;

    /** Allows you to check whether the layer at the given index can be deleted
     *
     *  @param[in] index The layer index to check
     *
     *  @return true if the layer can be deleted, otherwise false
    */
    bool canDeleteLayer(int index) const;

    void deleteLayer(int i);
    void deleteLayer(Layer*);
    bool addLayer(Layer* layer);

    /** Inserts a layer at the given index, taking ownership.
     *  Unlike addLayer(), the layer keeps its existing id — used to restore
     *  a layer previously removed with takeLayer() (undo/redo). */
    bool insertLayer(int index, Layer* layer);

    template<typename T>
    std::vector<T*> getLayersByType() const
    {
        std::vector<T*> result;
        for (Layer* layer : mLayers)
        {
            T* t = dynamic_cast<T*>(layer);
            if (t)
                result.push_back(t);
        }
        return result;
    }

    void modification() { modified = true; }
    bool isModified() const { return modified; }
    void setModified(bool b) { modified = b; }

    int getUniqueLayerID();

    ObjectData* data() { return &mData; }
    const ObjectData* data() const { return &mData; }
    void setData(const ObjectData&);

    int totalKeyFrameCount() const;

    /** Loads the keyframes of every visible layer in the window
     *  [frame - framesBehind, frame + framesAhead) into the LRU frame pool
     *  and sizes the pool's minimum accordingly. The window size is the
     *  caller's cache policy, not the Object's. */
    void updateActiveFrames(int frame, int framesBehind, int framesAhead) const;
    void setActiveFramePoolSize(int sizeInMB);

private:
    int getMaxLayerID();

    QString mFilePath;       //< where this object come from. (empty if new project)
    QString mMainXMLFile;    //< the location of main.xml

    /// The folder the pclx is uncompressed to for editing, including its
    /// data/ subfolder with all bitmap, vector and sound files.
    mutable WorkingDirectory mWorkingDir;

    QList<Layer*> mLayers;
    bool modified = false;

    QList<ColorRef> mPalette;

    ObjectData mData;
    mutable std::unique_ptr<ActiveFramePool> mActiveFramePool;
};


#endif
