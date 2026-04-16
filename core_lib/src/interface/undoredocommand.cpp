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
#include <algorithm>

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
                                       const QList<int>& collisionPositions,
                                       const QList<QPair<int, KeyFrame*>>& pastedClones,
                                       int layerId,
                                       const QString& description,
                                       Editor* editor,
                                       QUndoCommand* parent)
    : UndoRedoCommand(editor, parent)
{
    mLayerId = layerId;
    mAddedPositions = addedPositions;
    mCollisionPositions = collisionPositions;
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

    layer->deselectAll();

    // Remove in descending order to avoid coverage interactions.
    QList<int> sortedAddedPositions = mAddedPositions;
    std::sort(sortedAddedPositions.begin(), sortedAddedPositions.end(), std::greater<int>());
    for (int pos : sortedAddedPositions) {
        layer->removeKeyFrame(pos);
    }

    // Reverse collision shifts in reverse paste order.
    for (int i = mCollisionPositions.count() - 1; i >= 0; --i) {
        const int position = mCollisionPositions.at(i);
        if (!layer->newSelectionOfConnectedFrames(position + 1)) {
            continue;
        }
        layer->moveSelectedFrames(-1);
    }

    layer->deselectAll();

    emit editor()->framesModified();
    editor()->layers()->notifyAnimationLengthChanged();
}

void PasteFramesCommand::redo()
{
    UndoRedoCommand::redo();

    if (isFirstRedo()) { setFirstRedo(false); return; }

    Layer* layer = editor()->layers()->findLayerById(mLayerId);
    if (layer == nullptr) {
        setObsolete(true);
        return;
    }

    layer->deselectAll();

    // Replay the original paste behavior deterministically.
    for (auto& p : mPastedClones) {
        const int position = p.first;
        if (layer->getKeyFrameWhichCovers(position) != nullptr) {
            layer->newSelectionOfConnectedFrames(position);
            layer->moveSelectedFrames(1);
        }

        layer->addKeyFrame(p.first, p.second->clone());
        layer->setFrameSelected(position, true);
    }

    emit editor()->framesModified();
    editor()->layers()->notifyAnimationLengthChanged();
}

SetExposureCommand::SetExposureCommand(int offset,
                                       int layerId,
                                       const QList<int>& selectedByPos,
                                       const QList<int>& selectedByLast,
                                       bool hadSelectedFrames,
                                       int currentFramePos,
                                       const QString& description,
                                       Editor* editor,
                                       QUndoCommand* parent)
    : UndoRedoCommand(editor, parent)
{
    mOffset = offset;
    mLayerId = layerId;

    Layer* layer = editor->layers()->findLayerById(layerId);
    if (!layer) {
        setText(description);
        return;
    }

    // Snapshot all frame positions before the mutation
    QList<int> beforePositions;
    layer->foreachKeyFrame([&beforePositions](KeyFrame* frame) {
        beforePositions.append(frame->pos());
    });

    // Perform the mutation: restore the selection state that was active before
    // the caller delegated to us, then apply the exposure change.
    layer->deselectAll();
    for (int pos : selectedByLast) {
        layer->setFrameSelected(pos, true);
    }
    if (!hadSelectedFrames) {
        layer->setFrameSelected(currentFramePos, true);
    }

    layer->setExposureForSelectedFrames(offset);

    if (!hadSelectedFrames) {
        layer->deselectAll();
    }

    // Snapshot all frame positions after the mutation
    QList<int> afterPositions;
    layer->foreachKeyFrame([&afterPositions](KeyFrame* frame) {
        afterPositions.append(frame->pos());
    });

    // Build the moved-frames list from sorted before/after snapshots.
    // foreachKeyFrame iterates in descending key order (std::greater), so sort ascending.
    std::sort(beforePositions.begin(), beforePositions.end());
    std::sort(afterPositions.begin(), afterPositions.end());

    Q_ASSERT(beforePositions.size() == afterPositions.size());
    for (int i = 0; i < beforePositions.size(); ++i) {
        if (beforePositions[i] != afterPositions[i]) {
            mMovedFrames.append(qMakePair(beforePositions[i], afterPositions[i]));
        }
    }

    setText(description);
}

