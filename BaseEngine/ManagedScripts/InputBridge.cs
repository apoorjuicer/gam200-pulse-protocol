using System;
using ScriptAPI;


public class InputBridge : Script
{
    // =========================
    // Config
    // =========================
    public int KeyW = 87;  // GLFW_KEY_W
    public int KeyA = 65;  // GLFW_KEY_A
    public int KeyS = 83;  // GLFW_KEY_S
    public int KeyD = 68;  // GLFW_KEY_D

    // DEBUG: Set to true in editor to bypass TurnManager lock and test raw input
    public bool DebugBypassTurnLock = false;
    // DEBUG: Set to true to log every frame's input state (very spammy, use briefly)
    public bool DebugLogEveryFrame = false;

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

        // Check if input is locked by turn system
        if (!DebugBypassTurnLock)
        {
            if (TurnManager.Instance != null && TurnManager.Instance.IsInputLocked)
            {
                var input = GetInput();
                if (input.IsKeyTriggered(KeyW)) SendInputToSystem('W');
                if (input.IsKeyTriggered(KeyA)) SendInputToSystem('A');
                if (input.IsKeyTriggered(KeyS)) SendInputToSystem('S');
                if (input.IsKeyTriggered(KeyD)) SendInputToSystem('D');
            }

            // If paused, don't process input
            if (Application.IsPaused())
            {
                return;
            }
        }

        // Check for key presses using IsKeyTriggered (engine handles edge detection)
        var input = GetInput();

        // DEBUG: Log raw input state every frame to verify engine is detecting keys
        if (DebugLogEveryFrame)
        {
            bool w = input.IsKeyTriggered(KeyW);
            bool a = input.IsKeyTriggered(KeyA);
            bool s = input.IsKeyTriggered(KeyS);
            bool d = input.IsKeyTriggered(KeyD);
            if (w || a || s || d)
            {
                Console.WriteLine($"[InputBridge:DEBUG] Frame={Time.FrameCount} Triggered: W={w} A={a} S={s} D={d}");
            }
        }

        if (input.IsKeyTriggered(KeyW)) OnKeyPressed('W');
        if (input.IsKeyTriggered(KeyA)) OnKeyPressed('A');
        if (input.IsKeyTriggered(KeyS)) OnKeyPressed('S');
        if (input.IsKeyTriggered(KeyD)) OnKeyPressed('D');

        // Check for invalid keys
        CheckInvalidKeys(input);
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        Console.WriteLine("[InputBridge] ===== INITIALIZED =====");
        Console.WriteLine("[InputBridge] Using IsKeyTriggered for reliable input detection");
        if (DebugBypassTurnLock)
        {
            Console.WriteLine("[InputBridge] ⚠️ DEBUG MODE: TurnManager lock BYPASSED — all input accepted");
        }
        if (DebugLogEveryFrame)
        {
            Console.WriteLine("[InputBridge] ⚠️ DEBUG MODE: Logging raw input every frame");
        }
        initialized = true;
    }

    // =========================
    // Input Handling
    // =========================
    private void OnKeyPressed(char key)
    {
        // Log with full context
        string phase = TurnManager.Instance != null ? TurnManager.Instance.GetCurrentPhase().ToString() : "NO_TM";
        int beat = Conductor.Instance != null ? Conductor.Instance.GetCurrentBeat() : -1;
        int progress = ComboSystem.Instance != null ? ComboSystem.Instance.GetComboProgress() : -1;
        Console.WriteLine($"[InputBridge] 🎹 Key {key} pressed! (Phase={phase}, Beat={beat}, ComboProgress={progress}/{ComboSystem.Instance?.GetExpectedCombo()?.Length})");

        // PLAY KEY PRESS SFX
        AudioComponent audio = GetAudio();
        audio.PlaySFX($"SFX_{key}_Key");

        // Forward to ComboSystem using singleton
        if (ComboSystem.Instance != null)
        {
            ComboSystem.Instance.RegisterInput(key);

            // Check if combo is complete
            if (ComboSystem.Instance.HasCompletedCombo())
            {
                Console.WriteLine("[InputBridge] Combo sequence complete!");

                if (ComboSystem.Instance.PlayerDidSuccessfulCombo)
                {
                    Console.WriteLine("[InputBridge] SUCCESS!");
                }
                else
                {
                    Console.WriteLine("[InputBridge] FAILED!");
                }
            }
        }
        else
        {
            Console.WriteLine("[InputBridge] ⚠️ ERROR: ComboSystem.Instance is null!");
        }
    }

    // Check for invalid key presses (non-WASD keys)
    private void CheckInvalidKeys(InputComponent input)
    {
        // Check common keys that players might accidentally press
        int[] invalidKeys = {
            32,  // Space
            81,  // Q
            69,  // E
            82,  // R
            84,  // T
            89,  // Y
            85,  // U
            73,  // I
            79,  // O
            80,  // P
            70,  // F
            71,  // G
            72,  // H
            74,  // J
            75,  // K
            76,  // L
            90,  // Z
            88,  // X
            67,  // C
            86,  // V
            66,  // B
            78,  // N
            77   // M
        };

        foreach (int keyCode in invalidKeys)
        {
            if (input.IsKeyTriggered(keyCode))
            {
                // Play invalid key sound
                AudioComponent audio = GetAudio();
                audio.PlaySFX("SFX_Invalid_Key");
                Console.WriteLine($"[InputBridge] ⚠️ Invalid key pressed (code: {keyCode})");
                return; // Only play once per frame
            }
        }
    }
}