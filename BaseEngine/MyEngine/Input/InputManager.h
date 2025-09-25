// ==================================================================================
// InputManager.h
// Author: Chloe Lau Rey En
// Brief: InputManager handles rhythm inputs and sord combos.
//        Inherits from Systems for ECS compatibility but operates 
//        independently from SystemManager.
// ==================================================================================

#pragma once
#include "pch/pch.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include "Core/System.h"

// Input event structure for potential future ECS integration
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
        : type(t), key(k), timestamp(time), mouseX(0), mouseY(0), feedback(fb), comboName("") {
    }

    InputEvent(Type t, double x, double y, double time)
        : type(t), key(0), timestamp(time), mouseX(x), mouseY(y), feedback(""), comboName("") {
    }
};

// Callback type for future system communication
using InputCallback = std::function<void(const InputEvent&)>;

class InputManager : public Systems {
public:
    InputManager();
    ~InputManager();

    void Initialize(GLFWwindow* window);
    void Update(); // ECS-style update method
    void Shutdown();

    // Basic input detection - ECS systems can query these
    bool IsKeyPressed(int key) const;
    bool IsKeyJustPressed(int key) const;
    bool IsKeyReleased(int key) const;
    bool IsMousePressed(int button) const;
    bool IsMouseJustPressed(int button) const;
    void GetMousePosition(double& x, double& y) const;
    void GetMouseDelta(double& deltaX, double& deltaY) const;
    void SetMouseSensitivity(float sensitivity);

    // Rhythm game specific methods - for game systems to use
    bool IsRhythmKeyPressed(int key) const;
    bool IsAiming() const;
    double GetInputTiming() const;
    int GetCurrentWeaponSelected() const;

    // event system methods
    void RegisterInputListener(const std::string& systemName, InputCallback callback);
    void UnregisterInputListener(const std::string& systemName);

    // M1 testing and statistics
    //void RunM1Test();
    //void DisplayInputStats() const;

    // Public access to combo stats for other systems
    int GetTotalCombos() const { return totalCombosAttempted; }
    int GetSuccessfulCombos() const { return successfulCombos; }
    double GetComboSuccessRate() const {
        return totalCombosAttempted > 0 ? (double)successfulCombos / totalCombosAttempted : 0.0;
    }

private:
    // Core input state
    GLFWwindow* window;
    bool currentKeys[GLFW_KEY_LAST + 1];
    bool previousKeys[GLFW_KEY_LAST + 1];
    bool currentMouse[GLFW_MOUSE_BUTTON_LAST + 1];
    bool previousMouse[GLFW_MOUSE_BUTTON_LAST + 1];

    // Mouse state
    double mouseX, mouseY;
    double previousMouseX, previousMouseY;
    float mouseSensitivity;
    bool isFirstMouse;

    // ECS integration - for future system communication
    std::unordered_map<std::string, InputCallback> inputListeners;

    // Sword combo system
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
    double lastComboInputTime;
    double comboTimeout;

    // Statistics for ECS systems to access
    int totalRhythmInputs;
    int accurateRhythmInputs;
    int totalCombosAttempted;
    int successfulCombos;

    // M1 testing
    bool m1TestMode;
    double lastStatusTime;

    // Internal ECS-compatible methods
    void UpdateInputStates();
    void ProcessRhythmInputs();
    void ProcessComboInputs();
    void ProcessMouseInputs();
    void BroadcastInputEvent(const InputEvent& event);
    void InitializeSwordCombos();

    // Helper methods for feedback
    std::string GetTimingFeedback() const;
    std::string GetComboQuality(double actualTime, double maxTime) const;

    // Internal state management
    void UpdateKeyStates();
    void UpdateMouseStates();
};