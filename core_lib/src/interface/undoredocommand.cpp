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

#include <QDebug>
#include <QFile>

#include "layermanager.h"
#include "selectionmanager.h"
#include "soundmanager.h"

#include "layersound.h"
#include "layerbitmap.h"
#include "layervector.h"
#include "layercamera.h"
#include "layer.h"

#include "editor.h"
#include "undoredocommand.h"

UndoRedoCommand::UndoRedoCommand(Editor* editor, QUndoCommand* parent) : QUndoCommand(parent)
{
    qDebug() << "backupElement created";
    mEditor = editor;
}

KeyFrameRemoveCommand::KeyFrameRemoveCommand(const KeyFrame* undoKeyFrame,
                                         int layerId,
                                         int redoPosition,
                                         const QString &description,
                                         Editor *editor,
                                         QUndoCommand *parent) : UndoRedoCommand(editor, parent)
{
    this->undoKeyFrame = undoKeyFrame->clone();
    this->layerId = layerId;
    this->redoPosition = redoPosition;

    setText(description);
}

KeyFrameRemoveCommand::~KeyFrameRemoveCommand()
{
    delete undoKeyFrame;
}

void KeyFrameRemoveCommand::undo()
{
    Layer* layer = editor()->layers()->findLayerById(layerId);
    if (layer == nullptr) {
        // Until we support layer deletion recovery, we mark the command as
        // obsolete as soon as it's been
        return setObsolete(true);
    }

    UndoRedoCommand::undo();

    KeyFrame* restoredKey = undoKeyFrame->clone();
    if (layer->type() == Layer::SOUND)
    {
        // A cloned SoundClip has no media player; recreate it the way the
        // legacy restore path does. If the sound file is gone, the clip
        // can't be restored — drop the command instead of inserting a
        // silent keyframe.
        SoundClip* clip = static_cast<SoundClip*>(restoredKey);
        const QString soundFile = clip->fileName();
        if (soundFile.isEmpty() || !QFile::exists(soundFile))
        {
            delete clip;
            return setObsolete(true);
        }

        Status status = editor()->sound()->loadSound(clip, soundFile);
        if (!status.ok())
        {
            // loadSound only deletes the clip when it fails past its file
            // checks; on its FILE_NOT_FOUND/FAIL early returns the clip is
            // still ours to free.
            if (status == Status::FILE_NOT_FOUND || status == Status::FAIL)
            {
                delete clip;
            }
            return setObsolete(true);
        }
    }

    layer->addKeyFrame(undoKeyFrame->pos(), restoredKey);

    emit editor()->frameModified(undoKeyFrame->pos());
    editor()->layers()->notifyAnimationLengthChanged();
    editor()->scrubTo(undoKeyFrame->pos());
}

void KeyFrameRemoveCommand::redo()
{
    Layer* layer = editor()->layers()->findLayerById(layerId);
    if (layer == nullptr) {
        // Until we support layer deletion recovery, we mark the command as
        // obsolete as soon as it's been
        return setObsolete(true);
    }

    UndoRedoCommand::redo();

    if (isFirstRedo()) { setFirstRedo(false); return; }

    layer->removeKeyFrame(redoPosition);

    emit editor()->frameModified(redoPosition);
    editor()->layers()->notifyAnimationLengthChanged();
    editor()->scrubTo(redoPosition);
}

KeyFrameAddCommand::KeyFrameAddCommand(int position,
                                       int layerId,
                                       const QString &description,
                                       Editor *editor,
                                       QUndoCommand *parent)
    : UndoRedoCommand(editor, parent)
{
    this->position = position;
    this->layerId = layerId;

    setText(description);
}

void KeyFrameAddCommand::undo()
{
    Layer* layer = editor()->layers()->findLayerById(layerId);
    if (!layer) {
        return setObsolete(true);
    }

    UndoRedoCommand::undo();

    layer->removeKeyFrame(position);

    emit editor()->frameModified(position);
    editor()->layers()->notifyAnimationLengthChanged();
    editor()->layers()->setCurrentLayer(layer);
    editor()->scrubTo(position);
}

