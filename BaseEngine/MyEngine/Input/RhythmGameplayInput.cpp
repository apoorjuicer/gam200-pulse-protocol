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

static std::string keyToString(int key)
{
    switch (key)
    {
    case GLFW_KEY_W:
        return "W";
    case GLFW_KEY_A:
        return "A";
    case GLFW_KEY_S:
        return "S";
    case GLFW_KEY_D:
        return "D";
    case GLFW_KEY_Z:
        return "Z";
    default:
        return std::to_string(key);
    }
}

RhythmGameplayInput::RhythmGameplayInput()
{
    successfulCombos = 0;
    successfulCombosThisWindow = 0;
    currentCyclePosition = 1;
    initializeSwordCombos();
}

RhythmGameplayInput::~RhythmGameplayInput()
{
    Shutdown();
}

void RhythmGameplayInput::Initialize()
{
    // std::cout << "[RhythmGameplayInput] Successfully initialized." << std::endl;
    // std::cout << "[COMBOS READY] Available combos: Basic Strike (W->A), Swift Slash (S->D), Power Combo (A->S->D)" << std::endl;

    currentBeatIndex = 0;
    comboStartBeat = -1;
    lastBeatTime = 0.0;
    currentCyclePosition = 1;

    fontEntity = 0;

    // Initialize combos
    initializeSwordCombos();
}

void RhythmGameplayInput::Update(float deltaTime)
{
    auto *g_coordinator = Coordinator::GetInstance();

    // FIND FeedbackBar on first update
    if (fontEntity == 0)
    {
        auto activeEntities = g_coordinator->GetAllEntities();

        for (auto entity : activeEntities)
        {
            if (g_coordinator->HasComponent<Name>(entity))
            {
                auto &nameComp = g_coordinator->GetComponent<Name>(entity);
                if (nameComp.name == "FeedbackBar")
                {
                    fontEntity = entity;
                    // std::cout << "========================================" << std::endl;
                    // std::cout << "[RhythmGameplayInput] Found FeedbackBar: " << entity << std::endl;

                    // Check components
                    bool hasTransform = g_coordinator->HasComponent<Framework::Transform>(entity);
                    bool hasGUIButton = g_coordinator->HasComponent<GUIButton>(entity);
                    bool hasRenderable = g_coordinator->HasComponent<Framework::Renderable>(entity);

                    // std::cout << "[DEBUG] Has Transform: " << (hasTransform ? "YES" : "NO") << std::endl;
                    // std::cout << "[DEBUG] Has GUIButton: " << (hasGUIButton ? "YES" : "NO") << std::endl;
                    // std::cout << "[DEBUG] Has Renderable: " << (hasRenderable ? "YES" : "NO") << std::endl;

                    if (hasGUIButton)
                    {
                        auto &button = g_coordinator->GetComponent<GUIButton>(entity);
                        // std::cout << "[DEBUG] Initial normalColor alpha: " << button.normalColor.a << std::endl;
                        button.normalColor.a = 0.0f;
                        button.hoverColor.a = 0.0f;
                        /*std::cout << "[DEBUG] Set alpha to 0.0" << std::endl;*/
                    }

                    if (hasTransform)
                    {
                        auto &transform = g_coordinator->GetComponent<Framework::Transform>(entity);
                        /*std::cout << "[DEBUG] isVisible: " << transform.isVisible << std::endl;*/
                        transform.isVisible = false;
                        /*std::cout << "[DEBUG] Set isVisible to false" << std::endl;*/
                    }

                    /*std::cout << "========================================" << std::endl;*/
                    break;
                }
            }
        }

        if (fontEntity == 0)
        {
            return;
        }
    }

    processRhythmInputs();
    processComboInputs();

    // Timer logic
    if (fontEntity != 0)
    {
        if (g_coordinator->HasComponent<Framework::Transform>(fontEntity))
        {
            auto &transform = g_coordinator->GetComponent<Framework::Transform>(fontEntity);
            if (transform.autoHide)
            {
                transform.visibilityTimer -= deltaTime;
                // std::cout << "[TIMER] Visibility timer: " << transform.visibilityTimer << std::endl;

                if (transform.visibilityTimer <= 0.0f)
                {
                    // std::cout << "[HIDING] Hiding FeedbackBar now!" << std::endl;
                    transform.isVisible = false;
                    transform.autoHide = false;

                    if (g_coordinator->HasComponent<GUIButton>(fontEntity))
                    {
                        auto &button = g_coordinator->GetComponent<GUIButton>(fontEntity);
                        button.normalColor.a = 0.0f;
                        button.hoverColor.a = 0.0f;
                    }
                }
            }
        }
    }
}

