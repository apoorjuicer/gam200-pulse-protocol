/******************************************************************************/
/**
 * @file        InputManager.cpp
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Implements the InputManager system, handling GLFW-based
 *              keyboard input for rhythm gameplay. Provides real-time input
 *              state tracking, combo detection, rhythm timing feedback,
 *              event broadcasting to subscribed systems, debug toggles.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "pch/pch_temp.h"
#include "graphics/stb_image.h"
#include "../MyEngine/Input/InputManager.h"
#include "../MyEngine/CoreEngine/ECS/Coordinator.h"

InputManager::InputManager() : window(nullptr) {
	std::memset(currentKeys, 0, sizeof(currentKeys));
	std::memset(previousKeys, 0, sizeof(previousKeys));
	std::memset(currentMouseButtons, 0, sizeof(currentMouseButtons));
	std::memset(previousMouseButtons, 0, sizeof(previousMouseButtons));
	mousePosition = glm::vec2(0.0f);
}

InputManager::~InputManager() {
	Shutdown();
}

void InputManager::Initialize(GLFWwindow* glfwWindow) {
	window = glfwWindow;
	if (!window) {
		std::cerr << "[InputManager] Error: Invalid window pointer" << std::endl;
		return;
	}

	// Store InputManager pointer in window for callback access
	glfwSetWindowUserPointer(window, this);

	std::cout << "[InputManager] Successfully initialized." << std::endl;
}

void InputManager::Update(float) {
	/*std::cout << "[INPUT MANAGER UPDATE]" << std::endl;*/
	updateInputStates();

	// Clean up any invalid entity references from GUI tracking
	if (!createdEntitiesByGUI.empty()) {
		auto* g_coordinator = Coordinator::GetInstance();
		auto activeEntities = g_coordinator->GetAllEntities();

		// Remove entities that no longer exist in the active entity list
		createdEntitiesByGUI.erase(
			std::remove_if(createdEntitiesByGUI.begin(), createdEntitiesByGUI.end(),
				[&activeEntities](Entity e) {
					// Return true if entity is NOT in active list (remove it)
					return std::find(activeEntities.begin(), activeEntities.end(), e) == activeEntities.end();
				}),
			createdEntitiesByGUI.end()
		);
	}

	updateGUIButtons();
	processDebugToggles();

	// Test delayed message (Advanced Feature: Message Queuing)
	if (isKeyTriggered(GLFW_KEY_SPACE)) {
		auto* msgMgr = MessageManager::GetInstance();
		InputComboMessage* delayed = new InputComboMessage("DELAYED_COMBO", glfwGetTime());
		delayed->delay = 5.0;
		msgMgr->QueueMessage(delayed);
		std::cout << "[INPUT] Queued delayed combo (5s)" << std::endl;
	}

	// FOR MOUSE TESTING; CAN USE FOR MAIN MENU
	if (isMouseButtonTriggered(0)) {
		std::cout << "[MOUSE] Left Click at " << getMousePosition().x << ", " << getMousePosition().y << std::endl;
	}

	if (isMouseButtonTriggered(1)) {
		std::cout << "[MOUSE] Right Click at " << getMousePosition().x << ", " << getMousePosition().y << std::endl;
	}

	if (isMouseButtonReleased(0)) {
		std::cout << "[MOUSE] Left Released" << std::endl;
	}

	// Editor toggle
	//if (isKeyTriggered(GLFW_KEY_O)) {
	//	bool now = !MyEditor::IsEnabled();
	//	MyEditor::SetEnabled(now);
	//	std::cout << "[Editor] ImGui " << (now ? "ON" : "OFF") << std::endl;
	//}
}

void InputManager::Shutdown() {
	if (window) {
		window = nullptr;
		std::cout << "[InputManager] Shutdown completed." << std::endl;
	}
}

