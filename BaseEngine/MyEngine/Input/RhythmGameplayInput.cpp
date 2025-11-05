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

#include "pch/pch_temp.h"
#include "../MyEngine/Input/RhythmGameplayInput.h"
#include "../MyEngine/CoreEngine/Core/CoreEngine.h"

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
    //std::cout << "[RhythmGameplayInput] Successfully initialized." << std::endl;
    //std::cout << "[COMBOS READY] Available combos: Basic Strike (W->A), Swift Slash (S->D), Power Combo (A->S->D)" << std::endl;

    currentBeatIndex = 0;
    comboStartBeat = -1;
    lastBeatTime = 0.0;
    currentCyclePosition = 1;

    fontEntity = 0;

    // Initialize combos
    initializeSwordCombos();

    //auto* g_coordinator = Coordinator::GetInstance();
    //if (!g_coordinator) {
    //    std::cerr << "[RhythmGameplayInput] ERROR: Coordinator is null!" << std::endl;
    //    return;
    //}

    //// finds feedback bar
    //auto activeEntities = g_coordinator->GetAllEntities();

    //for (auto entity : activeEntities) {
    //    if (g_coordinator->HasComponent<Name>(entity)) {
    //        auto& nameComp = g_coordinator->GetComponent<Name>(entity);
    //        if (nameComp.name == "FeedbackBar") {
    //            fontEntity = entity;

    //            if (g_coordinator->HasComponent<GUIButton>(entity)) {
    //                auto& button = g_coordinator->GetComponent<GUIButton>(entity);
    //                button.normalColor.a = 0.0f;
    //                button.hoverColor.a = 0.0f;
    //            }

    //            if (g_coordinator->HasComponent<Framework::Transform>(entity)) {
    //                auto& transform = g_coordinator->GetComponent<Framework::Transform>(entity);
    //                transform.isVisible = false;
    //            }

    //            std::cout << "[RhythmGameplayInput] Found FeedbackBar: " << entity << std::endl;
    //            break;
    //        }
    //    }
    //}

    //if (fontEntity == 0) {
    //    std::cerr << "[RhythmGameplayInput] WARNING: FeedbackBar entity not found!" << std::endl;
    //}
}

void RhythmGameplayInput::Update(float deltaTime) {
    auto* g_coordinator = Coordinator::GetInstance();

    //if (fontEntity == 0) {
    //    return; // skips if feedback bar is not found
    //}

    processRhythmInputs();
    processComboInputs();

    // Timer logic
    /*if (g_coordinator->HasComponent<Framework::Transform>(fontEntity)) {
        auto& transform = g_coordinator->GetComponent<Framework::Transform>(fontEntity);
        if (transform.autoHide) {
            transform.visibilityTimer -= deltaTime;

            if (transform.visibilityTimer <= 0.0f) {
                transform.isVisible = false;
                transform.autoHide = false;

                if (g_coordinator->HasComponent<GUIButton>(fontEntity)) {
                    auto& button = g_coordinator->GetComponent<GUIButton>(fontEntity);
                    button.normalColor.a = 0.0f;
                    button.hoverColor.a = 0.0f;
                }
            }
        }
    }*/
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
    //if (event.type == InputEvent::Type::RHYTHM_INPUT) {
    //    auto& transform1 = g_coordinator->GetComponent<Framework::Transform>(fontEntity);
    //    transform1.isVisible = true;
    //    transform1.visibilityTimer = 2.0f;
    //    transform1.autoHide = true;
    //}

    for (const auto& [systemName, callback] : inputListeners) {
        callback(event);
    }
}

void RhythmGameplayInput::processRhythmInputs() {
    auto* g_coordinator = Coordinator::GetInstance();
    auto inputMgr = g_coordinator->GetSystem<InputManager>();

    //if (Engine->IsPaused()) {
    //    return;
    //}

    const int rhythmKeys[] = { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D };
    const char* keyNames[] = { "W", "A", "S", "D" };

    for (int i = 0; i < 4; ++i) {
        if (inputMgr->isKeyTriggered(rhythmKeys[i])) {

            std::cout << "[KEY DETECTED] " << keyNames[i] << " key pressed!" << std::endl;

            // Play sound
            //if (auto audio = g_coordinator->GetSystem<AudioManager>()) {
            //    audio->PlaySound("beat_placeholder", 0.8f);
            //}

            //// Show feedback bar by setting alpha to 1.0
            //if (g_coordinator->HasComponent<GUIButton>(fontEntity)) {
            //    auto& button = g_coordinator->GetComponent<GUIButton>(fontEntity);
            //    button.normalColor.a = 1.0f;  // Make visible
            //    button.hoverColor.a = 1.0f;

            //    auto& transform = g_coordinator->GetComponent<Framework::Transform>(fontEntity);
            //    transform.visibilityTimer = 2.0f;
            //    transform.autoHide = true;
            //}

            processRhythmTiming(rhythmKeys[i]);
            broadcastInputEvent(InputEvent(InputEvent::Type::RHYTHM_INPUT, rhythmKeys[i], glfwGetTime()));
        }
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

    // Play beat sound ONCE
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
        // Play combo success sound INSTEAD of beat sound
        if (auto audio = g_coordinator->GetSystem<AudioManager>()) {
            audio->PlaySound("combo_success", 0.9f);
        }

        std::cout << "[INPUT WINDOW - " << feedback << "] Key Pressed: " << keyToString(key)
            << " | Combo: " << completedComboName << std::endl;

        auto* msgMgr = MessageManager::GetInstance();
        InputComboMessage msg(completedComboName, glfwGetTime());
        msgMgr->Publish(msg);
    }
    else {
        // Play regular beat sound for non-combo or incomplete combo keys
        if (auto audio = g_coordinator->GetSystem<AudioManager>()) {
            audio->PlaySound("beat_placeholder", 0.8f);
        }

        std::cout << "[INPUT WINDOW - " << feedback << "] Key Pressed: " << keyToString(key)
            << " | Combo: unsuccessful" << std::endl;
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