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

#include "layermanager.h"
#include "selectionmanager.h"

#include "layersound.h"
#include "layerbitmap.h"
#include "layervector.h"
#include "layer.h"

#include "editor.h"
#include "object.h"
#include "undoredocommand.h"

UndoRedoCommand::UndoRedoCommand(Editor* editor, QUndoCommand* parent) : QUndoCommand(parent)
{
    qDebug() << "backupElement created";
    mEditor = editor;
}

KeyFrameRemoveCommand::KeyFrameRemoveCommand(const KeyFrame* undoKeyFrame,
                                         int undoLayerId,
                                         const QString &description,
                                         Editor *editor,
                                         QUndoCommand *parent) : UndoRedoCommand(editor, parent)
{
    this->undoKeyFrame = undoKeyFrame->clone();
    this->undoLayerId = undoLayerId;

    this->redoLayerId = editor->layers()->currentLayer()->id();
    this->redoPosition = editor->currentFrame();

    setText(description);
}

KeyFrameRemoveCommand::~KeyFrameRemoveCommand()
{
    delete undoKeyFrame;
}

void KeyFrameRemoveCommand::undo()
{
    Layer* layer = editor()->layers()->findLayerById(undoLayerId);
    if (layer == nullptr) {
        // Until we support layer deletion recovery, we mark the command as
        // obsolete as soon as it's been
        return setObsolete(true);
    }

    UndoRedoCommand::undo();

    layer->addKeyFrame(undoKeyFrame->pos(), undoKeyFrame->clone());

    emit editor()->frameModified(undoKeyFrame->pos());
    editor()->layers()->notifyAnimationLengthChanged();
    editor()->scrubTo(undoKeyFrame->pos());
}

