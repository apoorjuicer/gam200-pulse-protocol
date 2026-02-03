using System;
using ScriptAPI;

/// <summary>
/// CRITICAL: This script bridges C++ input (RhythmGameplayInput.cpp) to C# combo system
/// 
/// HOW IT WORKS:
/// 1. C++ RhythmGameplayInput detects W/A/S/D keys
/// 2. This script reads those inputs and forwards to ComboSystem
/// 3. TurnManager controls when input is allowed via Application.SetPaused()
/// 
/// SETUP:
/// - Attach to same entity as Conductor, TurnManager, ComboSystem
/// - C++ RhythmGameplayInput.cpp should respect Application.IsPaused()
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

        // Check for key presses (manual detection since C++ might not be calling us)
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
        Console.WriteLine("[InputBridge] ========================================");
        Console.WriteLine("[InputBridge] INITIALIZING INPUT BRIDGE");
        Console.WriteLine("[InputBridge] ========================================");
        Console.WriteLine("[InputBridge] This script forwards W/A/S/D inputs to ComboSystem");
        Console.WriteLine("[InputBridge] ");
        Console.WriteLine("[InputBridge] IMPORTANT: Your C++ RhythmGameplayInput.cpp should:");
        Console.WriteLine("[InputBridge]   1. Check Application.IsPaused() before processing input");
        Console.WriteLine("[InputBridge]   2. OR let this C# script handle ALL input");
        Console.WriteLine("[InputBridge] ");
        Console.WriteLine("[InputBridge] ========================================");

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
        Console.WriteLine($"[InputBridge] Key {key} pressed!");

        // Forward to ComboSystem
        if (ComboSystem.Instance != null)
        {
            ComboSystem.Instance.RegisterInput(key);

            // Play audio feedback
            var audio = GetAudio();
            if (audio != null)
            {
                switch (key)
                {
                    case 'W':
                        audio.PlaySFX("SFX_W_Key");
                        break;
                    case 'A':
                        audio.PlaySFX("SFX_A_Key");
                        break;
                    case 'S':
                        audio.PlaySFX("SFX_S_Key");
                        break;
                    case 'D':
                        audio.PlaySFX("SFX_D_Key");
                        break;
                }
            }

            // Check if combo is complete
            if (ComboSystem.Instance.HasCompletedCombo())
            {
                Console.WriteLine("[InputBridge] Combo sequence complete!");

                if (ComboSystem.Instance.PlayerDidSuccessfulCombo)
                {
                    // Play success sound
                    if (audio != null)
                    {
                        audio.PlaySFX("SFX_SwordSwing");  // Or whatever your success sound is
                    }
                }
                else
                {
                    // Play failure sound
                    if (audio != null)
                    {
                        audio.PlaySFX("SFX_WrongInput");
                    }
                }
            }
        }
        else
        {
            Console.WriteLine("[InputBridge] ERROR: ComboSystem.Instance is null!");
        }
    }
}