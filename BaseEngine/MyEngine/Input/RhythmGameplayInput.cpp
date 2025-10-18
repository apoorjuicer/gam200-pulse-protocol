/******************************************************************************/
/**
 * @file        RhythmGameplayInput.cpp
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

static std::string keyToString(int key) {
    switch (key) {
    case GLFW_KEY_W: return "W";
    case GLFW_KEY_A: return "A";
    case GLFW_KEY_S: return "S";
    case GLFW_KEY_D: return "D";
    case GLFW_KEY_Z: return "Z";
    default: return std::to_string(key);
    }
}

RhythmGameplayInput::RhythmGameplayInput() {
    successfulCombos = 0;
    successfulCombosThisWindow = 0;
    currentCyclePosition = 1;
    initializeSwordCombos();
}

RhythmGameplayInput::~RhythmGameplayInput() {
    Shutdown();
}

void RhythmGameplayInput::Initialize() {
    std::cout << "[RhythmGameplayInput] Successfully initialized." << std::endl;
    std::cout << "[COMBOS READY] Available combos: Basic Strike (W->A), Swift Slash (S->D), Power Combo (A->S->D)" << std::endl;

    currentBeatIndex = 0;
    comboStartBeat = -1;
    lastBeatTime = 0.0;
    currentCyclePosition = 1;

    auto* g_coordinator = Coordinator::GetInstance();

    Entity fontEntityObj = g_coordinator->CreateEntity();
    Framework::Transform fontTr{};
    fontTr.Pos = { 0.0f, 0.0f };
    fontTr.Scale = { 500.0f, 500.0f };
    fontTr.mdl_ref = 1.0f;
    fontTr.shd_ref = 3.0f;
    fontTr.isVisible = true;
    fontTr.currentFrame = 1;

    g_coordinator->AddComponent(fontEntityObj, fontTr);
    fontEntity = fontEntityObj;

    // Loaded Transform data from transform.json
    std::string path = "../../MyEngine/JSON/transform.json";
    Framework::Transform new_feedbackTransform;

    if (!Framework::LoadTransform(new_feedbackTransform, path)) {
        std::cerr << "Failed to load new_feedbackTransform!\n";
    }
    else {
        std::cout << "Loaded new_feedbackTransform From: " << std::filesystem::absolute(path) << "\n";
    }
    std::cout << "Testing data loaded from transform" << "\n";
    std::cout << "new_feedbackTransform: Pos" << new_feedbackTransform.Pos << "\n";
    std::cout << "new_feedbackTransform: Scale" << new_feedbackTransform.Scale << "\n";

    // Save Transform data to save_transform.json
    std::cout << "Testing saving data loaded to save_transform" << "\n";
    std::string saved_path = "../../MyEngine/JSON/save_transform.json";
    new_feedbackTransform.Pos = { 50.0f, 50.0f };
    if (!Framework::SaveTransform(new_feedbackTransform, saved_path)) {
        std::cerr << "Failed to save save_transform!\n";
    }
    else {
        std::cout << "Saved to: " << std::filesystem::absolute(saved_path) << "\n";
    }

    // Testing prefab assigning new Transform object with original transform
    Framework::Transform new_feedbackTransform4{ new_feedbackTransform };
    std::cout << "Testing prefab" << "\n";
    std::cout << "new_feedbackTransform4 Pos:" << new_feedbackTransform4.Pos << "\n";
    std::cout << "new_feedbackTransform4 Scale:" << new_feedbackTransform4.Scale << "\n";
}

void RhythmGameplayInput::Update(float deltaTime) {
    auto* g_coordinator = Coordinator::GetInstance();

    processRhythmInputs();
    processComboInputs();
    checkForMissedBeat();

    // Update fontEntity visibility timer
    auto& transform1 = g_coordinator->GetComponent<Framework::Transform>(fontEntity);
    if (transform1.autoHide && transform1.isVisible) {
        transform1.visibilityTimer -= deltaTime;
        if (transform1.visibilityTimer <= 0.0f) {
            transform1.isVisible = false;
            transform1.autoHide = false;
        }
    }
}

void RhythmGameplayInput::Shutdown() {
    inputListeners.clear();
    std::cout << "[RhythmGameplayInput] Shutdown completed." << std::endl;
}

void RhythmGameplayInput::registerInputListener(const std::string& systemName, InputCallback callback) {
    inputListeners[systemName] = callback;
}

void RhythmGameplayInput::unregisterInputListener(const std::string& systemName) {
    inputListeners.erase(systemName);
}

void RhythmGameplayInput::broadcastInputEvent(const InputEvent& event) {
    auto* g_coordinator = Coordinator::GetInstance();
    if (event.type == InputEvent::Type::RHYTHM_INPUT) {
        auto& transform1 = g_coordinator->GetComponent<Framework::Transform>(fontEntity);
        transform1.isVisible = true;
        transform1.visibilityTimer = 2.0f;
        transform1.autoHide = true;
    }

    for (const auto& [systemName, callback] : inputListeners) {
        callback(event);
    }
}

void RhythmGameplayInput::processRhythmInputs() {
    auto* g_coordinator = Coordinator::GetInstance();
    auto inputMgr = g_coordinator->GetSystem<InputManager>();

    const int rhythmKeys[] = { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D };
    bool inInputWindow = (currentCyclePosition >= 5 && currentCyclePosition <= 8);

    for (int i = 0; i < 4; ++i) {
        if (inputMgr->isKeyTriggered(rhythmKeys[i])) {
            if (inInputWindow) {
                processRhythmTiming(rhythmKeys[i]);
                std::string feedback = getTimingFeedback();
                InputEvent rhythmEvent(InputEvent::Type::RHYTHM_INPUT, rhythmKeys[i], glfwGetTime(), feedback);
                broadcastInputEvent(rhythmEvent);
            }
        }
    }

    if (inputMgr->isKeyTriggered(GLFW_KEY_Z)) {
        extern void ToggleObjectVisibility();
        ToggleObjectVisibility();
    }
}

void RhythmGameplayInput::processComboInputs() {
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
                    InputEvent comboEvent(InputEvent::Type::COMBO_EXECUTED, 0, glfwGetTime(), "COMBO", combo.name);
                    broadcastInputEvent(comboEvent);
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

void RhythmGameplayInput::initializeSwordCombos() {
    swordCombos.clear();
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_W, GLFW_KEY_A}, "Basic Strike");
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_S, GLFW_KEY_D}, "Swift Slash");
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D}, "Power Combo");
}

std::string RhythmGameplayInput::getTimingFeedback() {
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

void RhythmGameplayInput::processRhythmTiming(int key) {
    if (!canInput) return;
    auto* g_coordinator = Coordinator::GetInstance();

    inputReceivedOnThisBeat = true;

    if (currentComboSequence.empty()) {
        comboStartBeat = currentBeatIndex;
    }

    currentComboSequence.push_back(key);

    std::string feedback = getTimingFeedback();

    if (auto audio = g_coordinator->GetSystem<AudioManager>()) {
        audio->PlaySound("beat_placeholder", 0.8f);
    }

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
        std::cout << "[INPUT WINDOW - " << feedback << "] Key Pressed: " << keyToString(key) << " | Combo: " << completedComboName << std::endl;

        auto* msgMgr = MessageManager::GetInstance();
        InputComboMessage msg(completedComboName, glfwGetTime());
        msgMgr->Publish(msg);
    }
    else {
        std::cout << "[INPUT WINDOW - " << feedback << "] Key Pressed: " << keyToString(key) << " | Combo: unsuccessful" << std::endl;
    }
}

void RhythmGameplayInput::checkForMissedBeat() {
    if (!canInput) return;
    auto* g_coordinator = Coordinator::GetInstance();
    auto audioSystem = g_coordinator->GetSystem<AudioManager>();
    if (!audioSystem) {
        return;
    }

    double currentTime = audioSystem->GetMusicPosition();

    if (currentTime - lastBeatTime < beatInterval) {
        return;
    }

    lastBeatTime = currentTime - fmod(currentTime, beatInterval);
    currentBeatIndex++;

    currentCyclePosition = ((currentBeatIndex - 1) % 8) + 1;

    if (currentCyclePosition >= 1 && currentCyclePosition <= 4) {
        std::cout << "[INCOMING] GET READY.." << std::endl;
        inputReceivedOnThisBeat = false;
    }
    else if (currentCyclePosition >= 5 && currentCyclePosition <= 8) {
        if (currentCyclePosition == 5) {
            inputReceivedOnThisBeat = false;
        }

        if (currentCyclePosition > 5 && !inputReceivedOnThisBeat) {
            std::cout << "[MISS] NO INPUT DETECTED" << std::endl;
        }

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

    if (currentCyclePosition <= 4) {
        inputReceivedOnThisBeat = false;
    }
}