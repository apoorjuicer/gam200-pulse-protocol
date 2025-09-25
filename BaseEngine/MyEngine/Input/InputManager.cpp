// ==================================================================================
// InputManager.cpp  
// Author: Chloe Lau Rey En
// Brief: Implementation of InputManager class with frame-based input detection.
//        Handles rhythm gameplay inputs, weapon switching, mouse aiming.
// ==================================================================================

#include "InputManager.h"
#include <iostream>
#include <cstring>
#include <iomanip>

InputManager::InputManager()
    : window(nullptr), mouseX(0.0), mouseY(0.0), previousMouseX(0.0),
    previousMouseY(0.0), mouseSensitivity(1.0f), isFirstMouse(true),
    comboTimeout(2.0), totalRhythmInputs(0), accurateRhythmInputs(0),
    totalCombosAttempted(0), successfulCombos(0) {

    // Initialize input state arrays
    std::memset(currentKeys, 0, sizeof(currentKeys));
    std::memset(previousKeys, 0, sizeof(previousKeys));
    std::memset(currentMouse, 0, sizeof(currentMouse));
    std::memset(previousMouse, 0, sizeof(previousMouse));

    // Initialize sword combos for rhythm game
    InitializeSwordCombos();

    std::cout << "[InputManager] System initialized" << std::endl;
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

    std::cout << "[InputManager] Successfully initialized for rhythm game input." << std::endl;
    std::cout << "[COMBOS READY] Available combos: Basic Strike (Q->W), Swift Slash (Q->E), Power Combo (Q->W->E)" << std::endl;
}

void InputManager::Update() {
    // Main ECS-style update loop
    UpdateInputStates();
    ProcessRhythmInputs();
    ProcessComboInputs();
    ProcessMouseInputs();
}

void InputManager::Shutdown() {
    if (window) {
        std::cout << "[InputManager] System shutting down..." << std::endl;

        // Clear all registered listeners (ECS cleanup)
        inputListeners.clear();
        window = nullptr;

        std::cout << "[InputManager] Shutdown completed." << std::endl;
    }
}

// ECS System Communication Methods
void InputManager::RegisterInputListener(const std::string& systemName, InputCallback callback) {
    inputListeners[systemName] = callback;
    std::cout << "[InputManager] ECS System '" << systemName << "' registered for input events" << std::endl;
}

void InputManager::UnregisterInputListener(const std::string& systemName) {
    if (inputListeners.erase(systemName)) {
        std::cout << "[InputManager] ECS System '" << systemName << "' unregistered from input events" << std::endl;
    }
}

void InputManager::BroadcastInputEvent(const InputEvent& event) {
    // Broadcast to all registered ECS systems
    for (const auto& [systemName, callback] : inputListeners) {
        callback(event);
    }
}

// Basic Input Detection Methods (for ECS systems to query)
bool InputManager::IsKeyPressed(int key) const {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return currentKeys[key];
}

bool InputManager::IsKeyJustPressed(int key) const {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return currentKeys[key] && !previousKeys[key];
}

bool InputManager::IsKeyReleased(int key) const {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return !currentKeys[key] && previousKeys[key];
}

bool InputManager::IsMousePressed(int button) const {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return currentMouse[button];
}

bool InputManager::IsMouseJustPressed(int button) const {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return currentMouse[button] && !previousMouse[button];
}

void InputManager::GetMousePosition(double& x, double& y) const {
    x = mouseX;
    y = mouseY;
}

void InputManager::GetMouseDelta(double& deltaX, double& deltaY) const {
    deltaX = (mouseX - previousMouseX) * mouseSensitivity;
    deltaY = (mouseY - previousMouseY) * mouseSensitivity;
}

void InputManager::SetMouseSensitivity(float sensitivity) {
    mouseSensitivity = sensitivity;
    std::cout << "[InputManager] Mouse sensitivity set to: " << sensitivity << std::endl;
}

// Rhythm Game Specific Methods
bool InputManager::IsRhythmKeyPressed(int key) const {
    return (key == GLFW_KEY_Q || key == GLFW_KEY_W || key == GLFW_KEY_E) && IsKeyJustPressed(key);
}

bool InputManager::IsAiming() const {
    return IsMousePressed(GLFW_MOUSE_BUTTON_RIGHT);
}

double InputManager::GetInputTiming() const {
    return glfwGetTime();
}

int InputManager::GetCurrentWeaponSelected() const {
    return 1; // Always sword for now
}

// ECS-Compatible Input Processing Methods
void InputManager::UpdateInputStates() {
    // Update all input states for this frame
    UpdateKeyStates();
    UpdateMouseStates();
}

