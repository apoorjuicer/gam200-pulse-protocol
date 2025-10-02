// ==================================================================================
// InputManager.cpp  
// Author: Chloe Lau Rey En
// Brief: Implementation of InputManager class with synchronized cycle timing
// ==================================================================================

#include "pch/pch.h"
#include "graphics/stb_image.h"

static std::string KeyToString(int key) {
    switch (key) {
    case GLFW_KEY_W: return "W";
    case GLFW_KEY_A: return "A";
    case GLFW_KEY_S: return "S";
    case GLFW_KEY_D: return "D";
    case GLFW_KEY_Z: return "Z";
    default: return std::to_string(key);
    }
}

static GLuint LoadTexture(const char* filepath) {
    int width, height, channels;
    unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);

    if (!data) {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return textureID;
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

    //if (!Framework::SaveTransform(feedbackTransform, path)) {
    //    std::cerr << "Failed to save transform!\n";
    //}
    //else {
    //    std::cout << "Saved to: " << std::filesystem::absolute(path) << "\n";
    //}

    /* Loaded Transform data from transform.json */
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
    /* Save Transform data from transform.json */
    std::cout << "Testing saving data loaded to save_transform" << "\n";
    std::string saved_path = "../../MyEngine/JSON/save_transform.json";
    new_feedbackTransform.Pos = { 50.0f, 50.0f };
    if (!Framework::SaveTransform(new_feedbackTransform, saved_path)) {
        std::cerr << "Failed to save save_transform!\n";
    }
    else {
        std::cout << "Saved to: " << std::filesystem::absolute(saved_path) << "\n";
    }
    /*Testing prefab assigning new Transform objecct with original transform*/
    Framework::Transform new_feedbackTransform4{ new_feedbackTransform };
    std::cout << "Testing prefab" << "\n";
    std::cout << "new_feedbackTransform4 Pos:" << new_feedbackTransform4.Pos << "\n";
    std::cout << "new_feedbackTransform4 Scale:" << new_feedbackTransform4.Scale << "\n";

   // g_coordinator->AddComponent(entity, new_feedbackTransform);
    
    
    //SetFeedbackEntity(entity);


    // Entity entity = g_coordinator->CreateEntity();

    // Framework::Transform feedbackTransform2{};
    // feedbackTransform2.Pos = { 0.0f, 0.0f };
    // feedbackTransform2.Scale = { 500.0f, 500.0f };
    // feedbackTransform2.mdl_ref = 1.0f;
    // feedbackTransform2.shd_ref = 3.0f;
    // feedbackTransform2.isVisible = true;
    // feedbackTransform2.currentFrame = 1;

    // g_coordinator->AddComponent(entity, feedbackTransform2);
    // feedbackEntity = entity;
}

void InputManager::Update(float deltaTime) {
    UpdateInputStates();

    // Handle multiple debug toggles F1-F6
    const int debugKeys[] = { GLFW_KEY_F1, GLFW_KEY_F2, GLFW_KEY_F3, GLFW_KEY_F4, GLFW_KEY_F5, GLFW_KEY_F6 };
    bool* debugFlags[] = { &debugRectsEnabled, &debugLinesEnabled, &debugCirclesEnabled,
                          &debugCirclesFilledEnabled, &debugPointsEnabled, &debugGridEnabled };
    const char* debugNames[] = { "DEBUG RECTS", "DEBUG LINES", "DEBUG CIRCLES",
                                "DEBUG CIRCLES FILLED", "DEBUG POINTS", "DEBUG GRID" };

    for (int i = 0; i < 6; ++i) {
        bool keyPressed = (glfwGetKey(window, debugKeys[i]) == GLFW_PRESS);

        if (keyPressed && !debugToggleKeyPressed[i]) {
            *debugFlags[i] = !(*debugFlags[i]);
            std::cout << "[" << debugNames[i] << "] " << (*debugFlags[i] ? "ENABLED" : "DISABLED") << std::endl;
        }
        debugToggleKeyPressed[i] = keyPressed;
    }

    ProcessRhythmInputs();
    ProcessComboInputs();
    CheckForMissedBeat();

    auto* g_coordinator = Coordinator::GetInstance();

    // Update fontEntity only
    auto& transform1 = g_coordinator->GetComponent<Framework::Transform>(fontEntity);
    if (transform1.autoHide && transform1.isVisible) {
        transform1.visibilityTimer -= deltaTime;
        if (transform1.visibilityTimer <= 0.0f) {
            transform1.isVisible = false;
            transform1.autoHide = false;
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
        auto& transform1 = g_coordinator->GetComponent<Framework::Transform>(fontEntity);
        transform1.isVisible = true;
        transform1.visibilityTimer = 2.0f;
        transform1.autoHide = true;
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
    
    // Handle Z key for object visibility toggle
    if (IsKeyJustPressed(GLFW_KEY_Z)) {
        // Declare the function as extern to avoid linking issues
        extern void ToggleObjectVisibility();
        ToggleObjectVisibility();
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

        auto* msgMgr = MessageManager::GetInstance();
        InputComboMessage msg(completedComboName, GetInputTiming());
        msgMgr->Publish(msg);
    }
    else {
        std::cout << "[INPUT WINDOW - " << feedback << "] Key Pressed: " << KeyToString(key) << " | Combo: unsuccessful" << std::endl;
    }
}

void InputManager::CheckForMissedBeat() {
    if (!canInput) return;
    auto* g_coordinator = Coordinator::GetInstance();
    auto audioSystem = g_coordinator->GetSystem<AudioManager>();
    if (!audioSystem) {
        std::cout << "[DEBUG] No audio system!\n";
        return;
    }

    double currentTime = audioSystem->GetMusicPosition();
    //std::cout << "[DEBUG] Music position: " << currentTime << " | Last beat: " << lastBeatTime << "\n";

    if (currentTime - lastBeatTime < beatInterval) {
        return;
    }

    lastBeatTime = currentTime - fmod(currentTime, beatInterval);
    currentBeatIndex++;

    // Update cycle position once per beat - synchronized
    currentCyclePosition = ((currentBeatIndex - 1) % 8) + 1;

    // useful to check if the currentTime and lastBeatTime is getting values
    //std::cout << "[DEBUG] Beat " << currentBeatIndex << " | Cycle position: " << currentCyclePosition << "\n";

    if (currentCyclePosition >= 1 && currentCyclePosition <= 4) {
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