void RhythmGameplayInput::Shutdown()
{
    inputListeners.clear();
    std::cout << "[RhythmGameplayInput] Shutdown completed." << std::endl;
}

void RhythmGameplayInput::registerInputListener(const std::string &systemName, InputCallback callback)
{
    inputListeners[systemName] = callback;
}

void RhythmGameplayInput::unregisterInputListener(const std::string &systemName)
{
    inputListeners.erase(systemName);
}

void RhythmGameplayInput::broadcastInputEvent(const InputEvent &event)
{
    auto *g_coordinator = Coordinator::GetInstance();
    if (event.type == InputEvent::Type::RHYTHM_INPUT)
    {
        auto &transform1 = g_coordinator->GetComponent<Framework::Transform>(fontEntity);
        transform1.isVisible = true;
        transform1.visibilityTimer = 2.0f;
        transform1.autoHide = true;
    }

    for (const auto &[systemName, callback] : inputListeners)
    {
        callback(event);
    }
}

void RhythmGameplayInput::processRhythmInputs()
{
    auto *g_coordinator = Coordinator::GetInstance();
    auto inputMgr = g_coordinator->GetSystem<InputManager>();

    if (Engine->IsPaused())
    {
        return;
    }

    const int rhythmKeys[] = {GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D};
    const char *keyNames[] = {"W", "A", "S", "D"};

    for (int i = 0; i < 4; ++i)
    {
        if (inputMgr->isKeyTriggered(rhythmKeys[i]))
        {

            std::cout << "[KEY DETECTED] " << keyNames[i] << " key pressed!" << std::endl;

            // Play sound
            if (auto audio = g_coordinator->GetSystem<AudioManager>())
            {
                audio->PlaySound("beat_placeholder", 0.8f);
            }

            // Show feedback bar by setting alpha to 1.0
            if (g_coordinator->HasComponent<GUIButton>(fontEntity))
            {
                auto &button = g_coordinator->GetComponent<GUIButton>(fontEntity);
                button.normalColor.a = 1.0f; // Make visible
                button.hoverColor.a = 1.0f;

                auto &transform = g_coordinator->GetComponent<Framework::Transform>(fontEntity);
                transform.visibilityTimer = 2.0f;
                transform.autoHide = true;
            }

            processRhythmTiming(rhythmKeys[i]);
            broadcastInputEvent(InputEvent(InputEvent::Type::RHYTHM_INPUT, rhythmKeys[i], glfwGetTime()));
        }
    }
}

void RhythmGameplayInput::processComboInputs()
{
    if (currentComboSequence.empty())
        return;

    for (const auto &combo : swordCombos)
    {
        if (currentComboSequence.size() == combo.keySequence.size())
        {
            if (currentComboSequence == combo.keySequence)
            {
                successfulCombos++;
                successfulCombosThisWindow++;

                bool isPartOfLongerCombo = false;
                for (const auto &longerCombo : swordCombos)
                {
                    if (longerCombo.keySequence.size() > combo.keySequence.size())
                    {
                        bool matches = true;
                        for (size_t i = 0; i < combo.keySequence.size(); ++i)
                        {
                            if (longerCombo.keySequence[i] != combo.keySequence[i])
                            {
                                matches = false;
                                break;
                            }
                        }
                        if (matches)
                        {
                            isPartOfLongerCombo = true;
                            break;
                        }
                    }
                }

                if (!isPartOfLongerCombo)
                {
                    InputEvent comboEvent(InputEvent::Type::COMBO_EXECUTED, 0, glfwGetTime(), "COMBO", combo.name);
                    broadcastInputEvent(comboEvent);
                    currentComboSequence.clear();
                }
                return;
            }
        }
    }

    bool isValidPrefix = false;
    for (const auto &combo : swordCombos)
    {
        if (currentComboSequence.size() < combo.keySequence.size())
        {
            bool matches = true;
            for (size_t i = 0; i < currentComboSequence.size(); ++i)
            {
                if (currentComboSequence[i] != combo.keySequence[i])
                {
                    matches = false;
                    break;
                }
            }
            if (matches)
            {
                isValidPrefix = true;
                break;
            }
        }
    }

    if (!isValidPrefix)
    {
        currentComboSequence.clear();
    }
}

