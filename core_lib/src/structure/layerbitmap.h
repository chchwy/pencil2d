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
#ifndef LAYERBITMAP_H
#define LAYERBITMAP_H

#include "layer.h"

class BitmapImage;
class QDir;

class LayerBitmap : public Layer
{
    Q_DECLARE_TR_FUNCTIONS(LayerBitmap)

public:
    explicit LayerBitmap(int id);
    ~LayerBitmap() override;

    QDomElement createDomElement(QDomDocument& doc) const override;
    void loadDomElement(const QDomElement& element, QString dataDirPath, ProgressCallback progressStep) override;
    Status presave(const QString& sDataFolder) override;

    BitmapImage* getBitmapImageAtFrame(int frameNumber);
    BitmapImage* getLastBitmapImageAtFrame(int frameNumber, int increment = 0);
    void replaceKeyFrame(const KeyFrame*) override;

    void repositionFrame(QPoint point, int frame);
    QRect getFrameBounds(int frame);

protected:
    Status saveKeyFrameFile(KeyFrame*, QString strPath) override;
    KeyFrame* createKeyFrame(int position) override;

private:
    void loadImageAtFrame(QString strFilePath, QPoint topLeft, int frameNumber, qreal opacity);
    QString filePath(KeyFrame* key, const QDir& dataFolder) const;
    QString fileName(KeyFrame* key) const;
    bool needSaveFrame(KeyFrame* key, const QString& strSavePath);
};

#endif
