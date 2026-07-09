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
#include "undotransaction.h"

#include "undoredomanager.h"

UndoTransaction::UndoTransaction(UndoRedoManager* manager, std::unique_ptr<UndoSaveState> state)
    : mManager(manager), mSaveState(std::move(state))
{
}

// Defined here because UndoSaveState is incomplete in the header.
UndoTransaction::~UndoTransaction() = default;
UndoTransaction::UndoTransaction(UndoTransaction&& other) noexcept = default;
UndoTransaction& UndoTransaction::operator=(UndoTransaction&& other) noexcept = default;

void UndoTransaction::setUserState(const UserSaveState& userState)
{
    if (mSaveState)
    {
        mSaveState->userState = userState;
    }
}

void UndoTransaction::commit(const QString& description)
{
    if (mSaveState == nullptr || mManager == nullptr)
    {
        return;
    }
    mManager->record(std::move(mSaveState), description);
    mManager = nullptr;
}

void UndoTransaction::discard()
{
    mSaveState.reset();
    mManager = nullptr;
}
