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
#include "../MyEngine/Graphics/GraphicsManager.h"

static Vector2 ScreenToWorld_ForInput(double fbX, double fbY)
{
	// World size - MUST match Transform.cpp VIRTUAL_W/H (1600x900)
	static constexpr float kWorldW = 1600.0f;
	static constexpr float kWorldH = 900.0f;

	// Get the game viewport (letterbox-aware)
	float drawX = static_cast<float>(GLHelper::GameOffsetX);
	float drawY = static_cast<float>(GLHelper::GameOffsetY);
	float drawW = static_cast<float>(GLHelper::GameWidth);
	float drawH = static_cast<float>(GLHelper::GameHeight);

	// Convert framebuffer pixels to NDC
	const float ndcX = ((fbX - drawX) / drawW) * 2.0f - 1.0f;
	const float ndcY = 1.0f - ((fbY - drawY) / drawH) * 2.0f;
	const glm::vec3 ndc(ndcX, ndcY, 1.0f);

	// Convert NDC to world using camera and normalization matrix
	const float sx = 2.0f / kWorldW;
	const float sy = 2.0f / kWorldH;
	const glm::mat3 N(sx, 0, 0, 0, sy, 0, 0, 0, 1);
	const glm::mat3 Cam = GLApp::BuildPanZoomNDC(GLApp::gCamera);
	const glm::mat3 invAll = glm::inverse(Cam * N);

	//static int frameCount = 0;
	//frameCount++;
	//if (frameCount % 60 == 0) {
	//	std::cout << "Camera center: (" << GLApp::gCamera.center.x << ", " << GLApp::gCamera.center.y << ")" << std::endl;
	//	std::cout << "Camera zoom: " << GLApp::gCamera.zoom << std::endl;
	//}

	const glm::vec3 w = invAll * ndc;

	// Shift Y UP to align buttons
	return Vector2{ w.x, w.y - 760.0f };  // Add 660 to Y (shift UP)
}

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
	processDebugToggles();

	auto* coord = Coordinator::GetInstance();
	auto sceneManager = coord->GetSystem<SceneManager>();

	// will re-implemnt when editor is fully polished
	//// ESC key: Go back to previous scene
	//if (isKeyTriggered(GLFW_KEY_ESCAPE) && sceneManager) {
	//	std::string currentScene = sceneManager->CurrentScene;

	//	// If we're at MainMenu, ESC does nothing
	//	if (currentScene == "MainMenu") {
	//		std::cout << "[InputManager] ESC pressed - already at MainMenu, doing nothing" << std::endl;
	//		return; // Early exit - don't process ESC further
	//	}

	//	// For all other scenes, go back to LastScene
	//	std::string lastScene = sceneManager->GrabLastScene();

	//	if (!lastScene.empty() && lastScene != currentScene) {
	//		sceneManager->CurrentScene = lastScene;
	//		sceneManager->ClearLastScene();  // Clear so we can't toggle back
	//		std::cout << "[InputManager] ESC pressed - returning to: " << lastScene << std::endl;
	//	}
	//	else {
	//		std::cout << "[InputManager] ESC pressed - no previous scene to return to" << std::endl;
	//	}
	//}

	// Test delayed message (Advanced Feature: Message Queuing)
	if (isKeyTriggered(GLFW_KEY_SPACE)) {
		auto* msgMgr = MessageManager::GetInstance();
		InputComboMessage* delayed = new InputComboMessage("DELAYED_COMBO", glfwGetTime());
		delayed->delay = 5.0;
		msgMgr->QueueMessage(delayed);
		std::cout << "[INPUT] Queued delayed combo (5s)" << std::endl;
	}

	// Toggle fullscreen with F11
	if (isKeyTriggered(GLFW_KEY_F11))
	{
		if (Engine)
			Engine->ToggleFullscreen();
	}

	// FOR MOUSE TESTING; CAN USE FOR MAIN MENU
	//if (isMouseButtonTriggered(0)) {
	//	std::cout << "[MOUSE] Left Click at " << getMousePosition().x << ", " << getMousePosition().y << std::endl;
	//}

	//if (isMouseButtonTriggered(1)) {
	//	std::cout << "[MOUSE] Right Click at " << getMousePosition().x << ", " << getMousePosition().y << std::endl;
	//}

	//if (isMouseButtonReleased(0)) {
	//	std::cout << "[MOUSE] Left Released" << std::endl;
	//}

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

	Vector2 worldPos = ScreenToWorld_ForInput(fbX, fbY);
	mousePositionVirtual = glm::vec2(worldPos.x, worldPos.y);

	// Check if mouse is in the game viewport
	double localX = fbX - GLHelper::GameOffsetX;
	double localY = fbY - GLHelper::GameOffsetY;
	mouseInGameViewport = (localX >= 0.0 && localY >= 0.0 && localX <= GLHelper::GameWidth && localY <= GLHelper::GameHeight);
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

	if (isKeyTriggered(GLFW_KEY_F7)) {
		stressToggleRequested = true;
	}

	// F8: Toggle debug overlay (FPS counter, performance stats)
	if (isKeyTriggered(GLFW_KEY_F8)) {
		auto* coord = Coordinator::GetInstance();
		auto graphics = coord->GetSystem<GraphicsManager>();
		if (graphics) {
			graphics->ToggleDebugOverlay();
			std::cout << "[DEBUG OVERLAY] "
				<< (graphics->IsDebugOverlayEnabled() ? "ENABLED" : "DISABLED")
				<< std::endl;
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