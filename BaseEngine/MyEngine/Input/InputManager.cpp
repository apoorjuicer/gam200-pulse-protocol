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

static std::string KeyToString(int key) {
    switch (key) {
    case GLFW_KEY_Q: return "Q";
    case GLFW_KEY_W: return "W";
    case GLFW_KEY_E: return "E";
    case GLFW_KEY_SPACE: return "SPACE";
    case GLFW_KEY_ESCAPE: return "ESC";
    default: return std::to_string(key); // fallback for debugging
    }
}

InputManager::InputManager()
    : window(nullptr), mouseX(0.0), mouseY(0.0), previousMouseX(0.0),
    previousMouseY(0.0), mouseSensitivity(1.0f), isFirstMouse(true),
    comboTimeout(2.0), totalRhythmInputs(0), accurateRhythmInputs(0),
    totalCombosAttempted(0), successfulCombos(0) {

    std::memset(currentKeys, 0, sizeof(currentKeys));
    std::memset(previousKeys, 0, sizeof(previousKeys));
    std::memset(currentMouse, 0, sizeof(currentMouse));
    std::memset(previousMouse, 0, sizeof(previousMouse));

    InitializeSwordCombos();
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
    std::cout << "[COMBOS READY] Available combos: Basic Strike (Q->W), Swift Slash (Q->E), Power Combo (Q->W->E)" << std::endl;

    currentBeatIndex = -4;  // Start at -4 so countdown can happen on beats -4, -3, -2, -1
    comboStartBeat = -1;
    lastBeatTime = 0.0;

    inCountdown = true;
    countdownBeats = 4;

    // Print first countdown immediately to sync with first beat
    std::cout << "[COUNTDOWN] " << countdownBeats << ".." << std::endl;
    countdownBeats--;
}

void InputManager::Update() {
    UpdateInputStates();
    ProcessRhythmInputs();
    ProcessComboInputs();
    ProcessMouseInputs();
    CheckForMissedBeat();
}

void InputManager::Shutdown() {
    if (window) {
        inputListeners.clear();
        window = nullptr;

        std::cout << "[InputManager] Shutdown completed." << std::endl;
    }
}

// ECS System Communication Methods
void InputManager::RegisterInputListener(const std::string& systemName, InputCallback callback) {
    inputListeners[systemName] = callback;
    //std::cout << "[InputManager] ECS System '" << systemName << "' registered for input events" << std::endl;
}

void InputManager::UnregisterInputListener(const std::string& systemName) {
    if (inputListeners.erase(systemName)) {
        //std::cout << "[InputManager] ECS System '" << systemName << "' unregistered from input events" << std::endl;
    }
}

void InputManager::BroadcastInputEvent(const InputEvent& event) {
    // broadcast to all systems egistered in our ECS
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

// rhythm game specific methods
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
    return 1; // always sword for now
}

void InputManager::UpdateInputStates() {
    // update all input states for this frame
    UpdateKeyStates();
    UpdateMouseStates();
}

void InputManager::ProcessRhythmInputs() {
    const int rhythmKeys[] = { GLFW_KEY_Q, GLFW_KEY_W, GLFW_KEY_E };
    for (int i = 0; i < 3; ++i) {
        if (IsKeyJustPressed(rhythmKeys[i])) {
            // Only process input if not in countdown OR if countdown just ended
            if (!inCountdown || (inCountdown && countdownBeats == 0)) {
                ProcessRhythmTiming(rhythmKeys[i]);
                std::string feedback = GetTimingFeedback();
                InputEvent rhythmEvent(InputEvent::Type::RHYTHM_INPUT, rhythmKeys[i], GetInputTiming(), feedback);
                BroadcastInputEvent(rhythmEvent);
            }
        }
    }
}

void InputManager::ProcessComboInputs() {
    if (currentComboSequence.empty()) return;

    for (const auto& combo : swordCombos) {
        if (currentComboSequence.size() == combo.keySequence.size()) {
            if (currentComboSequence == combo.keySequence) {
                totalCombosAttempted++;
                successfulCombos++;
                successfulCombosThisWindow++;

                bool isPartOfLongerCombo = false;
                for (const auto& longerCombo : swordCombos) {
                    if (longerCombo.keySequence.size() > combo.keySequence.size()) {
                        bool matches = true;
                        for (size_t i = 0; i < combo.keySequence.size(); ++i) {
                            if (longerCombo.keySequence[i] != combo.keySequence[i]) {
                                matches = false;
                                break;
                            }
                        }
                        if (matches) {
                            isPartOfLongerCombo = true;
                            break;
                        }
                    }
                }

                if (!isPartOfLongerCombo) {
                    currentComboSequence.clear();
                }
                return;
            }
        }
    }

    bool isValidPrefix = false;
    for (const auto& combo : swordCombos) {
        if (currentComboSequence.size() < combo.keySequence.size()) {
            bool matches = true;
            for (size_t i = 0; i < currentComboSequence.size(); ++i) {
                if (currentComboSequence[i] != combo.keySequence[i]) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                isValidPrefix = true;
                break;
            }
        }
    }

    if (!isValidPrefix) {
        totalCombosAttempted++;
        // Remove the immediate feedback - let it be handled at end of window
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
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_Q, GLFW_KEY_W}, "Basic Strike", 1.0);
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_Q, GLFW_KEY_E}, "Swift Slash", 0.8);
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_Q, GLFW_KEY_W, GLFW_KEY_E}, "Power Combo", 1.5);
}

