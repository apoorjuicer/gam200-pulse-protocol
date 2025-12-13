/******************************************************************************/
/**
 * @file        UndoManager.cpp
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin
 * @brief       Implementation of class UndoManager member functions
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#include "UndoManager.h"
#include <iostream>
void UndoManager::Push(Command action) {
	if (!action) { //no action early return
		return;
	}
	// Enforce max history length
	// drop the oldest "step" if full
	if (undo_stack.size() == MAX_HISTORY) {
		//erase the first element in the container
		//std::cout << "removing oldest element\n";
		undo_stack.erase(undo_stack.begin());
	}
	//add most recent "action" or "step"
	//to the back of the undo_stack
	//std::cout << "[UndoManager] Push, size before = " << undo_stack.size() << "\n";
	undo_stack.push_back(std::move(action));
	//std::cout << "[UndoManager] size after = " << undo_stack.size() << "\n";
}
void  UndoManager::Undo()
{
	if (undo_stack.empty()) {
		return;
	}
	Command cmd = std::move(undo_stack.back());
	undo_stack.pop_back();
	// Execute the undo action
	cmd();
	//std::cout << "[UndoManager] size after undo = " << undo_stack.size() << "\n";
}
//checks whether the undo stack is empty
//returns true if we can undo
bool  UndoManager::CanUndo() const
{
	return !undo_stack.empty();
}
void  UndoManager::Clear() //clear the stack when changing scenes
{
	undo_stack.clear();
}
UndoManager::operator bool() const noexcept {
	return !undo_stack.empty();  //
}
size_t UndoManager::Capacity() const noexcept 
{ 
	return MAX_HISTORY;
}