void KeyFrameAddCommand::redo()
{
    Layer* layer = editor()->layers()->findLayerById(layerId);
    if (!layer) {
        return setObsolete(true);
    }

    UndoRedoCommand::redo();

    // Ignore automatic redo when added to undo stack
    if (isFirstRedo()) { setFirstRedo(false); return; }

    layer->addNewKeyFrameAt(position);

    emit editor()->frameModified(position);
    editor()->layers()->notifyAnimationLengthChanged();
    editor()->layers()->setCurrentLayer(layer);
    editor()->scrubTo(position);
}

MoveKeyFramesCommand::MoveKeyFramesCommand(int offset,
                                         QList<int> listOfPositions,
                                         int layerId,
                                         const QString& description,
                                         Editor* editor,
                                         QUndoCommand *parent)
    : UndoRedoCommand(editor, parent)
{
    this->frameOffset = offset;
    this->positions = listOfPositions;

    this->layerId = layerId;

    setText(description);
}

void MoveKeyFramesCommand::undo()
{
    Layer* undoLayer = editor()->layers()->findLayerById(layerId);

    if (!undoLayer) {
        return setObsolete(true);
    }

    UndoRedoCommand::undo();

    for (int position : qAsConst(positions)) {
        undoLayer->setFrameSelected(position, true);
    }
    undoLayer->moveSelectedFrames(-frameOffset);

    emit editor()->framesModified();
}

void MoveKeyFramesCommand::redo()
{
    Layer* redoLayer = editor()->layers()->findLayerById(layerId);

    if (!redoLayer) {
        return setObsolete(true);
    }

    UndoRedoCommand::redo();

    // Ignore automatic redo when added to undo stack
    if (isFirstRedo()) { setFirstRedo(false); return; }

    QList<int> newPositions = positions;


    for (int position : qAsConst(newPositions)) {
        redoLayer->setFrameSelected(position, true);
    }

    redoLayer->moveSelectedFrames(frameOffset);

    emit editor()->framesModified();
}
BitmapReplaceCommand::BitmapReplaceCommand(const BitmapImage* undoBitmap,
                             const BitmapImage* redoBitmap,
                             const int layerId,
                             const QString& description,
                             Editor *editor,
                             QUndoCommand *parent) : UndoRedoCommand(editor, parent)
{
    this->undoBitmap = *undoBitmap;
    this->redoBitmap = *redoBitmap;
    this->layerId = layerId;

    setText(description);
}

void BitmapReplaceCommand::undo()
{
    Layer* layer = editor()->layers()->findLayerById(layerId);
    if (!layer) {
        return setObsolete(true);
    }

    UndoRedoCommand::undo();

    static_cast<LayerBitmap*>(layer)->replaceKeyFrame(&undoBitmap);

    editor()->scrubTo(undoBitmap.pos());
}

void BitmapReplaceCommand::redo()
{
    Layer* layer = editor()->layers()->findLayerById(layerId);
    if (!layer) {
        return setObsolete(true);
    }

    UndoRedoCommand::redo();

    // Ignore automatic redo when added to undo stack
    if (isFirstRedo()) { setFirstRedo(false); return; }

    static_cast<LayerBitmap*>(layer)->replaceKeyFrame(&redoBitmap);

    editor()->scrubTo(redoBitmap.pos());
}

VectorReplaceCommand::VectorReplaceCommand(const VectorImage* undoVector,
                                   const VectorImage* redoVector,
                                   const int layerId,
                                   const QString& description,
                                   Editor* editor,
                                   QUndoCommand* parent) : UndoRedoCommand(editor, parent)
{
    this->undoVector = *undoVector;
    this->redoVector = *redoVector;
    this->layerId = layerId;

    setText(description);
}

void VectorReplaceCommand::undo()
{
    Layer* layer = editor()->layers()->findLayerById(layerId);
    if (!layer) {
        return setObsolete(true);
    }

    UndoRedoCommand::undo();

    static_cast<LayerVector*>(layer)->replaceKeyFrame(&undoVector);

    editor()->scrubTo(undoVector.pos());
}

void VectorReplaceCommand::redo()
{
    Layer* layer = editor()->layers()->findLayerById(layerId);
    if (!layer) {
        return setObsolete(true);
    }

    UndoRedoCommand::redo();

    // Ignore automatic redo when added to undo stack
    if (isFirstRedo()) { setFirstRedo(false); return; }

    static_cast<LayerVector*>(layer)->replaceKeyFrame(&redoVector);

    editor()->scrubTo(redoVector.pos());
}

