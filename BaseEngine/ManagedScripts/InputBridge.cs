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

    // Optional: if you’re using a kill-cheat on K, this prevents “invalid key” SFX
    public int DebugAllowedExtraKey = 75; // K
    public bool DebugTreatExtraKeyAsValid = true;

    // DEBUG: Set true to bypass TurnManager lock and test raw input
    public bool DebugBypassTurnLock = false;
    // DEBUG: Set true to log triggered input (spammy, use briefly)
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

        // Always grab input ONCE (fixes CS0136)
        InputComponent input = GetInput();

        // If paused, don't process anything (unless you explicitly want debug while paused)
        if (!DebugBypassTurnLock && Application.IsPaused())
            return;

        // If input is locked by turn system, do NOT forward inputs
        if (!DebugBypassTurnLock)
        {
            if (TurnManager.Instance != null && TurnManager.Instance.IsInputLocked)
            {
                // Optional: still log what the player is pressing while locked
                if (DebugLogEveryFrame)
                {
                    bool wL = input.IsKeyTriggered(KeyW);
                    bool aL = input.IsKeyTriggered(KeyA);
                    bool sL = input.IsKeyTriggered(KeyS);
                    bool dL = input.IsKeyTriggered(KeyD);
                    if (wL || aL || sL || dL)
                        Console.WriteLine($"[InputBridge:DEBUG] INPUT LOCKED | Frame={Time.FrameCount} Triggered: W={wL} A={aL} S={sL} D={dL}");
                }
                return;
            }
        }

        // DEBUG: Log raw input state
        if (DebugLogEveryFrame)
        {
            bool w = input.IsKeyTriggered(KeyW);
            bool a = input.IsKeyTriggered(KeyA);
            bool s = input.IsKeyTriggered(KeyS);
            bool d = input.IsKeyTriggered(KeyD);
            if (w || a || s || d)
                Console.WriteLine($"[InputBridge:DEBUG] Frame={Time.FrameCount} Triggered: W={w} A={a} S={s} D={d}");
        }

        // Forward valid keys
        if (input.IsKeyTriggered(KeyW)) OnKeyPressed('W');
        if (input.IsKeyTriggered(KeyA)) OnKeyPressed('A');
        if (input.IsKeyTriggered(KeyS)) OnKeyPressed('S');
        if (input.IsKeyTriggered(KeyD)) OnKeyPressed('D');

        // Invalid keys
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
            Console.WriteLine("[InputBridge] ⚠️ DEBUG MODE: TurnManager lock BYPASSED — all input accepted");

        if (DebugLogEveryFrame)
            Console.WriteLine("[InputBridge] ⚠️ DEBUG MODE: Logging raw input every frame");

        initialized = true;
    }

    // =========================
    // Input Handling
    // =========================
    private void OnKeyPressed(char key)
    {
        string phase = TurnManager.Instance != null ? TurnManager.Instance.GetCurrentPhase().ToString() : "NO_TM";
        int beat = Conductor.Instance != null ? Conductor.Instance.GetCurrentBeat() : -1;
        int progress = ComboSystem.Instance != null ? ComboSystem.Instance.GetComboProgress() : -1;

        int expectedLen = -1;
        if (ComboSystem.Instance != null && ComboSystem.Instance.GetExpectedCombo() != null)
            expectedLen = ComboSystem.Instance.GetExpectedCombo().Length;

        Console.WriteLine($"[InputBridge] 🎹 Key {key} pressed! (Phase={phase}, Beat={beat}, ComboProgress={progress}/{expectedLen})");

        // Play key press SFX
        AudioComponent audio = GetAudio();
        audio.PlaySFX($"SFX_{key}_Key");

        // Forward to ComboSystem
        if (ComboSystem.Instance != null)
        {
            ComboSystem.Instance.RegisterInput(key);

            if (ComboSystem.Instance.HasCompletedCombo())
            {
                Console.WriteLine("[InputBridge] Combo sequence complete!");
                Console.WriteLine(ComboSystem.Instance.PlayerDidSuccessfulCombo ? "[InputBridge] SUCCESS!" : "[InputBridge] FAILED!");
            }
        }
        else
        {
            Console.WriteLine("[InputBridge] ⚠️ ERROR: ComboSystem.Instance is null!");
        }
    }

    // =========================
    // Invalid Key Handling
    // =========================
    private void CheckInvalidKeys(InputComponent input)
    {
        int[] invalidKeys =
        {
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
            if (DebugTreatExtraKeyAsValid && keyCode == DebugAllowedExtraKey)
                continue;

            if (input.IsKeyTriggered(keyCode))
            {
                AudioComponent audio = GetAudio();
                audio.PlaySFX("SFX_Invalid_Key");
                Console.WriteLine($"[InputBridge] ⚠️ Invalid key pressed (code: {keyCode})");
                return;
            }
        }
    }
}