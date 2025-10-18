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
    std::cout << "[InputManager] Successfully initialized." << std::endl;
}

void InputManager::Update(float deltaTime) {
    updateInputStates();
    processDebugToggles();

    // PURELY FOR MOUSE TESTING; TO REMOVE AFTER MAIN MENU IS IMPLEMENTED
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

    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++) {
        currentKeys[key] = (glfwGetKey(window, key) == GLFW_PRESS);
    }

    for (int button = 0; button < 8; button++) {
        currentMouseButtons[button] = (glfwGetMouseButton(window, button) == GLFW_PRESS);
    }

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mousePosition = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
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