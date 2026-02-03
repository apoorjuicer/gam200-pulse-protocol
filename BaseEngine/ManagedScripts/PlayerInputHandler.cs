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
    // Input Settings
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

        // Get input component (it's a struct, so no null check needed)
        InputComponent input = GetInput();

        // Check each key
        if (input.IsKeyTriggered(KeyW))
        {
            RegisterKey('W');
        }
        else if (input.IsKeyTriggered(KeyA))
        {
            RegisterKey('A');
        }
        else if (input.IsKeyTriggered(KeyS))
        {
            RegisterKey('S');
        }
        else if (input.IsKeyTriggered(KeyD))
        {
            RegisterKey('D');
        }
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        Console.WriteLine("[PlayerInputHandler] Initialized");
        initialized = true;
    }

    // =========================
    // Input Processing
    // =========================
    private void RegisterKey(char key)
    {
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

    // =========================
    // Helpers
    // =========================
    private InputComponent GetInput()
    {
        // InputComponent is a struct (value type), not a class
        // Your engine provides this through the Script base class or entity
        // Based on your InputAPI.cpp, it should be accessible via the entity ID

        // This should work based on your engine's scripting bridge:
        return new InputComponent(EntityID);
    }
}