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

extern Coordinator g_coordinator;

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

// -------------------------------------------------
// Class Implementation
// -------------------------------------------------
InputManager::InputManager() : window(nullptr) {
    std::memset(currentKeys, 0, sizeof(currentKeys));
    std::memset(previousKeys, 0, sizeof(previousKeys));

    successfulCombos = 0;
    successfulCombosThisWindow = 0;

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
    CheckForMissedBeat();

    auto& transform = g_coordinator.GetComponent<Framework::Transform>(0);

    if (transform.autoHide && transform.isVisible) {
        float dt = 0.016f; // approximately 60 FPS
        transform.visibilityTimer -= dt;
        if (transform.visibilityTimer <= 0.0f) {
            transform.isVisible = false;
            transform.autoHide = false;
        }
    }
}

void InputManager::Shutdown() {
    if (window) {
        inputListeners.clear();
        window = nullptr;

        std::cout << "[InputManager] Shutdown completed." << std::endl;
    }
}

// -------------------------------------------------
// System communication Implementation
// -------------------------------------------------
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
    // When broadcasting rhythm input, update the visual entity
    if (event.type == InputEvent::Type::RHYTHM_INPUT) {
        auto& transform = g_coordinator.GetComponent<Framework::Transform>(0);

        // Show visual feedback
        transform.isVisible = true;
        transform.visibilityTimer = 0.5f;
        transform.autoHide = true;
    }

    // Broadcast to all registered listeners
    for (const auto& [systemName, callback] : inputListeners) {
        callback(event);
    }
}

// -------------------------------------------------
// Input Detection Implementation
// -------------------------------------------------
bool InputManager::IsKeyJustPressed(int key) const {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return currentKeys[key] && !previousKeys[key];
}

double InputManager::GetInputTiming() const {
    return glfwGetTime();
}

bool InputManager::IsRhythmKeyPressed(int key) const {
    return (key == GLFW_KEY_Q || key == GLFW_KEY_W || key == GLFW_KEY_E) && IsKeyJustPressed(key);
}

// -------------------------------------------------
// Internal Helper Implementation
// -------------------------------------------------
void InputManager::UpdateInputStates() {
    // Copy previous frame's key states
    std::memcpy(previousKeys, currentKeys, sizeof(currentKeys));

    // Update current frame's key states
    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++) {
        currentKeys[key] = (glfwGetKey(window, key) == GLFW_PRESS);
    }
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
                    InputEvent comboEvent(InputEvent::Type::COMBO_EXECUTED, 0, GetInputTiming(), "COMBO", combo.name);
                    BroadcastInputEvent(comboEvent);
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
        currentComboSequence.clear();
    }
}

void InputManager::InitializeSwordCombos() {
    swordCombos.clear();
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_Q, GLFW_KEY_W}, "Basic Strike");
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_Q, GLFW_KEY_E}, "Swift Slash");
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_Q, GLFW_KEY_W, GLFW_KEY_E}, "Power Combo");
}

std::string InputManager::GetTimingFeedback() {
    auto audioSystem = g_coordinator.GetSystem<AudioManager>();

    double musicPosition = audioSystem->GetMusicPosition();
    double beatPosition = fmod(musicPosition, beatInterval);

    double distance = std::min(beatPosition, fabs(beatInterval - beatPosition));

    if (distance <= 0.05) return "PERFECT";  // very tight
    if (distance <= 0.15) return "GOOD";     // slightly off
    return "MISS";                           // outside tolerance
}

void InputManager::ProcessRhythmTiming(int key) {
    if (!canInput) return;

    inputReceivedOnThisBeat = true;

    if (currentComboSequence.empty()) {
        comboStartBeat = currentBeatIndex;
    }

    currentComboSequence.push_back(key);

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