LayerRemoveCommand::LayerRemoveCommand(Layer* takenLayer,
                                       int layerIndex,
                                       const QString& description,
                                       Editor* editor,
                                       QUndoCommand* parent) : UndoRedoCommand(editor, parent)
{
    Q_ASSERT(takenLayer != nullptr);
    this->takenLayer.reset(takenLayer);
    this->layerId = takenLayer->id();
    this->layerIndex = layerIndex;

    setText(description);
}

LayerRemoveCommand::~LayerRemoveCommand() = default;

void LayerRemoveCommand::undo()
{
    UndoRedoCommand::undo();

    if (takenLayer == nullptr) {
        return setObsolete(true);
    }

    editor()->layers()->restoreLayer(takenLayer.release(), layerIndex);
}

void LayerRemoveCommand::redo()
{
    UndoRedoCommand::redo();

    // Ignore automatic redo when added to undo stack; the layer was
    // already taken out of the document before the command was pushed.
    if (isFirstRedo()) { setFirstRedo(false); return; }

    takenLayer.reset(editor()->layers()->takeLayer(layerId));
    if (takenLayer == nullptr) {
        return setObsolete(true);
    }
}

LayerAddCommand::LayerAddCommand(int layerId,
                                 int layerIndex,
                                 const QString& description,
                                 Editor* editor,
                                 QUndoCommand* parent) : UndoRedoCommand(editor, parent)
{
    this->layerId = layerId;
    this->layerIndex = layerIndex;

    setText(description);
}

LayerAddCommand::~LayerAddCommand() = default;

void LayerAddCommand::undo()
{
    UndoRedoCommand::undo();

    takenLayer.reset(editor()->layers()->takeLayer(layerId));
    if (takenLayer == nullptr) {
        return setObsolete(true);
    }
}

void LayerAddCommand::redo()
{
    UndoRedoCommand::redo();

    // Ignore automatic redo when added to undo stack
    if (isFirstRedo()) { setFirstRedo(false); return; }

    if (takenLayer == nullptr) {
        return setObsolete(true);
    }

    editor()->layers()->restoreLayer(takenLayer.release(), layerIndex);
}

LayerMoveCommand::LayerMoveCommand(int fromIndex,
                                   int toIndex,
                                   const QString& description,
                                   Editor* editor,
                                   QUndoCommand* parent) : UndoRedoCommand(editor, parent)
{
    this->fromIndex = fromIndex;
    this->toIndex = toIndex;

    setText(description);
}

void LayerMoveCommand::moveLayer(int from, int to)
{
    const int count = editor()->layers()->count();
    if (from < 0 || from >= count || to < 0 || to >= count) {
        return setObsolete(true);
    }

    // A move is a series of adjacent swaps, mirroring the timeline's
    // drag behavior; Editor::swapLayers handles the current-layer and
    // UI updates per swap.
    if (to < from)
    {
        for (int i = from - 1; i >= to; i--)
            editor()->swapLayers(i, i + 1);
    }
    else
    {
        for (int i = from + 1; i <= to; i++)
            editor()->swapLayers(i, i - 1);
    }
}

void LayerMoveCommand::undo()
{
    UndoRedoCommand::undo();
    moveLayer(toIndex, fromIndex);
}

void LayerMoveCommand::redo()
{
    UndoRedoCommand::redo();

    // Ignore automatic redo when added to undo stack
    if (isFirstRedo()) { setFirstRedo(false); return; }

    moveLayer(fromIndex, toIndex);
}

LayerRenameCommand::LayerRenameCommand(int layerId,
                                       const QString& oldName,
                                       const QString& newName,
                                       const QString& description,
                                       Editor* editor,
                                       QUndoCommand* parent) : UndoRedoCommand(editor, parent)
{
    this->layerId = layerId;
    this->oldName = oldName;
    this->newName = newName;

    setText(description);
}

void LayerRenameCommand::rename(const QString& name)
{
    Layer* layer = editor()->layers()->findLayerById(layerId);
    if (layer == nullptr) {
        return setObsolete(true);
    }

    layer->setName(name);
    editor()->layers()->notifyLayerChanged(layer);
}

