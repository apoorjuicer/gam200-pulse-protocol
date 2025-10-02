// ==================================================================================
// InputManager.h
// Author: Chloe Lau Rey En
// Brief: InputManager handles rhythm inputs and sword combos.
//        ECS-compatible but can also operate independently.
// ==================================================================================

#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include "Core/System.h"
#include "Core/AssetsManager.h"

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
    void Update();
    void Shutdown();

    void SetFeedbackEntity(Entity entity) { feedbackEntity = entity; }

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

private:
    Entity feedbackEntity;
    // -------------------------------------------------
    // Core input state
    // -------------------------------------------------
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
};