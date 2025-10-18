/******************************************************************************/
/**
 * @file        InputManager.h
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Declares the InputManager system, responsible for managing
 *              GLFW keyboard inputs with rhythm-based timing. Provides:
 *                - Real-time input state tracking (per-frame key states).
 *                - Rhythm gameplay support with beat synchronization.
 *                - Sword combo detection (multi-key sequences).
 *                - Debug draw toggles via F1–F6 keys.
 *                - Integration with ECS (Transform + AudioManager).
 *                - Event broadcasting to subscribed systems (InputEvent).
 *
 *              Functions as a standalone input manager for non-ECS contexts.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/


#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include "CoreEngine/ECS/System.h"
#include "CoreEngine/Asset/AssetsManager.h"

// -----------------------------------------------------------------------------
// InputEvent structure for ECS communication
// -----------------------------------------------------------------------------
struct InputEvent {
    enum class Type {
        RHYTHM_INPUT,
        COMBO_EXECUTED,
    };

    Type type;
    int key;
    double timestamp;
    std::string feedback;
    std::string comboName;

    InputEvent(Type t, int k, double time, const std::string& fb = "", const std::string& combo = "")
        : type(t), key(k), timestamp(time), feedback(fb), comboName(combo) {
    }
};

using InputCallback = std::function<void(const InputEvent&)>;

// -----------------------------------------------------------------------------
// InputManager class
// -----------------------------------------------------------------------------
class InputManager : public Systems {
public:
    InputManager();
    ~InputManager();

    void Initialize(GLFWwindow* window);
    void Update(float deltaTime);
    void Shutdown();

    void SetFontEntity(Entity entity) { fontEntity = entity; }
    //Entity GetFeedbackEntity2() const { return feedbackEntity2; }

    // -------------------------------------------------
    // Rhythm game configuration
    // -------------------------------------------------
    float songBPM = 70.0f;
    float beatInterval = 60.0f / songBPM; // seconds per beat
    float beatTolerance = 0.3f;           // timing window
    int playerChances = 3;
    bool canInput = true;
    int maxComboBeats = 4;     // must finish within 4 beats

    bool CanInput() const { return canInput; }

    // -------------------------------------------------
    // System communication
    // -------------------------------------------------
    void RegisterInputListener(const std::string& systemName, InputCallback callback);
    void UnregisterInputListener(const std::string& systemName);
    void BroadcastInputEvent(const InputEvent& event);

    // -------------------------------------------------
    // Basic input queries
    // -------------------------------------------------
    bool IsKeyJustPressed(int key) const;
    double GetInputTiming() const;

    // -------------------------------------------------
    // Rhythm methods
    // -------------------------------------------------
    bool IsRhythmKeyPressed(int key) const;
    void ProcessRhythmTiming(int key);

    // Stats
    int GetSuccessfulCombos() const { return successfulCombos; }

    // Debug helper
    std::set<Entity> DebugEntities;

    // Debug helper
    bool IsDebugRectsEnabled() const { return debugRectsEnabled; }
    bool IsDebugLinesEnabled() const { return debugLinesEnabled; }
    bool IsDebugCirclesEnabled() const { return debugCirclesEnabled; }
    bool IsDebugCirclesFilledEnabled() const { return debugCirclesFilledEnabled; }
    bool IsDebugPointsEnabled() const { return debugPointsEnabled; }
    bool IsDebugGridEnabled() const { return debugGridEnabled; }

private:

    //Entity feedbackEntity;
    //Entity feedbackEntity2;

    Entity fontEntity;
    // -------------------------------------------------
    // Core input state
    // -------------------------------------------- -----
    GLFWwindow* window = nullptr;
    bool currentKeys[GLFW_KEY_LAST + 1];
    bool previousKeys[GLFW_KEY_LAST + 1];

    // Beat tracking
    int currentBeatIndex = 0;
    int comboStartBeat = -1;
    double lastBeatTime = 0.0;
    bool inputReceivedOnThisBeat = false;
    int currentCyclePosition = 1;

    // System callbacks
    std::unordered_map<std::string, InputCallback> inputListeners;

    // -------------------------------------------------
    // Combo system (Now only contains sword)
    // -------------------------------------------------
    struct SwordCombo {
        std::vector<int> keySequence;
        std::string name;

        SwordCombo(const std::vector<int>& keys, const std::string& n)
            : keySequence(keys), name(n) {
        }
    };

    std::vector<SwordCombo> swordCombos;
    std::vector<int> currentComboSequence;
    int successfulCombos = 0;
    int successfulCombosThisWindow = 0;

    // -------------------------------------------------
    // Internal helpers
    // -------------------------------------------------
    void UpdateInputStates();
    void ProcessRhythmInputs();
    void ProcessComboInputs();
    void InitializeSwordCombos();
    std::string GetTimingFeedback();
    void CheckForMissedBeat();

    bool debugRectsEnabled = false;
    bool debugLinesEnabled = false;
    bool debugCirclesEnabled = false;
    bool debugCirclesFilledEnabled = false;
    bool debugPointsEnabled = false;
    bool debugGridEnabled = false;

    bool debugToggleKeyPressed[6] = { false }; // Track F1-F6
};

// Forward declaration for external function
void ToggleObjectVisibility();