void InputManager::updateInputStates() {
	std::memcpy(previousKeys, currentKeys, sizeof(currentKeys));
	std::memcpy(previousMouseButtons, currentMouseButtons, sizeof(currentMouseButtons));

	for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++)
		currentKeys[key] = (glfwGetKey(window, key) == GLFW_PRESS);

	for (int button = 0; button < 8; button++)
		currentMouseButtons[button] = (glfwGetMouseButton(window, button) == GLFW_PRESS);

	// Read cursor in window coordinates (top-left origin)
	double xw, yw;
	glfwGetCursorPos(window, &xw, &yw);
	mousePosition = glm::vec2(static_cast<float>(xw), static_cast<float>(yw));

	// Convert to FRAMEBUFFER pixels (handles HiDPI)
	int winW, winH, fbW, fbH;
	glfwGetWindowSize(window, &winW, &winH);
	glfwGetFramebufferSize(window, &fbW, &fbH);

	// Guard against divide-by-zero
	double fbX = 0.0, fbY = 0.0;
	if (winW > 0 && winH > 0) {
		fbX = xw * static_cast<double>(fbW) / static_cast<double>(winW);
		fbY = yw * static_cast<double>(fbH) / static_cast<double>(winH);
	}

	// Get position relative to game viewport
	double localX = fbX - GLApp::GameOffsetX;
	double localY = fbY - GLApp::GameOffsetY;

	mouseInGameViewport =
		(localX >= 0.0 && localY >= 0.0 &&
			localX <= GLApp::GameWidth &&
			localY <= GLApp::GameHeight);

	// SIMPLE conversion matching Transform system exactly
	// Transform does: PnormX = Pos.x / GameWidth
	// So: Pos.x = PnormX * GameWidth
	// Our buttons are stored directly as Pos.x (e.g., -600)

	// Map mouse from [0, GameWidth] to [-GameWidth/2, GameWidth/2]
	float vx = static_cast<float>(localX) - (GLApp::GameWidth * 0.5f);

	// Map mouse from [0, GameHeight] (top) to [-GameHeight/2, GameHeight/2] (bottom-left)
	float vy = (GLApp::GameHeight * 0.5f) - static_cast<float>(localY);

	vx += -150.0f;  // Start with 300 and adjust up or down as needed

	mousePositionVirtual = glm::vec2(vx, vy);
}

void InputManager::processDebugToggles() {
	const int debugKeys[] = { GLFW_KEY_F1, GLFW_KEY_F2, GLFW_KEY_F3, GLFW_KEY_F4, GLFW_KEY_F5, GLFW_KEY_F6 };
	bool* debugFlags[] = { &debugRectsEnabled, &debugLinesEnabled, &debugCirclesEnabled,
						  &debugCirclesFilledEnabled, &debugPointsEnabled, &debugGridEnabled };
	const char* debugNames[] = { "DEBUG RECTS", "DEBUG LINES", "DEBUG CIRCLES",
								"DEBUG CIRCLES FILLED", "DEBUG POINTS", "DEBUG GRID" };

	for (int i = 0; i < 6; ++i) {
		if (isKeyTriggered(debugKeys[i])) {
			*debugFlags[i] = !(*debugFlags[i]);
			std::cout << "[" << debugNames[i] << "] " << (*debugFlags[i] ? "ENABLED" : "DISABLED") << std::endl;
		}
	}
}

// -------------------------------------------------
// Keyboard queries - 3 states as required
// -------------------------------------------------
bool InputManager::isKeyPressed(int key) const {
	if (key < 0 || key > GLFW_KEY_LAST) return false;
	return currentKeys[key];
}

bool InputManager::isKeyTriggered(int key) const {
	if (key < 0 || key > GLFW_KEY_LAST) return false;
	return currentKeys[key] && !previousKeys[key];
}

bool InputManager::isKeyReleased(int key) const {
	if (key < 0 || key > GLFW_KEY_LAST) return false;
	return !currentKeys[key] && previousKeys[key];
}

// -------------------------------------------------
// Mouse queries
// -------------------------------------------------
bool InputManager::isMouseButtonPressed(int button) const {
	if (button < 0 || button >= 8) return false;
	return currentMouseButtons[button];
}

bool InputManager::isMouseButtonTriggered(int button) const {
	if (button < 0 || button >= 8) return false;
	return currentMouseButtons[button] && !previousMouseButtons[button];
}

bool InputManager::isMouseButtonReleased(int button) const {
	if (button < 0 || button >= 8) return false;
	return !currentMouseButtons[button] && previousMouseButtons[button];
}

glm::vec2 InputManager::getMousePosition() const {
	return mousePosition;
}

// =====================================================================
// IN-GAME GUI SYSTEM (M2 Rubric)
// =====================================================================