void LayerRenameCommand::undo()
{
    UndoRedoCommand::undo();
    rename(oldName);
}

void LayerRenameCommand::redo()
{
    UndoRedoCommand::redo();

    // Ignore automatic redo when added to undo stack
    if (isFirstRedo()) { setFirstRedo(false); return; }

    rename(newName);
}

CameraReplaceCommand::CameraReplaceCommand(const Camera* undoCamera,
                                           const Camera* redoCamera,
                                           int layerId,
                                           const QString& description,
                                           Editor* editor,
                                           QUndoCommand* parent)
    : UndoRedoCommand(editor, parent),
      undoCamera(*undoCamera),
      redoCamera(*redoCamera)
{
    this->layerId = layerId;

    setText(description);
}

void CameraReplaceCommand::apply(const Camera& camera)
{
    Layer* layer = editor()->layers()->findLayerById(layerId);
    if (layer == nullptr || layer->type() != Layer::CAMERA) {
        return setObsolete(true);
    }

    LayerCamera* cameraLayer = static_cast<LayerCamera*>(layer);
    if (cameraLayer->getCameraAtFrame(camera.pos()) == nullptr) {
        return setObsolete(true);
    }

    cameraLayer->replaceKeyFrame(&camera);

    emit editor()->frameModified(camera.pos());
    editor()->scrubTo(camera.pos());
}

void CameraReplaceCommand::undo()
{
    UndoRedoCommand::undo();
    apply(undoCamera);
}

void CameraReplaceCommand::redo()
{
    UndoRedoCommand::redo();

    // Ignore automatic redo when added to undo stack
    if (isFirstRedo()) { setFirstRedo(false); return; }

    apply(redoCamera);
}

TransformCommand::TransformCommand(const QRectF& undoSelectionRect,
                                   const QPointF& undoTranslation,
                                   const qreal undoRotationAngle,
                                   const qreal undoScaleX,
                                   const qreal undoScaleY,
                                   const QPointF& undoTransformAnchor,
                                   const bool roundPixels,
                                   const QString& description,
                                   Editor* editor,
                                   QUndoCommand *parent) : UndoRedoCommand(editor, parent)
{
    this->roundPixels = roundPixels;

    this->undoSelectionRect = undoSelectionRect;
    this->undoAnchor = undoTransformAnchor;
    this->undoTranslation = undoTranslation;
    this->undoRotationAngle = undoRotationAngle;
    this->undoScaleX = undoScaleX;
    this->undoScaleY = undoScaleY;

    auto selectMan = editor->select();
    redoSelectionRect = selectMan->mySelectionRect();
    redoAnchor = selectMan->currentTransformAnchor();
    redoTranslation = selectMan->myTranslation();
    redoRotationAngle = selectMan->myRotation();
    redoScaleX = selectMan->myScaleX();
    redoScaleY = selectMan->myScaleY();

    setText(description);
}

void TransformCommand::undo()
{
    UndoRedoCommand::undo();
    apply(undoSelectionRect,
          undoTranslation,
          undoRotationAngle,
          undoScaleX,
          undoScaleY,
          undoAnchor,
          roundPixels);
}

void TransformCommand::redo()
{
    UndoRedoCommand::redo();

    // Ignore automatic redo when added to undo stack
    if (isFirstRedo()) { setFirstRedo(false); return; }

    apply(redoSelectionRect,
          redoTranslation,
          redoRotationAngle,
          redoScaleX,
          redoScaleY,
          redoAnchor,
          roundPixels);
}

void TransformCommand::apply(const QRectF& selectionRect,
                             const QPointF& translation,
                             const qreal rotationAngle,
                             const qreal scaleX,
                             const qreal scaleY,
                             const QPointF& selectionAnchor,
                             const bool roundPixels)
{
    auto selectMan = editor()->select();
    selectMan->setSelection(selectionRect, roundPixels);
    selectMan->setTransformAnchor(selectionAnchor);
    selectMan->setTranslation(translation);
    selectMan->setRotation(rotationAngle);
    selectMan->setScale(scaleX, scaleY);

    selectMan->calculateSelectionTransformation();
}
