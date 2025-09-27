// ==================================================================================
// InputManager.h
// Author: Chloe Lau Rey En
// Brief: InputManager handles rhythm inputs and sword combos.
//        ECS-compatible but can also operate independently.
// ==================================================================================

#pragma once
#include "pch/pch.h"
#include <GLFW/glfw3.h>
#include "Core/System.h"
#include "Core/SystemManager.h"

// -----------------------------------------------------------------------------
// InputEvent structure for ECS communication
// -----------------------------------------------------------------------------
struct InputEvent {
    enum class Type {
        RHYTHM_INPUT,
        COMBO_EXECUTED,
        MOUSE_INPUT
    };

    Type type;
    int key;
    double timestamp;
    double mouseX, mouseY;
    std::string feedback;
    std::string comboName;

    InputEvent(Type t, int k, double time, const std::string& fb = "")
        : type(t), key(k), timestamp(time),
        mouseX(0), mouseY(0), feedback(fb), comboName("") {
    }

    InputEvent(Type t, double x, double y, double time)
        : type(t), key(0), timestamp(time),
        mouseX(x), mouseY(y), feedback(""), comboName("") {
    }
};

using InputCallback = std::function<void(const InputEvent&)>;

// -----------------------------------------------------------------------------
// InputManager class
// -----------------------------------------------------------------------------
class InputManager : public Systems {
public:
	InputManager();
	~InputManager(); // destructor

    void Initialize(GLFWwindow* window);
    void Update();
    void Shutdown();

    // -------------------------------------------------
    // Rhythm game configuration
    // -------------------------------------------------
    float songBPM = 70.0f;
    float beatInterval = 60.0f / songBPM; // seconds per beat
    float beatTolerance = 0.2f;           // timing window
    int playerChances = 3;
    bool canInput = true;
    int maxComboBeats = 4;     // must finish within 4 beats

    bool CanInput() const { return canInput; }

    // -------------------------------------------------
    // ECS / Input Event communication
    // -------------------------------------------------
    void RegisterInputListener(const std::string& systemName, InputCallback callback);
    void UnregisterInputListener(const std::string& systemName);

    // -------------------------------------------------
    // Input querying (for other systems)
    // -------------------------------------------------
    bool IsKeyPressed(int key) const;
    bool IsKeyJustPressed(int key) const;
    bool IsKeyReleased(int key) const;
    bool IsMousePressed(int button) const;
    bool IsMouseJustPressed(int button) const;
    void GetMousePosition(double& x, double& y) const;
    void GetMouseDelta(double& deltaX, double& deltaY) const;
    void SetMouseSensitivity(float sensitivity);

    // -------------------------------------------------
    // Rhythm gameplay methods
    // -------------------------------------------------
    bool IsRhythmKeyPressed(int key) const;
    bool IsAiming() const;
    double GetInputTiming() const;
    int GetCurrentWeaponSelected() const;
    bool IsInputOnBeat();
    void ProcessRhythmTiming(int key);

    // Combo stats
    int GetTotalCombos() const { return totalCombosAttempted; }
    int GetSuccessfulCombos() const { return successfulCombos; }
    double GetComboSuccessRate() const {
        return totalCombosAttempted > 0 ? (double)successfulCombos / totalCombosAttempted : 0.0;
    }

private:
    // -------------------------------------------------
    // Core input state
    // -------------------------------------------------
    GLFWwindow* window = nullptr;
    bool currentKeys[GLFW_KEY_LAST + 1];
    bool previousKeys[GLFW_KEY_LAST + 1];
    bool currentMouse[GLFW_MOUSE_BUTTON_LAST + 1];
    bool previousMouse[GLFW_MOUSE_BUTTON_LAST + 1];

    // Mouse
    double mouseX = 0.0, mouseY = 0.0;
    double previousMouseX = 0.0, previousMouseY = 0.0;
    float mouseSensitivity = 1.0f;
    bool isFirstMouse = true;

    // Beat tracking
    int currentBeatIndex = 0;
    int comboStartBeat = -1;
    double lastBeatTime = 0.0;
    bool inputReceivedOnThisBeat = false;

    // Countdown state
    bool inCountdown = true;
    int countdownBeats = 4;   // 4..3..2..1..

    // ECS callbacks
    std::unordered_map<std::string, InputCallback> inputListeners;

    // -------------------------------------------------
    // Combos
    // -------------------------------------------------
    struct SwordCombo {
        std::vector<int> keySequence;
        std::string name;
        double maxTimeBetweenInputs;

        SwordCombo(const std::vector<int>& keys, const std::string& n, double maxTime)
            : keySequence(keys), name(n), maxTimeBetweenInputs(maxTime) {
        }
    };

    std::vector<SwordCombo> swordCombos;
    std::vector<int> currentComboSequence;
    double lastComboInputTime = 0.0;
    double comboTimeout = 2.0;

    // -------------------------------------------------
    // Stats
    // -------------------------------------------------
    int totalRhythmInputs = 0;
    int accurateRhythmInputs = 0;
    int totalCombosAttempted = 0;
    int successfulCombos = 0;
    int successfulCombosThisWindow = 0;

    // -------------------------------------------------
    // Internal helpers
    // -------------------------------------------------
    void UpdateInputStates();
    void ProcessRhythmInputs();
    void ProcessComboInputs();
    void ProcessMouseInputs();
    void BroadcastInputEvent(const InputEvent& event);
    void InitializeSwordCombos();
    std::string GetTimingFeedback();
    void UpdateKeyStates();
    void UpdateMouseStates();
    void CheckForMissedBeat();
};