void InputManager::updateGUIButtons() {
	auto* g_coordinator = Coordinator::GetInstance();

	// USE VIRTUAL COORDINATES (already computed in updateInputStates)
	float mouseX = mousePositionVirtual.x;
	float mouseY = mousePositionVirtual.y;

	// DEBUG: Print mouse coordinates once per frame
	static int debugCounter = 0;
	if (debugCounter++ % 60 == 0) {  // Print every 60 frames to avoid spam
		/*std::cout << "[DEBUG] Mouse Window: (" << mousePosition.x << ", " << mousePosition.y << ")" << std::endl;
		std::cout << "[DEBUG] Mouse Virtual: (" << mouseX << ", " << mouseY << ")" << std::endl;
		std::cout << "[DEBUG] mouseInGameViewport: " << mouseInGameViewport << std::endl;*/
	}


	for (auto const& entity : GUIButtonEntities) {
		if (!g_coordinator->HasComponent<GUIButton>(entity)) continue;
		if (!g_coordinator->HasComponent<Framework::Transform>(entity)) continue;

		auto& button = g_coordinator->GetComponent<GUIButton>(entity);
		auto& transform = g_coordinator->GetComponent<Framework::Transform>(entity);

		// Check visibility and enabled state
		if (!transform.isVisible) {
			button.currentState = ButtonState::Normal;
			continue;
		}

		if (!button.isEnabled) {
			button.currentState = ButtonState::Normal;
			continue;
		}

		// Assumes position is at the CENTER of the button
		float halfWidth = transform.Scale.x * 0.5f;
		float halfHeight = transform.Scale.y * 0.5f;

		float left = transform.Pos.x - halfWidth;
		float right = transform.Pos.x + halfWidth;
		float bottom = transform.Pos.y - halfHeight;
		float top = transform.Pos.y + halfHeight;

		// DEBUG: Print button bounds
		//if (debugCounter % 60 == 1) {
		//	std::cout << "[DEBUG] Button '" << button.buttonID << "' bounds:" << std::endl;
		//	std::cout << "  Pos: (" << transform.Pos.x << ", " << transform.Pos.y << ")" << std::endl;
		//	std::cout << "  Scale: (" << transform.Scale.x << ", " << transform.Scale.y << ")" << std::endl;
		//	std::cout << "  Left: " << left << ", Right: " << right << std::endl;
		//	std::cout << "  Bottom: " << bottom << ", Top: " << top << std::endl;
		//}


		// Check if mouse is over button (both in virtual coordinates now!)
		bool isMouseOver = (mouseX >= left && mouseX <= right &&
			mouseY >= bottom && mouseY <= top &&
			mouseInGameViewport);

		button.wasClickedThisFrame = false;

		if (isMouseOver) {
			button.currentState = ButtonState::Hover;
			//std::cout << "[DEBUG] Mouse OVER button: " << button.buttonID << std::endl;

			if (isMouseButtonTriggered(0)) {
				std::cout << "[GUI Button] Button clicked: " << button.buttonID << std::endl;
				button.wasClickedThisFrame = true;
				executeButtonAction(button);
			}
		}
		else {
			button.currentState = ButtonState::Normal;
		}
	}
}

bool InputManager::isMouseOverButton(const glm::vec2& buttonPos, const glm::vec2& buttonSize,
	const glm::vec2& mouseWorld) const {
	float left = buttonPos.x - buttonSize.x * 0.5f;
	float right = buttonPos.x + buttonSize.x * 0.5f;
	float bottom = buttonPos.y - buttonSize.y * 0.5f;
	float top = buttonPos.y + buttonSize.y * 0.5f;

	return (mouseWorld.x >= left && mouseWorld.x <= right &&
		mouseWorld.y >= bottom && mouseWorld.y <= top &&
		mouseInGameViewport);
}

void InputManager::executeButtonAction(const GUIButton& button) {
	Factory factory;

	if (button.actionType == "CreateEntity") {
		if (!button.entityJsonPath.empty()) {
			auto result = factory.CreateEntityFromJson(button.entityJsonPath);

			if (result.has_value()) {
				Entity newEntity = result.value();
				createdEntitiesByGUI.push_back(newEntity);
				//std::cout << "[InputManager] Created entity from: " << button.entityJsonPath << std::endl;
			}
		}
	}
	else if (button.actionType == "DestroyEntity") {
		if (!createdEntitiesByGUI.empty()) {
			Entity toDestroy = createdEntitiesByGUI.back();
			createdEntitiesByGUI.pop_back();

			factory.DeleteEntity(toDestroy);
			//std::cout << "[InputManager] Destroyed entity: " << toDestroy << std::endl;
		}
	}
}