void KeyFrameRemoveCommand::redo()
{
    Layer* layer = editor()->layers()->findLayerById(redoLayerId);
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

KeyFrameAddCommand::KeyFrameAddCommand(int undoPosition,
                                       int undoLayerId,
                                       const QString &description,
                                       Editor *editor,
                                       QUndoCommand *parent)
    : UndoRedoCommand(editor, parent)
{
    this->undoPosition = undoPosition;
    this->undoLayerId = undoLayerId;

    this->redoLayerId = editor->layers()->currentLayer()->id();
    this->redoPosition = editor->currentFrame();

    setText(description);
}

void KeyFrameAddCommand::undo()
{
    Layer* layer = editor()->layers()->findLayerById(undoLayerId);
    if (!layer) {
        return setObsolete(true);
    }

    UndoRedoCommand::undo();

    layer->removeKeyFrame(undoPosition);

    emit editor()->frameModified(undoPosition);
    editor()->layers()->notifyAnimationLengthChanged();
    editor()->layers()->setCurrentLayer(layer);
    editor()->scrubTo(undoPosition);
}

void KeyFrameAddCommand::redo()
{
    Layer* layer = editor()->layers()->findLayerById(redoLayerId);
    if (!layer) {
        return setObsolete(true);
    }

    UndoRedoCommand::redo();

    // Ignore automatic redo when added to undo stack
    if (isFirstRedo()) { setFirstRedo(false); return; }

    layer->addNewKeyFrameAt(redoPosition);

    emit editor()->frameModified(redoPosition);
    editor()->layers()->notifyAnimationLengthChanged();
    editor()->layers()->setCurrentLayer(layer);
    editor()->scrubTo(redoPosition);
}

MoveKeyFramesCommand::MoveKeyFramesCommand(int offset,
                                         QList<int> listOfPositions,
                                         int undoLayerId,
                                         const QString& description,
                                         Editor* editor,
                                         QUndoCommand *parent)
    : UndoRedoCommand(editor, parent)
{
    this->frameOffset = offset;
    this->positions = listOfPositions;

    this->undoLayerId = undoLayerId;
    this->redoLayerId = editor->layers()->currentLayer()->id();

    setText(description);
}

void MoveKeyFramesCommand::undo()
{
    Layer* undoLayer = editor()->layers()->findLayerById(undoLayerId);

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
    Layer* redoLayer = editor()->layers()->findLayerById(redoLayerId);

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
RemoveKeyFramesCommand::RemoveKeyFramesCommand(const QList<int>& positions,
                                               int layerId,
                                               const QString& description,
                                               Editor* editor,
                                               QUndoCommand* parent)
    : UndoRedoCommand(editor, parent)
{
    mLayerId = layerId;

    Layer* layer = editor->layers()->findLayerById(layerId);
    if (layer) {
        for (int pos : positions) {
            KeyFrame* frame = layer->getKeyFrameAt(pos);
            if (frame) {
                mFrames.append(qMakePair(pos, frame->clone()));
            }
        }
    }

    setText(description);
}

RemoveKeyFramesCommand::~RemoveKeyFramesCommand()
{
    for (auto& pair : mFrames) {
        delete pair.second;
    }
}

void RemoveKeyFramesCommand::undo()
{
    Layer* layer = editor()->layers()->findLayerById(mLayerId);
    if (layer == nullptr) {
        return setObsolete(true);
    }

    UndoRedoCommand::undo();

    for (const auto& pair : qAsConst(mFrames)) {
        layer->addKeyFrame(pair.first, pair.second->clone());
    }

    emit editor()->framesModified();
    editor()->layers()->notifyAnimationLengthChanged();
}

void RemoveKeyFramesCommand::redo()
{
    Layer* layer = editor()->layers()->findLayerById(mLayerId);
    if (layer == nullptr) {
        return setObsolete(true);
    }

    UndoRedoCommand::redo();

    if (isFirstRedo()) { setFirstRedo(false); return; }

    for (const auto& pair : qAsConst(mFrames)) {
        layer->removeKeyFrame(pair.first);
    }

    emit editor()->framesModified();
}

BitmapReplaceCommand::BitmapReplaceCommand(const BitmapImage* undoBitmap,
                             const int undoLayerId,
                             const QString& description,
                             Editor *editor,
                             QUndoCommand *parent) : UndoRedoCommand(editor, parent)
{

    this->undoBitmap = *undoBitmap;
    this->undoLayerId = undoLayerId;

    Layer* layer = editor->layers()->currentLayer();
    redoLayerId = layer->id();
    redoBitmap = *static_cast<LayerBitmap*>(layer)->
            getLastBitmapImageAtFrame(editor->currentFrame());

    setText(description);
}

void BitmapReplaceCommand::undo()
{
    Layer* layer = editor()->layers()->findLayerById(undoLayerId);
    if (!layer) {
        return setObsolete(true);
    }

    UndoRedoCommand::undo();

    static_cast<LayerBitmap*>(layer)->replaceKeyFrame(&undoBitmap);

    editor()->scrubTo(undoBitmap.pos());
}

void BitmapReplaceCommand::redo()
{
    Layer* layer = editor()->layers()->findLayerById(redoLayerId);
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
                                   const int undoLayerId,
                                   const QString& description,
                                   Editor* editor,
                                   QUndoCommand* parent) : UndoRedoCommand(editor, parent)
{

    this->undoVector = *undoVector;
    this->undoLayerId = undoLayerId;
    Layer* layer = editor->layers()->currentLayer();
    redoLayerId = layer->id();
    redoVector = *static_cast<LayerVector*>(layer)->
            getLastVectorImageAtFrame(editor->currentFrame(), 0);

    setText(description);
}

void VectorReplaceCommand::undo()
{
    Layer* layer = editor()->layers()->findLayerById(undoLayerId);
    if (!layer) {
        return setObsolete(true);
    }

    UndoRedoCommand::undo();

    static_cast<LayerVector*>(layer)->replaceKeyFrame(&undoVector);

    editor()->scrubTo(undoVector.pos());
}

void VectorReplaceCommand::redo()
{
    Layer* layer = editor()->layers()->findLayerById(redoLayerId);
    if (!layer) {
        return setObsolete(true);
    }

    UndoRedoCommand::redo();

    // Ignore automatic redo when added to undo stack
    if (isFirstRedo()) { setFirstRedo(false); return; }

    static_cast<LayerVector*>(layer)->replaceKeyFrame(&redoVector);

    editor()->scrubTo(redoVector.pos());
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

DeleteLayerCommand::DeleteLayerCommand(int layerIndex,
                                       int layerId,
                                       const QString& description,
                                       Editor* editor,
                                       QUndoCommand* parent)
    : UndoRedoCommand(editor, parent)
{
    mLayerIndex = layerIndex;
    mLayerId = layerId;

    // Take ownership of the layer now so it isn't destroyed by the caller
    mLayer = editor->object()->takeLayer(layerId);

    setText(description);
}

DeleteLayerCommand::~DeleteLayerCommand()
{
    delete mLayer;
}

void DeleteLayerCommand::undo()
{
    UndoRedoCommand::undo();

    // Re-insert the layer at its original position (preserving its ID)
    editor()->object()->insertLayerAt(mLayerIndex, mLayer);
    mLayer = nullptr; // ownership transferred back to the Object

    emit editor()->layers()->layerCountChanged(editor()->layers()->count());
    editor()->layers()->setCurrentLayer(mLayerIndex);
}

void DeleteLayerCommand::redo()
{
    UndoRedoCommand::redo();

    // Skip the automatic redo call when first pushed onto the stack —
    // the caller already performed the deletion before pushing this command
    if (isFirstRedo()) { setFirstRedo(false); return; }

    LayerManager* layerMgr = editor()->layers();

    // Adjust the current layer index before removing, mirroring LayerManager::deleteLayer
    if (mLayerIndex == editor()->object()->getLayerCount() - 1 &&
        mLayerIndex == layerMgr->currentLayerIndex())
    {
        layerMgr->setCurrentLayer(layerMgr->currentLayerIndex() - 1);
    }

    mLayer = editor()->object()->takeLayer(mLayerId);

    if (mLayerIndex >= layerMgr->currentLayerIndex())
    {
        layerMgr->setCurrentLayer(layerMgr->currentLayerIndex());
    }

    emit layerMgr->layerDeleted(mLayerIndex);
    emit layerMgr->layerCountChanged(layerMgr->count());
}

PasteFramesCommand::PasteFramesCommand(const QList<int>& addedPositions,
                                       const QList<int>& displacedOriginalPositions,
                                       const QList<QPair<int, KeyFrame*>>& pastedClones,
                                       int layerId,
                                       const QString& description,
                                       Editor* editor,
                                       QUndoCommand* parent)
    : UndoRedoCommand(editor, parent)
{
    mLayerId = layerId;
    mAddedPositions = addedPositions;
    mDisplacedOrigPositions = displacedOriginalPositions;
    mPastedClones = pastedClones;

    setText(description);
}

PasteFramesCommand::~PasteFramesCommand()
{
    for (auto& p : mPastedClones) {
        delete p.second;
    }
}

void PasteFramesCommand::undo()
{
    UndoRedoCommand::undo();

    Layer* layer = editor()->layers()->findLayerById(mLayerId);
    if (layer == nullptr) {
        setObsolete(true);
        return;
    }

    // Remove all newly added frames
    for (int pos : mAddedPositions) {
        layer->removeKeyFrame(pos);
    }

    // Move displaced frames back: they are now at origPos + 1, so move -1
    // Note: if multiple clipboard frames collided with the same displaced group,
    // mDisplacedOrigPositions may contain duplicates, which would misapply offsets.
    for (int origPos : mDisplacedOrigPositions) {
        layer->moveKeyFrame(origPos + 1, -1);
    }

    emit editor()->framesModified();
    editor()->layers()->notifyAnimationLengthChanged();
}

void PasteFramesCommand::redo()
{
    if (isFirstRedo()) { setFirstRedo(false); return; }

    Layer* layer = editor()->layers()->findLayerById(mLayerId);
    if (layer == nullptr) {
        setObsolete(true);
        return;
    }

    // Move displaced frames forward
    for (int origPos : mDisplacedOrigPositions) {
        layer->moveKeyFrame(origPos, 1);
    }

    // Re-add pasted frames using stored clones
    for (auto& p : mPastedClones) {
        layer->addKeyFrame(p.first, p.second->clone());
    }

    emit editor()->framesModified();
    editor()->layers()->notifyAnimationLengthChanged();
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
