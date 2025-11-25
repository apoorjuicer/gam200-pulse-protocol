/******************************************************************************/
/**
 * @file        InputManager.h
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Raw input system which handles just the tracking of
				keyboard and mouse state tracking.
 *              Separated from game logic - just reads input states.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#pragma once
#include "pch/pch_temp.h"
#include "../MyEngine/CoreEngine/ECS/System.h"
#include "../MyEngine/CoreEngine/Message/MessageManager.h"

class InputManager : public Systems {
public:
	InputManager();
	~InputManager();

	void Initialize(GLFWwindow* window);
	void Update(float);
	void Shutdown();

	// -------------------------------------------------
	// Keyboard: 3 states [IsPressed, IsTriggered, IsReleased]
	// -------------------------------------------------
	bool isKeyTriggered(int key) const;
	bool isKeyReleased(int key) const;
	bool isKeyPressed(int key) const;

	// -------------------------------------------------
	// Mouse: 3 states [IsPressed, IsTriggered, IsReleased] + Getting pos of mouse
	// -------------------------------------------------
	bool isMouseButtonPressed(int button) const;
	bool isMouseButtonTriggered(int button) const;
	bool isMouseButtonReleased(int button) const;


	glm::vec2 getMousePosition() const;

	glm::vec2 getMousePositionVirtual() const { return mousePositionVirtual; }

	bool isMouseInGameViewport() const { return mouseInGameViewport; }


	unsigned int getLastCharTyped() const { return lastCharTyped; }
	void pushChar(unsigned int c) { lastCharTyped = c; }
	void clearLastChar() { lastCharTyped = 0; }

	// -------------------------------------------------
	// Debug toggles
	// -------------------------------------------------
	bool isDebugRectsEnabled() const { return debugRectsEnabled; }
	bool isDebugLinesEnabled() const { return debugLinesEnabled; }
	bool isDebugCirclesEnabled() const { return debugCirclesEnabled; }
	bool isDebugCirclesFilledEnabled() const { return debugCirclesFilledEnabled; }
	bool isDebugPointsEnabled() const { return debugPointsEnabled; }
	bool isDebugGridEnabled() const { return debugGridEnabled; }
	bool ConsumeStressToggleRequest() {
		bool out = stressToggleRequested;
		stressToggleRequested = false;
		return out;
	}
	void SetWindow(GLFWwindow* w) { window = w; }
	std::set<Entity> DebugEntities;

private:
	GLFWwindow* window = nullptr;
	unsigned int lastCharTyped = 0;

	// Keyboard state
	bool currentKeys[GLFW_KEY_LAST + 1];
	bool previousKeys[GLFW_KEY_LAST + 1];
	bool keyRepeat[GLFW_KEY_LAST + 1] = { false };
	bool isKeyRepeating(int key) const { return keyRepeat[key]; }


	// Mouse state
	bool currentMouseButtons[8];
	bool previousMouseButtons[8];
	glm::vec2 mousePosition;

	glm::vec2 mousePositionVirtual{ 0.0f };    // 0..VIRTUAL_W/H
	bool      mouseInGameViewport{ false };    // inside letterboxed viewport?

	// Debug
	bool debugRectsEnabled = false;
	bool debugLinesEnabled = false;
	bool debugCirclesEnabled = false;
	bool debugCirclesFilledEnabled = false;
	bool debugPointsEnabled = false;
	bool debugGridEnabled = false;
	bool debugToggleKeyPressed[6] = { false };
	bool stressToggleRequested = false;

	// for raw input functions
	void updateInputStates();
	void processDebugToggles();
};