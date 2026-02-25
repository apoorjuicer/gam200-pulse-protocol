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

    
    public int DebugAllowedExtraKey = 75; // K
    public bool DebugTreatExtraKeyAsValid = true;

    
    public bool DebugBypassTurnLock = false;
    
    public bool DebugLogEveryFrame = false;

    // =========================
    // State
    // =========================
    private bool initialized = false;

    // C# edge detection: previous-frame held state for each gameplay key.
    //
    
    private bool prevW = false;
    private bool prevA = false;
    private bool prevS = false;
    private bool prevD = false;

    // Same edge-detection for invalid keys so we don't fire the SFX every
    // frame while a wrong key is held.
    private bool prevInvalidKeyDown = false;

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

        // Always grab input ONCE
        InputComponent input = GetInput();

        // If paused, don't process anything
        if (!DebugBypassTurnLock && Application.IsPaused())
            return;

        // TurnManager lock — only accept input during PlayerInput phase
        if (!DebugBypassTurnLock)
        {
            if (TurnManager.Instance != null && TurnManager.Instance.IsInputLocked)
            {
                // Update prev states so edge detection is accurate when lock lifts
                prevW = input.IsKeyPressed(KeyW);
                prevA = input.IsKeyPressed(KeyA);
                prevS = input.IsKeyPressed(KeyS);
                prevD = input.IsKeyPressed(KeyD);
                return;
            }
        }

        // Sample current held state
        bool curW = input.IsKeyPressed(KeyW);
        bool curA = input.IsKeyPressed(KeyA);
        bool curS = input.IsKeyPressed(KeyS);
        bool curD = input.IsKeyPressed(KeyD);

        // Derive "just pressed this frame" via C# edge detection
        bool tapW = curW && !prevW;
        bool tapA = curA && !prevA;
        bool tapS = curS && !prevS;
        bool tapD = curD && !prevD;

        // DEBUG: Log raw input state
        if (DebugLogEveryFrame && (tapW || tapA || tapS || tapD))
            Console.WriteLine($"[InputBridge:DEBUG] Frame={Time.FrameCount} Tap: W={tapW} A={tapA} S={tapS} D={tapD}");

        // Forward tapped keys to combo system
        if (tapW) OnKeyPressed('W');
        if (tapA) OnKeyPressed('A');
        if (tapS) OnKeyPressed('S');
        if (tapD) OnKeyPressed('D');

        // Store current state for next frame's edge detection
        prevW = curW;
        prevA = curA;
        prevS = curS;
        prevD = curD;

        // Invalid keys (edge-detected to avoid SFX spamming while held)
        CheckInvalidKeys(input);
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        Console.WriteLine("[InputBridge] ===== INITIALIZED =====");
        Console.WriteLine("[InputBridge] Using C# edge detection (IsKeyPressed + prev state)");
        Console.WriteLine("[InputBridge] TurnManager lock is ACTIVE");

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

        Console.WriteLine($"[InputBridge] Key {key} pressed! (Phase={phase}, Beat={beat}, ComboProgress={progress}/{expectedLen})");

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
                if (ComboSystem.Instance.PlayerDidSuccessfulCombo)
                {
                    Console.WriteLine("[InputBridge] SUCCESS! Applying damage immediately.");
                    EnemyRhythmController.Instance?.TakeDamage(10f);
                    
                    // I forgot to add the attack anim call erm 
                    PlayerAnimationController.Instance?.PlayAttack();
                }
                else
                {
                    Console.WriteLine("[InputBridge] FAILED!");
                }
            }
        }
        else
        {
            Console.WriteLine("[InputBridge] WARNING: ComboSystem.Instance is null!");
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

        bool anyInvalidDown = false;
        foreach (int keyCode in invalidKeys)
        {
            if (DebugTreatExtraKeyAsValid && keyCode == DebugAllowedExtraKey)
                continue;

            if (input.IsKeyPressed(keyCode))
            {
                anyInvalidDown = true;
                break;
            }
        }

        // Only fire SFX on the frame the invalid key is first pressed (edge detection)
        if (anyInvalidDown && !prevInvalidKeyDown)
        {
            AudioComponent audio = GetAudio();
            audio.PlaySFX("SFX_Invalid_Key");
            Console.WriteLine("[InputBridge] Invalid key pressed");
        }

        prevInvalidKeyDown = anyInvalidDown;
    }
}
