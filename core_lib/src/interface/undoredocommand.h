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
                        int undoLayerId,
                        const QString& description,
                        Editor* editor,
                        QUndoCommand* parent = nullptr
                                               );
    ~KeyFrameRemoveCommand() override;

    void undo() override;
    void redo() override;

private:

    int undoLayerId = 0;
    int redoLayerId = 0;

    KeyFrame* undoKeyFrame = nullptr;
    int redoPosition = 0;
};

class KeyFrameAddCommand : public UndoRedoCommand
{
public:
    KeyFrameAddCommand(int undoPosition,
                        int undoLayerId,
                        const QString& description,
                        Editor* editor,
                        QUndoCommand* parent = nullptr);
    ~KeyFrameAddCommand() = default;

    void undo() override;
    void redo() override;

private:

    int undoLayerId = 0;
    int redoLayerId = 0;

    int undoPosition = 0;
    int redoPosition = 0;
};

class MoveKeyFramesCommand : public UndoRedoCommand
{
public:
    MoveKeyFramesCommand(int offset,
                         QList<int> listOfPositions,
                         int undoLayerId,
                         const QString& description,
                         Editor* editor,
                         QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int undoLayerId = 0;
    int redoLayerId = 0;

    int frameOffset = 0;
    QList<int> positions;
};

class RemoveKeyFramesCommand : public UndoRedoCommand
{
public:
    RemoveKeyFramesCommand(const QList<int>& positions,
                           int layerId,
                           const QString& description,
                           Editor* editor,
                           QUndoCommand* parent = nullptr);
    ~RemoveKeyFramesCommand() override;

    void undo() override;
    void redo() override;

private:
    int mLayerId = 0;
    // Each entry: position -> cloned keyframe (owned by this command)
    QList<QPair<int, KeyFrame*>> mFrames;
};

class BitmapReplaceCommand : public UndoRedoCommand
{

public:
    BitmapReplaceCommand(const BitmapImage* backupBitmap,
                  const int undoLayerId,
                  const QString& description,
                  Editor* editor,
                  QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int undoLayerId = 0;
    int redoLayerId = 0;

    BitmapImage undoBitmap;
    BitmapImage redoBitmap;
};

class VectorReplaceCommand : public UndoRedoCommand
{
public:
    VectorReplaceCommand(const VectorImage* undoVector,
                     const int undoLayerId,
                     const QString& description,
                     Editor* editor,
                     QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int undoLayerId = 0;
    int redoLayerId = 0;

    VectorImage undoVector;
    VectorImage redoVector;
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

class DeleteLayerCommand : public UndoRedoCommand
{
public:
    DeleteLayerCommand(int layerIndex,
                       int layerId,
                       const QString& description,
                       Editor* editor,
                       QUndoCommand* parent = nullptr);
    ~DeleteLayerCommand() override;

    void undo() override;
    void redo() override;

private:
    int mLayerIndex = 0;  // position to re-insert on undo
    int mLayerId = 0;
    Layer* mLayer = nullptr;  // owns the layer while between redo and undo states
};

class PasteFramesCommand : public UndoRedoCommand
{
public:
    // beforeFrames/afterFrames: full layer snapshots for deterministic undo/redo
    // Each pair is: key position -> cloned keyframe (owned by this command)
    PasteFramesCommand(const QList<QPair<int, KeyFrame*>>& beforeFrames,
                       const QList<QPair<int, KeyFrame*>>& afterFrames,
                       int layerId,
                       const QString& description,
                       Editor* editor,
                       QUndoCommand* parent = nullptr);
    ~PasteFramesCommand() override;

    void undo() override;
    void redo() override;

private:
    void applySnapshot(Layer* layer, const QList<QPair<int, KeyFrame*>>& snapshot) const;

    int mLayerId = 0;
    QList<QPair<int, KeyFrame*>> mBeforeFrames;
    QList<QPair<int, KeyFrame*>> mAfterFrames;
};

class SetExposureCommand : public UndoRedoCommand
{
public:
    SetExposureCommand(int offset,
                       int layerId,
                       const QList<int>& selectedByPos,
                       const QList<int>& selectedByLast,
                       bool hadSelectedFrames,
                       int currentFramePos,
                       const QString& description,
                       Editor* editor,
                       QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    void applyPositions(Layer* layer, const QList<QPair<int, int>>& moves);

    int mLayerId = 0;
    int mOffset = 0;
    // Each pair: (before_position, after_position) for frames that moved
    QList<QPair<int, int>> mMovedFrames;
};

class InsertExposureCommand : public UndoRedoCommand
{
public:
    InsertExposureCommand(int insertPosition,
                          int layerId,
                          const QString& description,
                          Editor* editor,
                          QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int mLayerId = 0;
    int mInsertPosition = 0;
    int mNewKeyPosition = 0;
    QList<int> mShiftedPositions; // before-positions of frames shifted by the exposure insert
};

class MoveFrameCommand : public UndoRedoCommand
{
public:
    MoveFrameCommand(int position,
                     int offset,
                     int layerId,
                     const QString& description,
                     Editor* editor,
                     QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int mLayerId = 0;
    int mFromPos = 0;
    int mOffset = 0;
    bool mMoved = false; // false if moveKeyFrame returned false (no-op)
};

class ReverseFrameOrderCommand : public UndoRedoCommand
{
public:
    ReverseFrameOrderCommand(const QList<int>& selectedFrames,
                              int layerId,
                              const QString& description,
                              Editor* editor,
                              QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    void applyReverse(Layer* layer) const;

    int mLayerId = 0;
    QList<int> mSelectedFrames; // sorted ascending
};

class CameraTransformCommand : public UndoRedoCommand
{
public:
    CameraTransformCommand(const Camera& before,
                           const Camera& after,
                           int layerId,
                           int frame,
                           const QString& description,
                           Editor* editor,
                           QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    bool apply(const Camera& state);

    int mLayerId = 0;
    int mFrame = 1;
    Camera mBefore;
    Camera mAfter;
};

class AddLayerCommand : public UndoRedoCommand
{
public:
    AddLayerCommand(int layerIndex,
                    int layerId,
                    const QString& description,
                    Editor* editor,
                    QUndoCommand* parent = nullptr);
    ~AddLayerCommand() override;

    void undo() override;
    void redo() override;

private:
    int mLayerIndex = 0;
    int mLayerId = 0;
    Layer* mLayer = nullptr;
};

class DuplicateKeyFrameCommand : public UndoRedoCommand
{
public:
    DuplicateKeyFrameCommand(int layerId,
                             int framePos,
                             const KeyFrame* key,
                             const QString& description,
                             Editor* editor,
                             QUndoCommand* parent = nullptr);
    ~DuplicateKeyFrameCommand() override;

    void undo() override;
    void redo() override;

private:
    int mLayerId = 0;
    int mFramePos = 1;
    KeyFrame* mKeyClone = nullptr;
};

class SwapLayersCommand : public UndoRedoCommand
{
public:
    SwapLayersCommand(int leftIndex,
                      int rightIndex,
                      const QString& description,
                      Editor* editor,
                      QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    bool applySwap();

    int mLeftIndex = 0;
    int mRightIndex = 0;
    bool mDidSwap = false;
};

#endif // UNDOREDOCOMMAND_H
