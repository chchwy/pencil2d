/*

Pencil2D - Traditional Animation Software
Copyright (C) 2005-2007 Patrick Corrieri & Pascal Naidon
Copyright (C) 2008-2009 Mj Mendoza IV
Copyright (C) 2012-2020 Matthew Chiawen Chang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/

#ifndef UNDOREDOCOMMAND_H
#define UNDOREDOCOMMAND_H

#include <QUndoCommand>
#include <QRectF>

#include <memory>

#include "bitmapimage.h"
#include "vectorimage.h"
#include "soundclip.h"
#include "camera.h"
#include "layer.h"

class Editor;
class UndoRedoManager;
class PreferenceManager;
class SoundClip;
class Camera;
class KeyFrame;
class TransformCommand;

class UndoRedoCommand : public QUndoCommand
{
public:
    explicit UndoRedoCommand(Editor* editor, QUndoCommand* parent = nullptr);
    ~UndoRedoCommand() = default;

protected:
    Editor* editor() const { return mEditor; }

    bool isFirstRedo() const { return mIsFirstRedo; }
    void setFirstRedo(const bool state) { mIsFirstRedo = state; }

private:
    Editor* mEditor = nullptr;
    bool mIsFirstRedo = true;
};

class KeyFrameRemoveCommand : public UndoRedoCommand
{
public:
    KeyFrameRemoveCommand(const KeyFrame* undoKeyFrame,
                        int layerId,
                        int redoPosition,
                        const QString& description,
                        Editor* editor,
                        QUndoCommand* parent = nullptr
                                               );
    ~KeyFrameRemoveCommand() override;

    void undo() override;
    void redo() override;

private:

    int layerId = 0;

    KeyFrame* undoKeyFrame = nullptr;
    int redoPosition = 0;
};

class KeyFrameAddCommand : public UndoRedoCommand
{
public:
    KeyFrameAddCommand(int position,
                        int layerId,
                        const QString& description,
                        Editor* editor,
                        QUndoCommand* parent = nullptr);
    ~KeyFrameAddCommand() = default;

    void undo() override;
    void redo() override;

private:

    int layerId = 0;
    int position = 0;
};

class MoveKeyFramesCommand : public UndoRedoCommand
{
public:
    MoveKeyFramesCommand(int offset,
                         QList<int> listOfPositions,
                         int layerId,
                         const QString& description,
                         Editor* editor,
                         QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int layerId = 0;

    int frameOffset = 0;
    QList<int> positions;
};

class BitmapReplaceCommand : public UndoRedoCommand
{

public:
    BitmapReplaceCommand(const BitmapImage* undoBitmap,
                  const BitmapImage* redoBitmap,
                  const int layerId,
                  const QString& description,
                  Editor* editor,
                  QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int layerId = 0;

    BitmapImage undoBitmap;
    BitmapImage redoBitmap;
};

class VectorReplaceCommand : public UndoRedoCommand
{
public:
    VectorReplaceCommand(const VectorImage* undoVector,
                     const VectorImage* redoVector,
                     const int layerId,
                     const QString& description,
                     Editor* editor,
                     QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int layerId = 0;

    VectorImage undoVector;
    VectorImage redoVector;
};

class LayerAddCommand : public UndoRedoCommand
{
public:
    LayerAddCommand(int layerId,
                    int layerIndex,
                    const QString& description,
                    Editor* editor,
                    QUndoCommand* parent = nullptr);
    ~LayerAddCommand() override;

    void undo() override;
    void redo() override;

private:
    int layerId = 0;
    int layerIndex = 0;

    /// Holds the layer while it is undone out of the document.
    std::unique_ptr<Layer> takenLayer;
};

class LayerMoveCommand : public UndoRedoCommand
{
public:
    LayerMoveCommand(int fromIndex,
                     int toIndex,
                     const QString& description,
                     Editor* editor,
                     QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    void moveLayer(int from, int to);

    int fromIndex = 0;
    int toIndex = 0;
};

class LayerRenameCommand : public UndoRedoCommand
{
public:
    LayerRenameCommand(int layerId,
                       const QString& oldName,
                       const QString& newName,
                       const QString& description,
                       Editor* editor,
                       QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    void rename(const QString& name);

    int layerId = 0;
    QString oldName;
    QString newName;
};

class TransformCommand : public UndoRedoCommand

{
public:
    TransformCommand(const QRectF& undoSelectionRect,
                     const QPointF& undoTranslation,
                     const qreal undoRotationAngle,
                     const qreal undoScaleX,
                     const qreal undoScaleY,
                     const QPointF& undoTransformAnchor,
                     const bool roundPixels,
                     const QString& description,
                     Editor* editor,
                     QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    void apply(const QRectF& selectionRect,
               const QPointF& translation,
               const qreal rotationAngle,
               const qreal scaleX,
               const qreal scaleY,
               const QPointF& selectionAnchor,
               const bool roundPixels);

    QRectF undoSelectionRect;
    QRectF redoSelectionRect;

    QPointF undoAnchor;
    QPointF redoAnchor;

    QPointF undoTranslation;
    QPointF redoTranslation;

    qreal undoScaleX;
    qreal undoScaleY;

    qreal redoScaleX;
    qreal redoScaleY;

    qreal undoRotationAngle;
    qreal redoRotationAngle;

    bool roundPixels;
};

#endif // UNDOREDOCOMMAND_H
