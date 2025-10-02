// ==================================================================================
// InputManager.cpp  
// Author: Chloe Lau Rey En
// Brief: Implementation of InputManager class with synchronized cycle timing
// ==================================================================================

#include "pch/pch.h"
#include <iostream>
#include <cstring>
#include <iomanip>

static std::string KeyToString(int key) {
    switch (key) {
    case GLFW_KEY_W: return "W";
    case GLFW_KEY_A: return "A";
    case GLFW_KEY_S: return "S";
    case GLFW_KEY_D: return "D";
    default: return std::to_string(key);
    }
}

InputManager::InputManager() : window(nullptr) {
    std::memset(currentKeys, 0, sizeof(currentKeys));
    std::memset(previousKeys, 0, sizeof(previousKeys));
    successfulCombos = 0;
    successfulCombosThisWindow = 0;
    currentCyclePosition = 1;
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
    std::cout << "[COMBOS READY] Available combos: Basic Strike (W->A), Swift Slash (S->D), Power Combo (A->S->D)" << std::endl;

    currentBeatIndex = 0;
    comboStartBeat = -1;
    lastBeatTime = 0.0;
    currentCyclePosition = 1;

    auto* g_coordinator = Coordinator::GetInstance();
   // Entity entity = g_coordinator->CreateEntity();

    //Framework::Transform feedbackTransform{};
    //feedbackTransform.Pos = { 0.0f, 0.0f };
    //feedbackTransform.Scale = { 500.0f, 500.0f };
    //feedbackTransform.mdl_ref = 1.0f;
    //feedbackTransform.shd_ref = 2.0f;
    //feedbackTransform.isVisible = true;
    //feedbackTransform.currentFrame = 1;

    std::string path = "../../MyEngine/JSON/transform.json";

    //if (!Framework::SaveTransform(feedbackTransform, path)) {
    //    std::cerr << "Failed to save transform!\n";
    //}
    //else {
    //    std::cout << "Saved to: " << std::filesystem::absolute(path) << "\n";
    //}

    Framework::Transform new_feedbackTransform;
    if (!Framework::LoadTransform(new_feedbackTransform, path)) {
        std::cerr << "Failed to load transform!\n";
    }
    else {
        std::cout << "Saved to: " << std::filesystem::absolute(path) << "\n";
    }

    //g_coordinator->AddComponent(entity, new_feedbackTransform);
    //SetFeedbackEntity(entity);


    Entity entity = g_coordinator->CreateEntity();

    Framework::Transform feedbackTransform2{};
    feedbackTransform2.Pos = { 0.0f, 0.0f };
    feedbackTransform2.Scale = { 500.0f, 500.0f };
    feedbackTransform2.mdl_ref = 1.0f;
    feedbackTransform2.shd_ref = 3.0f;
    feedbackTransform2.isVisible = true;
    feedbackTransform2.currentFrame = 1;

    g_coordinator->AddComponent(entity, feedbackTransform2);
    feedbackEntity = entity;
}

void InputManager::Update() {
    UpdateInputStates();
    ProcessRhythmInputs();
    ProcessComboInputs();
    CheckForMissedBeat();

    auto* g_coordinator = Coordinator::GetInstance();
    for (auto& entity : EntityMember)
    {
        auto& transform = g_coordinator->GetComponent<Framework::Transform>(entity);

        if (transform.autoHide && transform.isVisible) {
            float dt = 0.016f;
            transform.visibilityTimer -= dt;
            if (transform.visibilityTimer <= 0.0f) {
                transform.isVisible = false;
                transform.autoHide = false;
            }
        }

    }

    for (auto& entity2 : EntityMember)
    {
        auto& transform2 = g_coordinator->GetComponent<Framework::Transform>(entity2);

        if (transform2.autoHide && transform2.isVisible) {
            float dt = 0.016f;
            transform2.visibilityTimer -= dt;
            if (transform2.visibilityTimer <= 0.0f) {
                transform2.isVisible = false;
                transform2.autoHide = false;
            }
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

void InputManager::RegisterInputListener(const std::string& systemName, InputCallback callback) {
    inputListeners[systemName] = callback;
}

void InputManager::UnregisterInputListener(const std::string& systemName) {
    inputListeners.erase(systemName);
}

void InputManager::BroadcastInputEvent(const InputEvent& event) {
    auto* g_coordinator = Coordinator::GetInstance();
    if (event.type == InputEvent::Type::RHYTHM_INPUT)
    {
        auto& transform = g_coordinator->GetComponent<Framework::Transform>(feedbackEntity);  // Use feedbackEntity instead of 0
        transform.isVisible = true;
        transform.visibilityTimer = 0.5f;
        transform.autoHide = true;
    }

    for (const auto& [systemName, callback] : inputListeners) {
        callback(event);
    }
}

bool InputManager::IsKeyJustPressed(int key) const {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return currentKeys[key] && !previousKeys[key];
}

double InputManager::GetInputTiming() const {
    return glfwGetTime();
}

bool InputManager::IsRhythmKeyPressed(int key) const {
    return (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S || key == GLFW_KEY_D) && IsKeyJustPressed(key);
}

void InputManager::UpdateInputStates() {
    std::memcpy(previousKeys, currentKeys, sizeof(currentKeys));

    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++) {
        currentKeys[key] = (glfwGetKey(window, key) == GLFW_PRESS);
    }
}

void InputManager::ProcessRhythmInputs() {
    const int rhythmKeys[] = { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D };

    // Use the synchronized cycle position
    bool inInputWindow = (currentCyclePosition >= 5 && currentCyclePosition <= 8);

    for (int i = 0; i < 4; ++i) {
        if (IsKeyJustPressed(rhythmKeys[i])) {
            if (inInputWindow) {
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
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_W, GLFW_KEY_A}, "Basic Strike");
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_S, GLFW_KEY_D}, "Swift Slash");
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D}, "Power Combo");
}

