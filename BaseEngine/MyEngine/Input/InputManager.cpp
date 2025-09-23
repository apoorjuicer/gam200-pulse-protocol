// ==================================================================================
// InputManager.cpp  
// Author: Chloe Lau Rey En
// Brief: Implementation of InputManager class with frame-based input detection.
//        Handles rhythm gameplay inputs, weapon switching, mouse aiming, and
//        provides M1 testing functionality with console output.
// ==================================================================================

#include "InputManager.h"
#include <iostream>
#include <cstring>
#include <iomanip>

InputManager::InputManager()
    : window(nullptr), mouseX(0.0), mouseY(0.0), previousMouseX(0.0),
    previousMouseY(0.0), mouseSensitivity(1.0f), isFirstMouse(true),
    comboTimeout(2.0), totalRhythmInputs(0), accurateRhythmInputs(0),
    totalCombosAttempted(0), successfulCombos(0), m1TestMode(true), lastStatusTime(0.0) {

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

    // M1 testing demonstration
    if (m1TestMode) {
        RunM1Test();
    }
}

void InputManager::Shutdown() {
    if (window) {
        std::cout << "[InputManager] System shutting down..." << std::endl;
        DisplayInputStats();

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
    return 1; // Always sword for M1 milestone
}

// ECS-Compatible Input Processing Methods
void InputManager::UpdateInputStates() {
    // Update all input states for this frame
    UpdateKeyStates();
    UpdateMouseStates();
}

void InputManager::ProcessRhythmInputs() {
    const int rhythmKeys[] = { GLFW_KEY_Q, GLFW_KEY_W, GLFW_KEY_E };
    const char* keyNames[] = { "Q", "W", "E" };

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

            // M1 console output
            if (m1TestMode) {
                std::cout << "[M1 RHYTHM] " << keyNames[i] << " pressed | "
                    << feedback << " | Total inputs: " << totalRhythmInputs
                    << " | ECS listeners: " << inputListeners.size() << std::endl;
            }
        }
    }
}

void InputManager::ProcessComboInputs() {
    if (currentComboSequence.empty()) return;

    double currentTime = GetInputTiming();

    // Check for combo timeout
    if (currentTime - lastComboInputTime > comboTimeout) {
        if (m1TestMode && !currentComboSequence.empty()) {
            std::cout << "[TIMEOUT] Combo reset!" << std::endl;
        }
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

                if (m1TestMode) {
                    std::cout << "[EXECUTE " << combo.name << "]"
                        << " | Quality: " << comboQuality
                        << " | Timing: " << std::fixed << std::setprecision(2) << comboTime << "s"
                        << " | Success rate: " << std::fixed << std::setprecision(1)
                        << (GetComboSuccessRate() * 100.0) << "%"
                        << " | ECS Event Sent" << std::endl;
                }

                currentComboSequence.clear();
                return;
            }
        }
    }

    // If we have maximum inputs and no match, it's invalid
    if (currentComboSequence.size() >= 3) {
        if (m1TestMode) {
            std::cout << "[OVERLOAD] Invalid sequence - resetting" << std::endl;
        }
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

// M1 Testing - demonstrates ECS compatibility and all input functionality
void InputManager::RunM1Test() {
    // Test mouse functionality
    if (IsMouseJustPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        std::cout << "[MOUSE] Right-click (AIM) at position: ("
            << std::fixed << std::setprecision(1) << mouseX << ", " << mouseY
            << ") | ECS Event Broadcast" << std::endl;
    }

    if (IsMouseJustPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        if (IsAiming()) {
            std::cout << "[MOUSE] Left-click while aiming (ATTACK) at: ("
                << std::fixed << std::setprecision(1) << mouseX << ", " << mouseY
                << ") | ECS Event Broadcast" << std::endl;
        }
        else {
            std::cout << "[MOUSE] Left-click (UI INTERACTION) at: ("
                << std::fixed << std::setprecision(1) << mouseX << ", " << mouseY
                << ") | ECS Event Broadcast" << std::endl;
        }
    }

    // Test mouse movement during aiming
    if (IsAiming()) {
        double deltaX, deltaY;
        GetMouseDelta(deltaX, deltaY);
        if (std::abs(deltaX) > 1.0 || std::abs(deltaY) > 1.0) {
            std::cout << "[AIM] Mouse movement - Delta: ("
                << std::fixed << std::setprecision(2) << deltaX << ", " << deltaY
                << ") | Sensitivity: " << mouseSensitivity << std::endl;
        }
    }

    // ECS status updates every 5 seconds
    double currentTime = GetInputTiming();
    if (currentTime - lastStatusTime > 5.0) {
        std::cout << "\n=== INPUT STATUS ===" << std::endl;
        std::cout << "ECS Systems listening: " << inputListeners.size() << std::endl;
        std::cout << "Total rhythm inputs: " << totalRhythmInputs << std::endl;
        std::cout << "Accurate inputs: " << accurateRhythmInputs << "/" << totalRhythmInputs
            << " (" << std::fixed << std::setprecision(1)
            << (totalRhythmInputs > 0 ? 100.0 * accurateRhythmInputs / totalRhythmInputs : 0.0) << "%)" << std::endl;
        std::cout << "Successful combos: " << successfulCombos << "/" << totalCombosAttempted
            << " (" << std::fixed << std::setprecision(1) << (GetComboSuccessRate() * 100.0) << "%)" << std::endl;
        std::cout << "Current weapon: Sword" << std::endl;
        std::cout << "Available combos: Q->W, Q->E, Q->W->E" << std::endl;
        std::cout << "EntityMember count: " << EntityMember.size() << " (from Systems base)" << std::endl;
        std::cout << "=========================" << std::endl;
        lastStatusTime = currentTime;
    }
}

void InputManager::DisplayInputStats() const {
    std::cout << "\n=== INPUT STATS ===" << std::endl;
    std::cout << "Total ECS systems registered: " << inputListeners.size() << std::endl;
    std::cout << "Total rhythm inputs processed: " << totalRhythmInputs << std::endl;
    std::cout << "Accurate rhythm inputs: " << accurateRhythmInputs
        << " (" << std::fixed << std::setprecision(1)
        << (totalRhythmInputs > 0 ? 100.0 * accurateRhythmInputs / totalRhythmInputs : 0.0) << "%)" << std::endl;
    std::cout << "Total combo attempts: " << totalCombosAttempted << std::endl;
    std::cout << "Successful combos: " << successfulCombos
        << " (" << std::fixed << std::setprecision(1) << (GetComboSuccessRate() * 100.0) << "%)" << std::endl;
    std::cout << "ECS EntityMember set size: " << EntityMember.size() << std::endl;
    std::cout << "====================================" << std::endl;
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

    // Define the 3 sword combos for M1 demonstration
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_Q, GLFW_KEY_W}, "Basic Strike", 1.0);
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_Q, GLFW_KEY_E}, "Swift Slash", 0.8);
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_Q, GLFW_KEY_W, GLFW_KEY_E}, "Power Combo", 1.5);

    std::cout << "[InputManager] System initialized " << swordCombos.size() << " sword combos" << std::endl;
}

