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
#ifndef UNDOREDOMANAGER_H
#define UNDOREDOMANAGER_H

#include "basemanager.h"
#include "layer.h"
#include "keyframe.h"
#include "undotransaction.h"

#include "preferencesdef.h"

#include <QUndoStack>
#include <QRectF>
#include <QMap>

#include <memory>

class QAction;
class QUndoCommand;

class BitmapImage;
class VectorImage;
class Camera;
class SoundClip;
class KeyFrame;
class LegacyBackupElement;
class UndoRedoCommand;

/// The undo/redo type which correspond to what is being recorded
enum class UndoRedoRecordType {
    KEYFRAME_MODIFY, // Any modification that involve a keyframe
    KEYFRAME_REMOVE, // Removing a keyframe
    KEYFRAME_ADD, // Adding a keyframe
    KEYFRAME_MOVE,
    // SCRUB_LAYER, // Scrubbing layer
    // SCRUB_KEYFRAME, // Scrubbing keyframe
    INVALID
};

struct SelectionSaveState {

    SelectionSaveState() = default;
    SelectionSaveState(const QRectF& rect,
                       const qreal rotationAngle,
                       const qreal scaleX,
                       const qreal scaleY,
                       const QPointF& translation,
                       const QPointF& anchor)
    {
        this->bounds = rect;
        this->rotationAngle = rotationAngle;
        this->scaleX = scaleX;
        this->scaleY = scaleY;
        this->translation = translation;
        this->anchor = anchor;
    }

    QRectF  bounds;
    qreal   rotationAngle = 0.0;
    qreal   scaleX = 0.0;
    qreal   scaleY = 0.0;
    QPointF translation;
    QPointF anchor;
};

struct MoveFramesSaveState {

    MoveFramesSaveState() = default;
    MoveFramesSaveState(int offset,
                        const QList<int>& positions)
    {
        this->offset = offset;
        this->positions = positions;
    }

    int offset = 0;
    QList<int> positions;
};

/// Use this struct to store user related data that will later be added to the backup
/// This struct is meant to be safely shared and stored temporarily,
/// as such don't store ptrs here...
/// All data stored in here should be based on ZII (zero is initialization) principle
/// Only store what you need.
struct UserSaveState {
    MoveFramesSaveState moveFramesState = {};
};

/// This is the main undo/redo state structure which is meant to populate
/// whatever states that needs to be stored temporarily.
struct UndoSaveState {
    // Common data
    UndoRedoRecordType recordType = UndoRedoRecordType::INVALID;
    int layerId = 0;
    /// The frame position the state was captured at.
    int frameIndex = 0;
    Layer::LAYER_TYPE layerType = Layer::UNDEFINED;
    std::unique_ptr<KeyFrame> keyframe;
    SelectionSaveState selectionState = {};

    UserSaveState userState = {};
};

class UndoRedoManager : public BaseManager
{
    Q_OBJECT

public:
    explicit UndoRedoManager(Editor* editor);
    ~UndoRedoManager() override;

    bool init() override;
    Status load(Object*) override;
    Status save(Object*) override;

    /** Begins an undo transaction, capturing the before-state of the given
     *  layer at the given frame position.
     *
     *  Returns an inactive transaction when the layer doesn't exist or the
     *  new undo/redo system is disabled — commit() is then a no-op, so call
     *  sites don't need to check.
     */
    UndoTransaction beginTransaction(UndoRedoRecordType recordType, int layerId, int framePosition);

    /// Convenience overload capturing the current layer and frame.
    UndoTransaction beginTransaction(UndoRedoRecordType recordType);

    /** Checks whether there are unsaved changes.
     *  @return true if there are unsaved changes, otherwise false */
    bool hasUnsavedChanges() const;

    /** Overrides the backup-system choice read from the user preference at
     *  init(). Switching mid-session strands the other system's history, so
     *  only call this before any editing has been recorded (used by tests). */
    void setNewBackupSystemEnabled(bool enabled) { mNewBackupSystemEnabled = enabled; }