std::string InputManager::GetTimingFeedback() {
    auto* g_coordinator = Coordinator::GetInstance();
    auto audioSystem = g_coordinator->GetSystem<AudioManager>();

    if (!audioSystem) {
        return "GOOD";
    }

    double musicPosition = audioSystem->GetMusicPosition();
    double beatPosition = fmod(musicPosition, beatInterval);
    double distance = std::min(beatPosition, fabs(beatInterval - beatPosition));

    if (distance <= 0.05) return "PERFECT";
    if (distance <= 0.15) return "GOOD";
    return "MISS";
}

void InputManager::ProcessRhythmTiming(int key) {
    if (!canInput) return;
    auto* g_coordinator = Coordinator::GetInstance();

    inputReceivedOnThisBeat = true;

    if (currentComboSequence.empty()) {
        comboStartBeat = currentBeatIndex;
    }

    currentComboSequence.push_back(key);

    // Get timing quality first
    std::string feedback = GetTimingFeedback();

    // Play SFX immediately when input is processed
    if (auto audio = g_coordinator->GetSystem<AudioManager>()) {
        audio->PlaySound("beat_placeholder", 0.8f);
    }

    // Check if this completes a combo
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

    if (comboCompleted) {
        std::cout << "[INPUT WINDOW - " << feedback << "] Key Pressed: " << KeyToString(key) << " | Combo: " << completedComboName << std::endl;
    }
    else {
        std::cout << "[INPUT WINDOW - " << feedback << "] Key Pressed: " << KeyToString(key) << " | Combo: unsuccessful" << std::endl;
    }
}

void InputManager::CheckForMissedBeat() {
    if (!canInput) return;
    auto* g_coordinator = Coordinator::GetInstance();
    auto audioSystem = g_coordinator->GetSystem<AudioManager>();
    if (!audioSystem) return;

    double currentTime = audioSystem->GetMusicPosition();

    if (currentTime - lastBeatTime < beatInterval) {
        return;
    }

    lastBeatTime = currentTime - fmod(currentTime, beatInterval);
    currentBeatIndex++;

    // Update cycle position once per beat - synchronized
    currentCyclePosition = ((currentBeatIndex - 1) % 8) + 1;

    if (currentCyclePosition >= 1 && currentCyclePosition <= 4) {
        // Beats 1-4: INCOMING phase
        std::cout << "[INCOMING] GET READY.." << std::endl;
        inputReceivedOnThisBeat = false;
    }
    else if (currentCyclePosition >= 5 && currentCyclePosition <= 8) {
        // Beats 5-8: INPUT phase

        // Reset input flag when entering INPUT WINDOW (beat 5)
        if (currentCyclePosition == 5) {
            inputReceivedOnThisBeat = false;
        }

        // Only check for no input if we're past beat 5
        if (currentCyclePosition > 5 && !inputReceivedOnThisBeat) {
            std::cout << "[MISS] NO INPUT DETECTED" << std::endl;
        }

        // End of input window - check for combos
        if (currentCyclePosition == 8) {
            if (successfulCombosThisWindow == 0) {
                playerChances--;
                std::cout << "[MISS] No valid combo completed | Chances left: " << playerChances << std::endl;
            }

            currentComboSequence.clear();
            successfulCombosThisWindow = 0;
            comboStartBeat = currentBeatIndex + 1;
        }
    }

    if (playerChances <= 0) {
        canInput = false;
        std::cout << "[YOU DIED!] No more chances." << std::endl;
        if (audioSystem) audioSystem->StopMusic();
    }

    // Only reset at the end for INCOMING phase
    if (currentCyclePosition <= 4) {
        inputReceivedOnThisBeat = false;
    }
}