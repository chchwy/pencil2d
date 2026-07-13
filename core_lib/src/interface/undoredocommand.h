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
    /** @param addedKeyFrame The keyframe that was added, or nullptr.
     *  When given, it is cloned so redo can restore content-bearing
     *  additions (e.g. pasted frames); otherwise redo creates a new
     *  empty keyframe. */
    KeyFrameAddCommand(int position,
                        int layerId,
                        const KeyFrame* addedKeyFrame,
                        const QString& description,
                        Editor* editor,
                        QUndoCommand* parent = nullptr);
    ~KeyFrameAddCommand() override;

    void undo() override;
    void redo() override;

private:

    int layerId = 0;
    int position = 0;

    std::unique_ptr<KeyFrame> keyClone;
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
    /** The sub-images cover only the region that actually changed (the
     *  diff bounding box), not the whole canvas; they may be empty when
     *  the command only carries a selection-transform child. */
    BitmapReplaceCommand(const BitmapImage& undoSubImage,
                  const BitmapImage& redoSubImage,
                  const int layerId,
                  const int framePosition,
                  const QString& description,
                  Editor* editor,
                  QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    void applySubImage(const BitmapImage& subImage);

    int layerId = 0;
    int framePosition = 0;

    BitmapImage undoSubImage;
    BitmapImage redoSubImage;
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

class LayerRemoveCommand : public UndoRedoCommand
{
public:
    /** Takes ownership of the removed layer; the layer must already have
     *  been taken out of the document (LayerManager::takeLayer). */
    LayerRemoveCommand(Layer* takenLayer,
                       int layerIndex,
                       const QString& description,
                       Editor* editor,
                       QUndoCommand* parent = nullptr);
    ~LayerRemoveCommand() override;

    void undo() override;
    void redo() override;

private:
    int layerId = 0;
    int layerIndex = 0;

    /// Holds the layer while it is removed from the document.
    std::unique_ptr<Layer> takenLayer;
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

/** Ids for QUndoStack command compression; commands without an id never merge. */
enum UndoRedoCommandId
{
    OpacityCommandId = 1
};

class KeyFrameOpacityCommand : public UndoRedoCommand
{
public:
    /** One opacity value per position; the lists run in parallel. */
    KeyFrameOpacityCommand(int layerId,
                           const QList<int>& positions,
                           const QList<qreal>& undoOpacities,
                           const QList<qreal>& redoOpacities,
                           const QString& description,
                           Editor* editor,
                           QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

    /** Consecutive opacity changes to the same keyframes merge into one
     *  undo step, so dragging the opacity slider doesn't flood the
     *  history with one entry per tick. */
    int id() const override { return OpacityCommandId; }
    bool mergeWith(const QUndoCommand* other) override;

private:
    void apply(const QList<qreal>& opacities);

    int layerId = 0;
    QList<int> positions;
    QList<qreal> undoOpacities;
    QList<qreal> redoOpacities;
};

class CameraViewRectCommand : public UndoRedoCommand
{
public:
    CameraViewRectCommand(int layerId,
                          const QRect& undoViewRect,
                          const QRect& redoViewRect,
                          int framePosition,
                          const QString& description,
                          Editor* editor,
                          QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    void apply(const QRect& viewRect);

    int layerId = 0;
    int framePosition = 0;

    QRect undoViewRect;
    QRect redoViewRect;
};

class CameraReplaceCommand : public UndoRedoCommand
{
public:
    CameraReplaceCommand(const Camera* undoCamera,
                         const Camera* redoCamera,
                         int layerId,
                         const QString& description,
                         Editor* editor,
                         QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    void apply(const Camera& camera);

    int layerId = 0;

    Camera undoCamera;
    Camera redoCamera;
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