void SetExposureCommand::undo()
{
    Layer* layer = editor()->layers()->findLayerById(mLayerId);
    if (layer == nullptr) {
        return setObsolete(true);
    }

    UndoRedoCommand::undo();

    // Move each displaced frame from its after-position back to its before-position.
    // mMovedFrames pairs are (before, after). For undo we need (after -> before).
    // When undoing a positive offset (frames moved right), process in ascending
    // after-position order; for negative offset, descending after-position order.
    QList<QPair<int, int>> undoMoves;
    undoMoves.reserve(mMovedFrames.size());
    for (const auto& p : qAsConst(mMovedFrames)) {
        undoMoves.append(qMakePair(p.second, p.first)); // (after, before)
    }
    if (mOffset > 0) {
        std::sort(undoMoves.begin(), undoMoves.end(),
                  [](const QPair<int,int>& a, const QPair<int,int>& b){ return a.first < b.first; });
    } else {
        std::sort(undoMoves.begin(), undoMoves.end(),
                  [](const QPair<int,int>& a, const QPair<int,int>& b){ return a.first > b.first; });
    }

    applyPositions(layer, undoMoves);

    emit editor()->framesModified();
    editor()->layers()->notifyAnimationLengthChanged();
}

void SetExposureCommand::redo()
{
    UndoRedoCommand::redo();

    // Ignore automatic redo when first pushed onto the stack —
    // the constructor already performed the mutation.
    if (isFirstRedo()) { setFirstRedo(false); return; }

    Layer* layer = editor()->layers()->findLayerById(mLayerId);
    if (layer == nullptr) {
        return setObsolete(true);
    }

    // Move each displaced frame from its before-position to its after-position.
    // mMovedFrames pairs are (before, after). For redo we use them directly.
    // Process in descending before-position order for positive offset (moving right),
    // ascending for negative offset (moving left), to avoid collisions.
    QList<QPair<int, int>> redoMoves = mMovedFrames;
    if (mOffset > 0) {
        std::sort(redoMoves.begin(), redoMoves.end(),
                  [](const QPair<int,int>& a, const QPair<int,int>& b){ return a.first > b.first; });
    } else {
        std::sort(redoMoves.begin(), redoMoves.end(),
                  [](const QPair<int,int>& a, const QPair<int,int>& b){ return a.first < b.first; });
    }

    applyPositions(layer, redoMoves);

    emit editor()->framesModified();
    editor()->layers()->notifyAnimationLengthChanged();
}

void SetExposureCommand::applyPositions(Layer* layer, const QList<QPair<int, int>>& moves)
{
    for (const auto& p : qAsConst(moves)) {
        const int fromPos = p.first;
        const int toPos = p.second;
        if (fromPos != toPos) {
            layer->moveKeyFrame(fromPos, toPos - fromPos);
        }
    }
}

InsertExposureCommand::InsertExposureCommand(int insertPosition,
                                             int layerId,
                                             const QString& description,
                                             Editor* editor,
                                             QUndoCommand* parent)
    : UndoRedoCommand(editor, parent)
{
    mInsertPosition = insertPosition;
    mLayerId = layerId;
    mNewKeyPosition = insertPosition + 1;

    Layer* layer = editor->layers()->findLayerById(layerId);
    if (!layer) { setText(description); return; }

    // Snapshot before-positions of frames that will be shifted by insertExposureAt
    layer->foreachKeyFrame([&](KeyFrame* frame) {
        if (frame->pos() >= insertPosition + 1) {
            mShiftedPositions.append(frame->pos());
        }
    });
    std::sort(mShiftedPositions.begin(), mShiftedPositions.end());

    // Perform the mutation: shift connected frames then add the new key
    layer->insertExposureAt(insertPosition);
    layer->addNewKeyFrameAt(mNewKeyPosition);

    editor->scrubTo(mNewKeyPosition);
    emit editor->framesModified();
    editor->layers()->notifyAnimationLengthChanged();

    setText(description);
}

