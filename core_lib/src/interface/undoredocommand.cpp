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
#include "undoredomanager.h"
#include "undoredocommand.h"

UndoRedoCommand::UndoRedoCommand(Editor* editor, QUndoCommand* parent) : QUndoCommand(parent)
{
    qDebug() << "backupElement created";
    mEditor = editor;
}

namespace
{
/** Clones a stored keyframe for (re-)insertion into the given layer.
 *  Sound clips get their media player recreated the way the legacy restore
 *  path does. Returns nullptr when restoring isn't possible (e.g. the
 *  sound file is gone). */
KeyFrame* cloneKeyFrameForLayer(const KeyFrame* storedKeyFrame, const Layer* layer, Editor* editor)
{
    KeyFrame* restoredKey = storedKeyFrame->clone();
    if (layer->type() == Layer::SOUND)
    {
        SoundClip* clip = static_cast<SoundClip*>(restoredKey);
        const QString soundFile = clip->fileName();
        if (soundFile.isEmpty() || !QFile::exists(soundFile))
        {
            delete clip;
            return nullptr;
        }

        Status status = editor->sound()->loadSound(clip, soundFile);
        if (!status.ok())
        {
            // loadSound only deletes the clip when it fails past its file
            // checks; on its FILE_NOT_FOUND/FAIL early returns the clip is
            // still ours to free.
            if (status == Status::FILE_NOT_FOUND || status == Status::FAIL)
            {
                delete clip;
            }
            return nullptr;
        }
    }
    return restoredKey;
}
} // namespace

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

    KeyFrame* restoredKey = cloneKeyFrameForLayer(undoKeyFrame, layer, editor());
    if (restoredKey == nullptr)
    {
        // e.g. the underlying sound file is gone — drop the command
        // instead of inserting a broken keyframe.
        return setObsolete(true);
    }

    if (!layer->addKeyFrame(undoKeyFrame->pos(), restoredKey))
    {
        // The position is occupied — addKeyFrame doesn't take ownership
        // on failure.
        delete restoredKey;
        return setObsolete(true);
    }

    emit editor()->frameModified(undoKeyFrame->pos());
    editor()->layers()->notifyAnimationLengthChanged();
    editor()->undoRedo()->notifyCommandExecuted(layerId, undoKeyFrame->pos());
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
    editor()->undoRedo()->notifyCommandExecuted(layerId, redoPosition);
}

KeyFrameAddCommand::KeyFrameAddCommand(int position,
                                       int layerId,
                                       const KeyFrame* addedKeyFrame,
                                       const QString &description,
                                       Editor *editor,
                                       QUndoCommand *parent)
    : UndoRedoCommand(editor, parent)
{
    this->position = position;
    this->layerId = layerId;
    if (addedKeyFrame != nullptr)
    {
        this->keyClone.reset(addedKeyFrame->clone());
    }

    setText(description);
}

KeyFrameAddCommand::~KeyFrameAddCommand() = default;

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
    editor()->undoRedo()->notifyCommandExecuted(layerId, position);
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

    if (keyClone != nullptr)
    {
        KeyFrame* restoredKey = cloneKeyFrameForLayer(keyClone.get(), layer, editor());
        if (restoredKey == nullptr)
        {
            return setObsolete(true);
        }
        if (!layer->addKeyFrame(position, restoredKey))
        {
            // The position is occupied — addKeyFrame doesn't take
            // ownership on failure.
            delete restoredKey;
            return setObsolete(true);
        }
    }
    else
    {
        layer->addNewKeyFrameAt(position);
    }

    emit editor()->frameModified(position);
    editor()->layers()->notifyAnimationLengthChanged();
    editor()->undoRedo()->notifyCommandExecuted(layerId, position);
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

    // Rebuild the selection from the recorded state: the moved frames sit
    // at their post-move positions now, and whatever the live selection is
    // must not leak into the move.
    undoLayer->deselectAll();
    for (int position : qAsConst(positions)) {
        undoLayer->setFrameSelected(position + frameOffset, true);
    }
    undoLayer->moveSelectedFrames(-frameOffset);
    undoLayer->deselectAll();

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

    redoLayer->deselectAll();
    for (int position : qAsConst(positions)) {
        redoLayer->setFrameSelected(position, true);
    }

    redoLayer->moveSelectedFrames(frameOffset);
    redoLayer->deselectAll();

    emit editor()->framesModified();
}
BitmapReplaceCommand::BitmapReplaceCommand(const BitmapImage& undoSubImage,
                             const BitmapImage& redoSubImage,
                             const int layerId,
                             const int framePosition,
                             const QString& description,
                             Editor *editor,
                             QUndoCommand *parent) : UndoRedoCommand(editor, parent)
{
    this->undoSubImage = undoSubImage;
    this->redoSubImage = redoSubImage;
    this->layerId = layerId;
    this->framePosition = framePosition;

    setText(description);
}

void BitmapReplaceCommand::applySubImage(const BitmapImage& subImage)
{
    Layer* layer = editor()->layers()->findLayerById(layerId);
    if (layer == nullptr || layer->type() != Layer::BITMAP) {
        return setObsolete(true);
    }

    BitmapImage* liveImage = static_cast<LayerBitmap*>(layer)->getLastBitmapImageAtFrame(framePosition);
    if (liveImage == nullptr) {
        return setObsolete(true);
    }

    // CompositionMode_Source overwrites the changed region outright,
    // including writing transparency back — which is how undoing a stroke
    // erases it. Outside the sub-image the two states were identical.
    BitmapImage sourcePatch = subImage; // paste() needs a mutable image
    if (!sourcePatch.bounds().isEmpty())
    {
        liveImage->paste(&sourcePatch, QPainter::CompositionMode_Source);
    }

    editor()->undoRedo()->notifyCommandExecuted(layerId, framePosition);
}

void BitmapReplaceCommand::undo()
{
    UndoRedoCommand::undo();

    applySubImage(undoSubImage);
}

void BitmapReplaceCommand::redo()
{
    UndoRedoCommand::redo();

    // Ignore automatic redo when added to undo stack
    if (isFirstRedo()) { setFirstRedo(false); return; }

    applySubImage(redoSubImage);
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

    editor()->undoRedo()->notifyCommandExecuted(layerId, undoVector.pos());
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

    editor()->undoRedo()->notifyCommandExecuted(layerId, redoVector.pos());
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
    editor()->undoRedo()->notifyCommandExecuted(layerId, camera.pos());
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
    editor()->select()->restoreSelectionState(selectionRect,
                                              translation,
                                              rotationAngle,
                                              scaleX,
                                              scaleY,
                                              selectionAnchor,
                                              roundPixels);
}