void InputManager::ProcessRhythmInputs() {
    const int rhythmKeys[] = { GLFW_KEY_Q, GLFW_KEY_W, GLFW_KEY_E };

    for (int i = 0; i < 3; ++i) {
        if (IsKeyJustPressed(rhythmKeys[i])) {
            totalRhythmInputs++;

            // Generate timing feedback
            std::string feedback = GetTimingFeedback();
            if (feedback.find("PERFECT") != std::string::npos ||
                feedback.find("GREAT") != std::string::npos ||
                feedback.find("GOOD") != std::string::npos) {
                accurateRhythmInputs++;
            }

            // Create and broadcast ECS input event
            InputEvent rhythmEvent(InputEvent::Type::RHYTHM_INPUT, rhythmKeys[i], GetInputTiming(), feedback);
            BroadcastInputEvent(rhythmEvent);

            // Add to combo sequence for processing
            currentComboSequence.push_back(rhythmKeys[i]);
            if (currentComboSequence.size() == 1) {
                lastComboInputTime = GetInputTiming();
            }
        }
    }
}

void InputManager::ProcessComboInputs() {
    if (currentComboSequence.empty()) return;

    double currentTime = GetInputTiming();

    // Check for combo timeout
    if (currentTime - lastComboInputTime > comboTimeout) {
        currentComboSequence.clear();
        return;
    }

    // Check each sword combo pattern
    for (const auto& combo : swordCombos) {
        if (currentComboSequence.size() == combo.keySequence.size()) {
            bool matches = true;
            for (size_t i = 0; i < combo.keySequence.size(); ++i) {
                if (currentComboSequence[i] != combo.keySequence[i]) {
                    matches = false;
                    break;
                }
            }

            if (matches) {
                // Combo successfully executed!
                successfulCombos++;
                totalCombosAttempted++;

                // Calculate combo quality based on timing
                double comboTime = currentTime - lastComboInputTime;
                std::string comboQuality = GetComboQuality(comboTime, combo.maxTimeBetweenInputs);

                // Create and broadcast ECS combo event
                InputEvent comboEvent(InputEvent::Type::COMBO_EXECUTED, 0, currentTime, comboQuality);
                comboEvent.comboName = combo.name;
                BroadcastInputEvent(comboEvent);

                currentComboSequence.clear();
                return;
            }
        }
    }

    // If we have maximum inputs and no match, it's invalid
    if (currentComboSequence.size() >= 3) {
        totalCombosAttempted++;
        currentComboSequence.clear();
    }
}

void InputManager::ProcessMouseInputs() {
    // Process mouse clicks and broadcast events
    if (IsMouseJustPressed(GLFW_MOUSE_BUTTON_LEFT) || IsMouseJustPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        InputEvent mouseEvent(InputEvent::Type::MOUSE_INPUT, mouseX, mouseY, GetInputTiming());
        BroadcastInputEvent(mouseEvent);
    }
}

// Internal State Management Methods
void InputManager::UpdateKeyStates() {
    std::memcpy(previousKeys, currentKeys, sizeof(currentKeys));
    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++) {
        currentKeys[key] = (glfwGetKey(window, key) == GLFW_PRESS);
    }
}

void InputManager::UpdateMouseStates() {
    std::memcpy(previousMouse, currentMouse, sizeof(currentMouse));
    for (int button = GLFW_MOUSE_BUTTON_1; button <= GLFW_MOUSE_BUTTON_LAST; button++) {
        currentMouse[button] = (glfwGetMouseButton(window, button) == GLFW_PRESS);
    }

    previousMouseX = mouseX;
    previousMouseY = mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    if (isFirstMouse) {
        previousMouseX = mouseX;
        previousMouseY = mouseY;
        isFirstMouse = false;
    }
}

void InputManager::InitializeSwordCombos() {
    swordCombos.clear();

    // Define the 3 sword combos
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_Q, GLFW_KEY_W}, "Basic Strike", 1.0);
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_Q, GLFW_KEY_E}, "Swift Slash", 0.8);
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_Q, GLFW_KEY_W, GLFW_KEY_E}, "Power Combo", 1.5);

    std::cout << "[InputManager] System initialized " << swordCombos.size() << " sword combos" << std::endl;
}

// Helper Methods for Quality Feedback
std::string InputManager::GetTimingFeedback() const {
    // Simulate timing feedback with variety
    static int feedbackCounter = 0;
    feedbackCounter++;

    // Cycle through different feedback types to show range
    switch (feedbackCounter % 5) {
    case 0: return "PERFECT";
    case 1: return "GREAT";
    case 2: return "GOOD";
    case 3: return "OK";
    default: return "MISS";
    }
}

std::string InputManager::GetComboQuality(double actualTime, double maxTime) const {
    double ratio = actualTime / maxTime;

    if (ratio <= 0.3) {
        return "PERFECT";
    }
    else if (ratio <= 0.6) {
        return "GREAT";
    }
    else if (ratio <= 0.9) {
        return "GOOD";
    }
    else {
        return "SLOW";
    }
}