void InsertExposureCommand::undo()
{
    Layer* layer = editor()->layers()->findLayerById(mLayerId);
    if (layer == nullptr) { return setObsolete(true); }

    UndoRedoCommand::undo();

    layer->removeKeyFrame(mNewKeyPosition);

    // Restore shifted frames: they moved from origPos to origPos+1, so move back -1.
    // Process in ascending order (leftmost first) to avoid clobbering.
    for (int origPos : qAsConst(mShiftedPositions)) {
        layer->moveKeyFrame(origPos + 1, -1);
    }

    emit editor()->framesModified();
    editor()->layers()->notifyAnimationLengthChanged();
    editor()->scrubTo(mInsertPosition);
}

void InsertExposureCommand::redo()
{
    UndoRedoCommand::redo();

    if (isFirstRedo()) { setFirstRedo(false); return; }

    Layer* layer = editor()->layers()->findLayerById(mLayerId);
    if (layer == nullptr) { return setObsolete(true); }

    layer->insertExposureAt(mInsertPosition);
    layer->addNewKeyFrameAt(mNewKeyPosition);

    emit editor()->framesModified();
    editor()->layers()->notifyAnimationLengthChanged();
    editor()->scrubTo(mNewKeyPosition);
}

MoveFrameCommand::MoveFrameCommand(int position,
                                   int offset,
                                   int layerId,
                                   const QString& description,
                                   Editor* editor,
                                   QUndoCommand* parent)
    : UndoRedoCommand(editor, parent)
    , mLayerId(layerId)
    , mFromPos(position)
    , mOffset(offset)
{
    Layer* layer = editor->layers()->findLayerById(layerId);
    if (layer) {
        mMoved = layer->moveKeyFrame(mFromPos, mOffset);
        if (mMoved) {
            editor->scrubTo(mFromPos + mOffset);
            emit editor->framesModified();
            editor->layers()->notifyAnimationLengthChanged();
        }
    }
    setText(description);
}

void MoveFrameCommand::undo()
{
    Layer* layer = editor()->layers()->findLayerById(mLayerId);
    if (!layer) { return setObsolete(true); }
    if (!mMoved) { return; }

    UndoRedoCommand::undo();

    layer->moveKeyFrame(mFromPos + mOffset, -mOffset);
    editor()->scrubTo(mFromPos);
    emit editor()->framesModified();
    editor()->layers()->notifyAnimationLengthChanged();
}

void MoveFrameCommand::redo()
{
    UndoRedoCommand::redo();
    if (isFirstRedo()) { setFirstRedo(false); return; }

    Layer* layer = editor()->layers()->findLayerById(mLayerId);
    if (!layer) { return setObsolete(true); }
    if (!mMoved) { return; }

    layer->moveKeyFrame(mFromPos, mOffset);
    editor()->scrubTo(mFromPos + mOffset);
    emit editor()->framesModified();
    editor()->layers()->notifyAnimationLengthChanged();
}

ReverseFrameOrderCommand::ReverseFrameOrderCommand(const QList<int>& selectedFrames,
                                                   int layerId,
                                                   const QString& description,
                                                   Editor* editor,
                                                   QUndoCommand* parent)
    : UndoRedoCommand(editor, parent)
    , mLayerId(layerId)
    , mSelectedFrames(selectedFrames)
{
    Layer* layer = editor->layers()->findLayerById(layerId);
    if (layer) {
        applyReverse(layer);
        emit editor->framesModified();
    }
    setText(description);
}

void ReverseFrameOrderCommand::undo()
{
    Layer* layer = editor()->layers()->findLayerById(mLayerId);
    if (!layer) { return setObsolete(true); }

    UndoRedoCommand::undo();
    applyReverse(layer);
    emit editor()->framesModified();
}

void ReverseFrameOrderCommand::redo()
{
    UndoRedoCommand::redo();
    if (isFirstRedo()) { setFirstRedo(false); return; }

    Layer* layer = editor()->layers()->findLayerById(mLayerId);
    if (!layer) { return setObsolete(true); }

    applyReverse(layer);
    emit editor()->framesModified();
}

void ReverseFrameOrderCommand::applyReverse(Layer* layer) const
{
    // Re-establish the same selection, then reverse it.
    // reverseOrderOfSelection() is its own inverse so both undo and redo
    // call this helper with identical logic.
    layer->deselectAll();
    for (int pos : qAsConst(mSelectedFrames)) {
        layer->setFrameSelected(pos, true);
    }
    layer->reverseOrderOfSelection();
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
