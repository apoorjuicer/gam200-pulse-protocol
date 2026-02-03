using System;
using ScriptAPI;

public class PlayerInputHandler : Script
{
    // =========================
    // References
    // =========================
    public ComboSystem ComboSystemRef = null;
    public TurnManager TurnManagerRef = null;

    // =========================
    // Input Settings (GLFW key codes)
    // =========================
    public int KeyW = 87;  // GLFW_KEY_W
    public int KeyA = 65;  // GLFW_KEY_A
    public int KeyS = 83;  // GLFW_KEY_S
    public int KeyD = 68;  // GLFW_KEY_D

    // =========================
    // State
    // =========================
    private bool initialized = false;

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

        // Only accept input during player turn
        if (TurnManagerRef != null && TurnManagerRef.IsInputLocked)
        {
            return;
        }

        // IMPORTANT: This script is meant to work WITH your existing C++ RhythmGameplayInput
        // It just controls when input is allowed (turn-based locking)
        // The actual input detection happens in RhythmGameplayInput.cpp

        // If you want pure C# input handling, you would need to access InputManager
        // directly through your engine's C++/C# bridge, but that's not exposed yet.

        // For now, this script just manages the turn state and combo checking.
        // The C++ code (RhythmGameplayInput.cpp) should check TurnManager.IsInputLocked
        // before processing inputs.
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        Console.WriteLine("[PlayerInputHandler] Initialized");
        Console.WriteLine("[PlayerInputHandler] NOTE: Input is handled by C++ RhythmGameplayInput.cpp");
        Console.WriteLine("[PlayerInputHandler] This script manages turn-based locking only");
        initialized = true;
    }

    // =========================
    // Public API (for C++ or other scripts to call)
    // =========================
    public void RegisterKey(char key)
    {
        // This can be called by C++ RhythmGameplayInput when a key is pressed
        Console.WriteLine($"[PlayerInput] Key pressed: {key}");

        if (ComboSystemRef != null)
        {
            ComboSystemRef.RegisterInput(key);

            // Check if combo is complete
            if (ComboSystemRef.HasCompletedCombo())
            {
                Console.WriteLine("[PlayerInput] Combo sequence complete!");
            }
        }
        else
        {
            Console.WriteLine("[PlayerInput] ERROR: ComboSystem reference not set!");
        }
    }

    // Allow C++ or other systems to check if input should be locked
    public bool ShouldBlockInput()
    {
        if (TurnManagerRef == null)
            return false;

        return TurnManagerRef.IsInputLocked;
    }
}