    /** True when the experimental QUndoStack-based system is active. */
    bool isNewBackupSystemEnabled() const { return mNewBackupSystemEnabled; }

    /** Pushes a ready-made command onto the undo stack. Use for operations
     *  (e.g. layer structure changes) that don't need before/after keyframe
     *  capture; keyframe edits go through beginTransaction() instead.
     *  Deletes the command without recording when the new system is off. */
    void push(QUndoCommand* command);

    /** Groups every command recorded until endMacro() into a single undo
     *  step. Used for operations that modify several keyframes at once.
     *  If nothing is recorded in between, no undo entry is created.
     *  Does not support nesting. No-op while the new system is disabled. */
    void beginMacro(const QString& text);
    void endMacro();

    /** Called by undo commands after they have applied a change, naming the
     *  affected layer and frame. Commands report *what* changed through
     *  this; how the UI reacts (e.g. navigating there) is decided by the
     *  commandExecuted subscribers, not by the commands. */
    void notifyCommandExecuted(int layerId, int framePosition) { emit commandExecuted(layerId, framePosition); }

    QAction* createUndoAction(QObject* parent, const QIcon& icon);
    QAction* createRedoAction(QObject* parent, const QIcon& icon);

    void updateUndoAction(QAction* undoAction);
    void updateRedoAction(QAction* redoAction);

    /** Clears the undo stack */
    void clearStack();

    // Developer note:
    // Our legacy undo/redo system is not meant to be build upon anymore.
    // The implementation should however be kept until the new undo/redo system takes over.

    void legacyBackup(const QString& undoText);
    bool legacyBackup(int backupLayer, int backupFrame, const QString& undoText);
    /**
     * Restores integrity of the backup elements after a layer has been deleted.
     * Removes backup elements affecting the deleted layer and adjusts the layer
     * index on other backup elements as necessary.
     *
     * @param layerIndex The index of the layer that was deleted
     *
     * @warning This serves as a temporary hack to prevent crashes until #864 is
     *          done (see #1412).
     */
    void sanitizeLegacyBackupElementsAfterLayerDeletion(int layerIndex);
    void restoreLegacyKey();

    void rememberLastModifiedFrame(int layerNumber, int frameNumber);

    void onSettingChanged(SETTING setting);

signals:
    void didUpdateUndoStack();

    /** An undo command applied a change to the given layer and frame. */
    void commandExecuted(int layerId, int framePosition);

private:
    friend class UndoTransaction;

    /** Records the given save state as an undo command and pushes it onto
     *  the stack. Called by UndoTransaction::commit(). */
    void record(std::unique_ptr<UndoSaveState> state, const QString& description);

    void replaceKeyFrame(const UndoSaveState& undoState, const QString& description);
    void replaceBitmap(const UndoSaveState& undoState, const QString& description);
    void replaceVector(const UndoSaveState& undoState, const QString& description);
    void replaceCamera(const UndoSaveState& undoState, const QString& description);

    void addKeyFrame(const UndoSaveState& undoState, const QString& description);
    void removeKeyFrame(const UndoSaveState& undoState, const QString& description);
    void moveKeyFrames(const UndoSaveState& undoState, const QString& description);

    void initCommonKeyFrameState(UndoSaveState* undoSaveState, const Layer* layer, int frameIndex) const;

    void pushCommand(QUndoCommand* command);

    void legacyUndo();
    void legacyRedo();

    QUndoStack mUndoStack;

    // Legacy system
    int mLegacyBackupIndex = -1;
    LegacyBackupElement* mLegacyBackupAtSave = nullptr;
    QList<LegacyBackupElement*> mLegacyBackupList;

    int mLegacyLastModifiedLayer = -1;
    int mLegacyLastModifiedFrame = -1;

    /// A max-steps value waiting to be applied at the next clearStack();
    /// -1 when nothing is pending.
    int mPendingUndoLimit = -1;

    /// Lazily-opened macro state, see beginMacro().
    QString mPendingMacroText;
    bool mMacroPending = false;
    bool mMacroStarted = false;

    bool mNewBackupSystemEnabled = false;
};

#endif // UNDOREDOMANAGER_H
