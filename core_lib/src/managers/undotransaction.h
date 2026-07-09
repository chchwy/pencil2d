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
#ifndef UNDOTRANSACTION_H
#define UNDOTRANSACTION_H

#include <memory>

#include <QString>

class UndoRedoManager;
struct UndoSaveState;
struct UserSaveState;

/** RAII guard for recording one undoable change.
 *
 *  Created via UndoRedoManager::beginTransaction(), the guard captures the
 *  before-state of a specific layer and frame at construction time. After
 *  performing the change, call commit() to capture the after-state and push
 *  an undo command onto the stack.
 *
 *  A transaction that goes out of scope without commit() — a cancelled
 *  stroke, an early return, a tool switch mid-gesture — silently discards
 *  its captured state. Nothing is pushed and nothing leaks.
 */
class UndoTransaction final
{
public:
    /// Inactive transaction; commit() and discard() are no-ops.
    UndoTransaction() = default;
    ~UndoTransaction();

    UndoTransaction(UndoTransaction&& other) noexcept;
    UndoTransaction& operator=(UndoTransaction&& other) noexcept;
    UndoTransaction(const UndoTransaction&) = delete;
    UndoTransaction& operator=(const UndoTransaction&) = delete;

    /// True while holding uncommitted before-state.
    bool isActive() const { return mSaveState != nullptr; }

    /// Attaches user data (e.g. frame move offsets) to the pending state.
    void setUserState(const UserSaveState& userState);

    /// Captures the after-state and pushes the undo command. Deactivates
    /// the transaction; further calls are no-ops.
    void commit(const QString& description);

    /// Drops the pending state without recording anything.
    void discard();

private:
    friend class UndoRedoManager;
    UndoTransaction(UndoRedoManager* manager, std::unique_ptr<UndoSaveState> state);

    UndoRedoManager* mManager = nullptr;
    std::unique_ptr<UndoSaveState> mSaveState;
};

#endif // UNDOTRANSACTION_H
