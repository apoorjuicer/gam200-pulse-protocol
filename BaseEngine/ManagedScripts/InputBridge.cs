using System;
using ScriptAPI;

/// <summary>
/// Input bridge that detects keypresses and forwards to ComboSystem.
/// Also updates the InputDisplayUI for visual feedback.
/// </summary>
public class InputBridge : Script
{
    // =========================
    // Config
    // =========================
    public int KeyW = 87;  // GLFW_KEY_W
    public int KeyA = 65;  // GLFW_KEY_A
    public int KeyS = 83;  // GLFW_KEY_S
    public int KeyD = 68;  // GLFW_KEY_D

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private bool lastWState = false;
    private bool lastAState = false;
    private bool lastSState = false;
    private bool lastDState = false;

    // =========================
    // Update Loop
    // =========================
    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            return;
        }

        // Check if input is locked by turn system
        if (TurnManager.Instance != null && TurnManager.Instance.IsInputLocked)
        {
            // Reset key states so we don't register inputs during locked phase
            lastWState = false;
            lastAState = false;
            lastSState = false;
            lastDState = false;
            return;
        }

        // If paused, don't process input
        if (Application.IsPaused())
        {
            return;
        }

        // Check for key presses
        var input = GetInput();

        CheckKey(input, KeyW, ref lastWState, 'W');
        CheckKey(input, KeyA, ref lastAState, 'A');
        CheckKey(input, KeyS, ref lastSState, 'S');
        CheckKey(input, KeyD, ref lastDState, 'D');
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        Console.WriteLine("[InputBridge] Initialized with UI feedback");
        initialized = true;
    }

    // =========================
    // Input Detection
    // =========================
    private void CheckKey(InputComponent input, int keyCode, ref bool lastState, char keyChar)
    {
        bool currentState = input.IsKeyPressed(keyCode);

        // Detect rising edge (key just pressed)
        if (currentState && !lastState)
        {
            OnKeyPressed(keyChar);
        }

        lastState = currentState;
    }

    private void OnKeyPressed(char key)
    {
        Console.WriteLine($"[InputBridge] ?? Key {key} pressed!");

        // Forward to ComboSystem
        if (ComboSystem.Instance != null)
        {
            // Register the input
            ComboSystem.Instance.RegisterInput(key);

            // Get timing feedback from the last registered input
            var hitResults = ComboSystem.Instance.GetHitResults();
            string timing = hitResults.Count > 0 ? hitResults[hitResults.Count - 1] : "UNKNOWN";

            // Update UI display
            if (InputDisplayUI.Instance != null)
            {
                InputDisplayUI.Instance.ShowKeyPress(key, timing);
            }

            // Check if combo is complete
            if (ComboSystem.Instance.HasCompletedCombo())
            {
                Console.WriteLine("[InputBridge] Combo sequence complete!");

                bool success = ComboSystem.Instance.PlayerDidSuccessfulCombo;

                if (success)
                {
                    Console.WriteLine("[InputBridge] ? SUCCESS!");
                }
                else
                {
                    Console.WriteLine("[InputBridge] ? FAILED!");
                }

                // Show result in UI
                if (InputDisplayUI.Instance != null)
                {
                    InputDisplayUI.Instance.ShowComboResult(success);
                }
            }
        }
        else
        {
            Console.WriteLine("[InputBridge] ERROR: ComboSystem.Instance is null!");
        }
    }
}