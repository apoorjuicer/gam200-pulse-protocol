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

#include "pch/pch.h"
#include "graphics/stb_image.h"

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

	//LoadGUIButtons();

	std::cout << "[InputManager] Successfully initialized." << std::endl;
}

void InputManager::Update(float) {
	updateInputStates();
	//UpdateGUIButtons();
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
	if (isKeyTriggered(GLFW_KEY_O)) {
		bool now = !MyEditor::IsEnabled();
		MyEditor::SetEnabled(now);
		std::cout << "[Editor] ImGui " << (now ? "ON" : "OFF") << std::endl;
	}
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

	// Guard against divide-by-zero (minimized window etc.)
	double fbX = 0.0, fbY = 0.0;
	if (winW > 0 && winH > 0) {
		fbX = xw * static_cast<double>(fbW) / static_cast<double>(winW);
		fbY = yw * static_cast<double>(fbH) / static_cast<double>(winH);
	}

	double localX = fbX - GLApp::GameOffsetX;
	double localY = fbY - GLApp::GameOffsetY;

	mouseInGameViewport =
		(localX >= 0.0 && localY >= 0.0 &&
			localX <= GLApp::GameWidth &&
			localY <= GLApp::GameHeight);

	// Map to virtual coordinates (0..VIRTUAL_W/H). Clamp if you prefer to keep it in-range.
	float vx = static_cast<float>(localX / std::max(1e-6f, GLApp::PixelScale));
	float vy = static_cast<float>(localY / std::max(1e-6f, GLApp::PixelScale));

	// If your virtual projection is bottom-left origin (recommended):
	// GLFW window coords have origin at top-left; flip Y to bottom-left virtual space.
	vy = static_cast<float>(GLApp::VIRTUAL_H) - vy;

	// Optionally clamp to virtual bounds
	vx = std::clamp(vx, 0.0f, static_cast<float>(GLApp::VIRTUAL_W));
	vy = std::clamp(vy, 0.0f, static_cast<float>(GLApp::VIRTUAL_H));

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

// FOR GUI BUTTON SECTION
//void InputManager::LoadGUIButtons() {
//    auto* g_coordinator = Coordinator::GetInstance();
//    
//    std::ifstream ifs("../../MyEngine/JSON/GUIButtons.json");
//    if (!ifs.is_open()) {
//        std::cerr << "[InputManager] Cannot open GUIButtons.json" << std::endl;
//        return;
//    }
//    
//    rapidjson::IStreamWrapper isw(ifs);
//    rapidjson::Document d;
//    d.ParseStream(isw);
//    
//    if (!d.HasMember("buttons") || !d["buttons"].IsArray()) {
//        std::cerr << "[InputManager] No buttons array in config" << std::endl;
//        return;
//    }
//    
//    const auto& buttonsArray = d["buttons"];
//    for (rapidjson::SizeType i = 0; i < buttonsArray.Size(); ++i) {
//        const auto& buttonJson = buttonsArray[i];
//        
//        Entity entity = g_coordinator->CreateEntity();
//        
//        Framework::Transform transform;
//        if (buttonJson.HasMember("position") && buttonJson["position"].IsObject()) {
//            auto& pos = buttonJson["position"];
//            if (pos.HasMember("x")) transform.Pos.x = pos["x"].GetFloat();
//            if (pos.HasMember("y")) transform.Pos.y = pos["y"].GetFloat();
//        }
//        if (buttonJson.HasMember("size") && buttonJson["size"].IsObject()) {
//            auto& size = buttonJson["size"];
//            if (size.HasMember("x")) transform.Scale.x = size["x"].GetFloat();
//            if (size.HasMember("y")) transform.Scale.y = size["y"].GetFloat();
//        }
//        transform.isVisible = true;
//        g_coordinator->AddComponent(entity, transform);
//        
//        GUIButton button;
//        button.Deserialize(buttonJson);
//        g_coordinator->AddComponent(entity, button);
//        
//        std::cout << "[InputManager] Created button: " << button.buttonID << std::endl;
//    }
//}
//
//void InputManager::UpdateGUIButtons() {
//    auto* g_coordinator = Coordinator::GetInstance();
//    glm::vec2 mousePos = mousePositionVirtual;
//    
//    for (auto const& entity : g_coordinator->GetEntitiesWithComponent<GUIButton>()) {
//        if (!g_coordinator->HasComponent<GUIButton>(entity)) continue;
//        if (!g_coordinator->HasComponent<Framework::Transform>(entity)) continue;
//        
//        auto& button = g_coordinator->GetComponent<GUIButton>(entity);
//        auto& transform = g_coordinator->GetComponent<Framework::Transform>(entity);
//        
//        if (!button.isEnabled) {
//            button.currentState = GUIButton::State::Normal;
//            continue;
//        }
//        
//        glm::vec2 buttonPos(transform.Pos.x, transform.Pos.y);
//        glm::vec2 buttonSize(transform.Scale.x, transform.Scale.y);
//        bool isMouseOver = IsMouseOverButton(buttonPos, buttonSize);
//        
//        if (isMouseOver) {
//            button.currentState = GUIButton::State::Hover;
//            if (isMouseButtonReleased(0)) {
//                ExecuteButtonAction(button);
//            }
//        } else {
//            button.currentState = GUIButton::State::Normal;
//        }
//    }
//}
//
//bool InputManager::IsMouseOverButton(const glm::vec2& buttonPos, const glm::vec2& buttonSize) const {
//    glm::vec2 mousePos = mousePositionVirtual;
//    
//    float left = buttonPos.x - buttonSize.x * 0.5f;
//    float right = buttonPos.x + buttonSize.x * 0.5f;
//    float bottom = buttonPos.y - buttonSize.y * 0.5f;
//    float top = buttonPos.y + buttonSize.y * 0.5f;
//    
//    return (mousePos.x >= left && mousePos.x <= right &&
//            mousePos.y >= bottom && mousePos.y <= top &&
//            mouseInGameViewport);
//}
//
//void InputManager::ExecuteButtonAction(const GUIButton& button) {
//    auto* g_coordinator = Coordinator::GetInstance();
//    Factory factory;
//    
//    std::cout << "[InputManager] Button clicked: " << button.buttonID << std::endl;
//    
//    if (button.actionType == "CreateEntity") {
//        if (!button.entityJsonPath.empty()) {
//            Entity newEntity = factory.CreateEntityFromJson(button.entityJsonPath);
//            lastCreatedEntityByGUI = newEntity;
//            std::cout << "[InputManager] Created entity (ID: " << newEntity << ")" << std::endl;
//        }
//    } 
//    else if (button.actionType == "DestroyEntity") {
//        if (lastCreatedEntityByGUI != 0) {
//            g_coordinator->DestroyEntity(lastCreatedEntityByGUI);
//            std::cout << "[InputManager] Destroyed entity: " << lastCreatedEntityByGUI << std::endl;
//            lastCreatedEntityByGUI = 0;
//        } else {
//            std::cout << "[InputManager] No entity to destroy!" << std::endl;
//        }
//    }
//}