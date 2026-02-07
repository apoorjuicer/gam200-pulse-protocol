using System;
using ScriptAPI;

/// <summary>
/// TEMPORARY WORKAROUND: Manually wire all script references
/// This is needed because EngineInterface.cpp doesn't expose script discovery
/// 
/// SETUP INSTRUCTIONS:
/// 1. Create a "GameManager" entity in your scene
/// 2. Attach THIS script to it
/// 3. Attach ALL other scripts (Conductor, TurnManager, ComboSystem, etc.) to the SAME entity
/// 4. This script will find them and link them together
/// </summary>
public class GameManager : Script
{
    // =========================
    // PUBLIC: Set these in editor/prefab
    // =========================
    public int EnemyEntityID = 0;  // Set to your enemy entity's ID
    public int UIEntityID = 0;     // Set to your UI entity's ID

    // =========================
    // INTERNAL: Script instances (all on same entity as this)
    // =========================
    private Conductor conductor;
    private TurnManager turnManager;
    private ComboSystem comboSystem;
    private PlayerInputHandler playerInput;
    private EnemyRhythmController enemy;
    private ComboDisplayUI ui;

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

        // Check for manual input (since C++ might not be calling us)
        CheckManualInput();
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        Console.WriteLine("[GameManager] ========================================");
        Console.WriteLine("[GameManager] INITIALIZING GAME SYSTEMS");
        Console.WriteLine("[GameManager] ========================================");

        // HACK: We assume all scripts are on the SAME entity as GameManager
        // This is the only way to get references without proper script discovery

        // In a real implementation, you'd use:
        // conductor = FindScriptOfType<Conductor>();
        // But your engine doesn't support this yet!

        // For now, you must manually attach all scripts to the same entity
        // and set the static Instance fields

        // Check if Conductor.Instance exists (set in Conductor.Update)
        if (Conductor.Instance != null)
        {
            conductor = Conductor.Instance;
            Console.WriteLine("[GameManager] ? Found Conductor");
        }
        else
        {
            Console.WriteLine("[GameManager] ? Conductor not found! Make sure Conductor script is on same entity and has run at least once");
            return;
        }

        // Since we can't get other script references, print instructions
        Console.WriteLine("[GameManager] ========================================");
        Console.WriteLine("[GameManager] CRITICAL: Script references missing!");
        Console.WriteLine("[GameManager] ");
        Console.WriteLine("[GameManager] Your engine needs to add to EngineInterface.cpp:");
        Console.WriteLine("[GameManager]   public: Script^ FindScriptOfType(Type^ scriptType, int entityID)");
        Console.WriteLine("[GameManager] ");
        Console.WriteLine("[GameManager] Until then, you have 2 options:");
        Console.WriteLine("[GameManager] ");
        Console.WriteLine("[GameManager] OPTION 1: Manual singleton pattern");
        Console.WriteLine("[GameManager]   - Add 'public static X Instance' to each script");
        Console.WriteLine("[GameManager]   - Set Instance in each script's Update()");
        Console.WriteLine("[GameManager]   - Access via TurnManager.Instance, etc.");
        Console.WriteLine("[GameManager] ");
        Console.WriteLine("[GameManager] OPTION 2: Use C++ RhythmGameplayInput");
        Console.WriteLine("[GameManager]   - Modify RhythmGameplayInput.cpp to:");
        Console.WriteLine("[GameManager]     a) Check Application.IsPaused() before input");
        Console.WriteLine("[GameManager]     b) Call C# ComboSystem via bridge");
        Console.WriteLine("[GameManager] ");
        Console.WriteLine("[GameManager] ========================================");

        initialized = true;
    }

    // =========================
    // Manual Input (for testing)
    // =========================
    private void CheckManualInput()
    {
        // This is a TEMPORARY solution to test your system
        // Normally, RhythmGameplayInput.cpp would handle this

        var input = GetInput();

        if (!initialized || conductor == null)
            return;

        // Check if we're in player turn
        // (We can't actually check TurnManager without a reference, so we'll just always allow input for testing)

        if (input.IsKeyTriggered(87))  // W
        {
            Console.WriteLine("[GameManager] W pressed (manual)");
            OnKeyPressed('W');
        }
        if (input.IsKeyTriggered(65))  // A
        {
            Console.WriteLine("[GameManager] A pressed (manual)");
            OnKeyPressed('A');
        }
        if (input.IsKeyTriggered(83))  // S
        {
            Console.WriteLine("[GameManager] S pressed (manual)");
            OnKeyPressed('S');
        }
        if (input.IsKeyTriggered(68))  // D
        {
            Console.WriteLine("[GameManager] D pressed (manual)");
            OnKeyPressed('D');
        }
    }

    private void OnKeyPressed(char key)
    {
        // This should call ComboSystem.RegisterInput(key)
        // But we don't have a reference to ComboSystem!

        Console.WriteLine($"[GameManager] Key {key} detected, but can't forward to ComboSystem (no reference)");

        if (comboSystem != null)
        comboSystem.RegisterInput(key);
    }
}