void RhythmGameplayInput::initializeSwordCombos()
{
    swordCombos.clear();
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_W, GLFW_KEY_A}, "Basic Strike");
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_S, GLFW_KEY_D}, "Swift Slash");
    swordCombos.emplace_back(std::vector<int>{GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D}, "Power Combo");
}

std::string RhythmGameplayInput::getTimingFeedback()
{
    auto *g_coordinator = Coordinator::GetInstance();
    auto audioSystem = g_coordinator->GetSystem<AudioManager>();

    if (!audioSystem)
    {
        return "GOOD";
    }

    double musicPosition = audioSystem->GetBGMPositionSeconds();
    double beatPosition = fmod(musicPosition, beatInterval);
    double distance = std::min(beatPosition, fabs(beatInterval - beatPosition));

    if (distance <= 0.05)
        return "PERFECT";
    if (distance <= 0.15)
        return "GOOD";
    return "MISS";
}

void RhythmGameplayInput::processRhythmTiming(int key)
{
    if (!canInput)
        return;
    auto *g_coordinator = Coordinator::GetInstance();

    inputReceivedOnThisBeat = true;

    if (currentComboSequence.empty())
    {
        comboStartBeat = currentBeatIndex;
    }

    currentComboSequence.push_back(key);

    std::string feedback = getTimingFeedback();

    if (auto audio = g_coordinator->GetSystem<AudioManager>())
    {
        audio->PlaySound("beat_placeholder");
    }

    bool comboCompleted = false;
    std::string completedComboName = "";

    for (const auto &combo : swordCombos)
    {
        if (currentComboSequence.size() == combo.keySequence.size() &&
            currentComboSequence == combo.keySequence)
        {
            comboCompleted = true;
            completedComboName = combo.name;
            break;
        }
    }

    if (comboCompleted)
    {
        std::cout << "[INPUT WINDOW - " << feedback << "] Key Pressed: " << keyToString(key) << " | Combo: " << completedComboName << std::endl;

        auto *msgMgr = MessageManager::GetInstance();
        InputComboMessage msg(completedComboName, glfwGetTime());
        msgMgr->Publish(msg);
    }
    else
    {
        std::cout << "[INPUT WINDOW - " << feedback << "] Key Pressed: " << keyToString(key) << " | Combo: unsuccessful" << std::endl;
    }
}

void RhythmGameplayInput::checkForMissedBeat()
{
    if (!canInput)
        return;
    auto *g_coordinator = Coordinator::GetInstance();
    auto audioSystem = g_coordinator->GetSystem<AudioManager>();
    if (!audioSystem)
    {
        return;
    }

    double currentTime = audioSystem->GetBGMPositionSeconds();

    if (currentTime - lastBeatTime < beatInterval)
    {
        return;
    }

    lastBeatTime = currentTime - fmod(currentTime, beatInterval);
    currentBeatIndex++;

    currentCyclePosition = ((currentBeatIndex - 1) % 8) + 1;

    if (currentCyclePosition >= 1 && currentCyclePosition <= 4)
    {
        std::cout << "[INCOMING] GET READY.." << std::endl;
        inputReceivedOnThisBeat = false;
    }
    else if (currentCyclePosition >= 5 && currentCyclePosition <= 8)
    {
        if (currentCyclePosition == 5)
        {
            inputReceivedOnThisBeat = false;
        }

        if (currentCyclePosition > 5 && !inputReceivedOnThisBeat)
        {
            std::cout << "[MISS] NO INPUT DETECTED" << std::endl;
        }

        if (currentCyclePosition == 8)
        {
            if (successfulCombosThisWindow == 0)
            {
                playerChances--;
                std::cout << "[MISS] No valid combo completed | Chances left: " << playerChances << std::endl;
            }

            currentComboSequence.clear();
            successfulCombosThisWindow = 0;
            comboStartBeat = currentBeatIndex + 1;
        }
    }

    if (playerChances <= 0)
    {
        canInput = false;
        std::cout << "[YOU DIED!] No more chances." << std::endl;
        if (audioSystem)
            audioSystem->StopBGM();
    }

    if (currentCyclePosition <= 4)
    {
        inputReceivedOnThisBeat = false;
    }
}