// Helper Methods for Quality Feedback
std::string InputManager::GetTimingFeedback() const {
    // Simulate timing feedback with variety for M1 demonstration
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

/*
===============================================================================
TEAM INTEGRATION GUIDE - How to use InputManager in your systems:
===============================================================================

FOR AUDIO SYSTEM INTEGRATION:
- When your audio system is ready, connect it to InputManager:
  g_inputSystem->RegisterInputListener("AudioSystem", callback);
- Use rhythm events to sync audio with input timing
- InputManager can provide beat timing once audio system provides BPM

FOR RENDERING SYSTEM INTEGRATION:
- Register for input events to handle visual feedback:
  g_inputSystem->RegisterInputListener("RenderSystem", callback);
- Use combo events to trigger visual effects
- Query mouse position for cursor rendering, aiming reticles

FOR GAME LOGIC SYSTEMS:
- Register for specific events you care about
- Query input state directly when needed
- Access statistics for scoring, achievements

FOR ECS SYSTEMS SPECIFICALLY:
- Inherit from Systems base class
- Use EntityMember set for entity processing
- Register input callbacks in Initialize()
- Process your components in Update()

EXAMPLE ECS SYSTEM TEMPLATE:
class YourSystem : public Systems {
public:
    void Initialize() {
        // Register for input events you need
        g_inputSystem->RegisterInputListener("YourSystem",
            [this](const InputEvent& event) {
                HandleInputEvent(event);
            });
    }

    void Update() {
        // Process all entities with components this system needs
        for (Entity entity : EntityMember) {
            // Your component processing logic
        }
    }

private:
    void HandleInputEvent(const InputEvent& event) {
        switch (event.type) {
            case InputEvent::Type::RHYTHM_INPUT:
                // Handle rhythm input
                break;
            case InputEvent::Type::COMBO_EXECUTED:
                // Handle combo execution
                break;
            case InputEvent::Type::MOUSE_INPUT:
                // Handle mouse input
                break;
        }
    }
};

===============================================================================
*/