// Helper Methods for Quality Feedback
std::string InputManager::GetTimingFeedback() {
    extern Coordinator g_coordinator;
    auto audioSystem = g_coordinator.GetSystem<AudioManager>();

    double musicPosition = audioSystem->GetMusicPosition();
    double beatPosition = fmod(musicPosition, beatInterval);

    double distance = std::min(beatPosition, fabs(beatInterval - beatPosition));

    if (distance <= 0.05) return "PERFECT";  // very tight
    if (distance <= 0.15) return "GOOD";     // slightly off
    return "MISS";                           // outside tolerance
}

bool InputManager::IsInputOnBeat() {
    extern Coordinator g_coordinator;
    auto audioSystem = g_coordinator.GetSystem<AudioManager>();

    double musicPosition = audioSystem->GetMusicPosition();
    double beatPosition = fmod(musicPosition, beatInterval);

    return (beatPosition <= beatTolerance) || (beatPosition >= (beatInterval - beatTolerance));
}

void InputManager::ProcessRhythmTiming(int key) {
    if (!canInput) return;

    inputReceivedOnThisBeat = true;

    if (currentComboSequence.empty()) {
        comboStartBeat = currentBeatIndex;
    }

    currentComboSequence.push_back(key);
    lastComboInputTime = GetInputTiming();

    // Check if this completes a combo before showing feedback
    bool comboCompleted = false;
    std::string completedComboName = "";

    for (const auto& combo : swordCombos) {
        if (currentComboSequence.size() == combo.keySequence.size() &&
            currentComboSequence == combo.keySequence) {
            comboCompleted = true;
            completedComboName = combo.name;
            break;
        }
    }

    std::string feedback = GetTimingFeedback();

    if (comboCompleted) {
        // Only show timing + combo name for successful combos
        std::cout << "[" << feedback << " - " << completedComboName << "] Key: " << KeyToString(key) << std::endl;
    }
    else {
        // For non-completing keys, just show timing feedback
        std::cout << "[" << feedback << "] Key: " << KeyToString(key) << std::endl;
    }
}

void InputManager::CheckForMissedBeat() {
    if (!canInput) return;

    extern Coordinator g_coordinator;
    auto audioSystem = g_coordinator.GetSystem<AudioManager>();
    if (!audioSystem) return;

    double currentTime = audioSystem->GetMusicPosition();

    // Simple guard: only advance beat if enough time has passed
    if (currentTime - lastBeatTime < beatInterval) {
        return; // Not enough time passed for next beat
    }

    // Update beat tracking
    lastBeatTime = currentTime - fmod(currentTime, beatInterval);
    currentBeatIndex++;

    // --- Countdown phase ---
    if (inCountdown) {
        if (countdownBeats > 0) {
            std::cout << "[COUNTDOWN] " << countdownBeats << ".." << std::endl;
            countdownBeats--;
            inputReceivedOnThisBeat = false;
            return;
        }
        else {
            // Countdown finished
            inCountdown = false;
            currentBeatIndex = 1;
            comboStartBeat = 1;
            inputReceivedOnThisBeat = false;
            return;
        }
    }

    // --- Normal gameplay beats ---
    int beatInWindow = (currentBeatIndex - comboStartBeat) % maxComboBeats;

    if (beatInWindow < maxComboBeats - 1) {
        // Check if input was received BEFORE printing INCOMING
        if (!inputReceivedOnThisBeat) {
            std::cout << "[INCOMING] Beat " << (beatInWindow + 1)
                << " / " << maxComboBeats << std::endl;
        }
    }
    else {
        // End of 4-beat window
        if (successfulCombosThisWindow == 0) {
            playerChances--;
            std::cout << "[MISS] No valid combo completed | Chances left: "
                << playerChances << std::endl;
        }

        currentComboSequence.clear();
        comboStartBeat = currentBeatIndex + 1;
        successfulCombosThisWindow = 0;
    }

    if (playerChances <= 0) {
        canInput = false;
        std::cout << "[YOU DIED!] No more chances." << std::endl;
        if (audioSystem) audioSystem->StopMusic();
    }

    // Reset flag AFTER all processing is done
    inputReceivedOnThisBeat = false;
}
