/******************************************************************************/
/**
 * @file        UndoManager.h
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin
 * @brief       Undo Manager contains a "stack" of actions or steps that can be
 *				undo/reverted up to a maximum of 20 steps will be stored in
 *				MAX_HISTORY
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#pragma once
#include <vector>
#include <functional>
class UndoManager {
public:
	//std::function -> general purpose polymorphic function wrapper.
	//A better function ptor that can store ANY callable object
	//To be called when we need to undo
	using Command = std::function<void()>;
	void Push(Command action);
	void Undo();
	//checks whether the undo stack is empty
	//returns true if we can undo
	bool CanUndo() const;
	void Clear(); //clear the stack when changing scenes
	operator bool() const noexcept;
	size_t Capacity() const noexcept;
private:
	//keep track of history for undo
	static constexpr size_t MAX_HISTORY = 20;
	// stack of undo commands (LIFO)
	// Last-In-First-Out, store the previous step
	std::vector<Command> undo_stack;

};