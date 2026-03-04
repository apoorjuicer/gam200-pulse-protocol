/******************************************************************************/
/**
 * @file        UndoManager.cpp
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin
 * @brief       Implementation of class UndoManager member functions
 *
 * @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#include "UndoManager.h"
#include <iostream>
#include <utility>

void UndoManager::TrimHistory()
{
	if (undo_stack.size() > MAX_HISTORY) {
		undo_stack.erase(undo_stack.begin(),
			undo_stack.begin() + (undo_stack.size() - MAX_HISTORY));
	}
}

void UndoManager::Push(Action a) {
	if (m_applying) return;  // block re-entrant pushes during undo/redo
	// both must exist undo + redo
	if (!a.undo || !a.redo) { //no action early return
		return;
	}
	//
	// new action invalidates redo history
	redo_stack.clear();
	//add most recent "action" or "step"
	//to the back of the undo_stack
	//std::cout << "[UndoManager] Push, size before = " << undo_stack.size() << "\n";
	undo_stack.push_back(std::move(a));
	TrimHistory();
	//std::cout << "[UndoManager] size after = " << undo_stack.size() << "\n";
}
void UndoManager::Undo()
{

	if (undo_stack.empty()) {
		return;
	}
	Action a = std::move(undo_stack.back());
	undo_stack.pop_back();

    // undo the action
	m_applying = true;
    a.undo();
	m_applying = false;
	redo_stack.push_back(std::move(a)); // now redo-able
	//std::cout << "[UndoManager] size after undo = " << undo_stack.size() << "\n";
}

void UndoManager::Redo()
{
    if (redo_stack.empty())
        return;

    Action a = std::move(redo_stack.back());
	m_applying = true;
    redo_stack.pop_back();
	m_applying = false;
    // redo the action
	if (!a.redo) {
		std::cout << "[UndoManager] ERROR: redo is empty\n";
		return;
	}

	a.redo();                      // do redo
	undo_stack.push_back(std::move(a)); // now undo-able again
}

//checks whether the undo stack is empty
//returns true if we can undo
bool  UndoManager::CanUndo() const
{
	return !undo_stack.empty();
}
bool UndoManager::CanRedo() const
{
    return !redo_stack.empty();
}
void  UndoManager::ClearUndo() //clear the Undo stack when changing scenes
{
	undo_stack.clear();
}
void UndoManager::ClearRedo()
{
    redo_stack.clear();
}
UndoManager::operator bool() const noexcept {
	//return !undo_stack.empty();
	return CanUndo() || CanRedo();
}
size_t UndoManager::Capacity() const noexcept 
{ 
	return MAX_HISTORY;
}

