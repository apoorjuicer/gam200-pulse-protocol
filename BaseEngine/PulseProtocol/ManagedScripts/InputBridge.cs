using System;
using ScriptAPI;

public class InputBridge : Script
{
    // =========================
    // Config
    // =========================
    public int KeyW = 87;
    public int KeyA = 65;
    public int KeyS = 83;
    public int KeyD = 68;

    public int DebugAllowedExtraKey = 75;
    public bool DebugTreatExtraKeyAsValid = true;
    public bool DebugBypassTurnLock = false;
    public bool DebugLogEveryFrame = false;

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private bool prevW = false;
    private bool prevA = false;
    private bool prevS = false;
    private bool prevD = false;
    private bool prevInvalidKeyDown = false;

    // =========================
    // Update Loop
    // =========================
    public override void Update()
    {
        if (!initialized) { Initialize(); return; }

        InputComponent input = GetInput();

        if (!DebugBypassTurnLock && Application.IsPaused()) return;

        if (!DebugBypassTurnLock)
        {
            if (TurnManager.Instance == null || TurnManager.Instance.IsInputLocked)
            {
                prevW = input.IsKeyPressed(KeyW);
                prevA = input.IsKeyPressed(KeyA);
                prevS = input.IsKeyPressed(KeyS);
                prevD = input.IsKeyPressed(KeyD);
                return;
            }
        }

        bool curW = input.IsKeyPressed(KeyW);
        bool curA = input.IsKeyPressed(KeyA);
        bool curS = input.IsKeyPressed(KeyS);
        bool curD = input.IsKeyPressed(KeyD);

        bool tapW = curW && !prevW;
        bool tapA = curA && !prevA;
        bool tapS = curS && !prevS;
        bool tapD = curD && !prevD;

        if (DebugLogEveryFrame && (tapW || tapA || tapS || tapD))
            Console.WriteLine($"[InputBridge:DEBUG] Frame={Time.FrameCount} Tap: W={tapW} A={tapA} S={tapS} D={tapD}");

        if (tapW) OnKeyPressed('W');
        if (tapA) OnKeyPressed('A');
        if (tapS) OnKeyPressed('S');
        if (tapD) OnKeyPressed('D');

        prevW = curW;
        prevA = curA;
        prevS = curS;
        prevD = curD;

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
            Console.WriteLine("[InputBridge] ⚠️ DEBUG MODE: TurnManager lock BYPASSED");
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
        int expectedLen = ComboSystem.Instance?.GetExpectedCombo()?.Length ?? -1;

        // Console.WriteLine($"[InputBridge] Key {key} pressed! (Phase={phase}, Beat={beat}, ComboProgress={progress}/{expectedLen})");

        if (ComboSystem.Instance != null)
        {
            // Check if key is correct BEFORE registering
            string expected = ComboSystem.Instance.GetExpectedCombo();
            int currentProgress = ComboSystem.Instance.GetComboProgress();
            bool isCorrect = currentProgress < expected.Length &&
                             key == expected[currentProgress];

            // Play correct or wrong SFX depending on the key pressed
            AudioComponent audio = GetAudio();
            audio.PlaySFX(isCorrect ? "SFX_W_Key" : "SFX_Invalid_Key"); // the SFX_W_Key is just placeholder for now; will replace once have a SFX for correct key input

            // Forward to ComboSystem
            ComboSystem.Instance.RegisterInput(key);

            if (ComboSystem.Instance.HasCompletedCombo())
            {
                // Console.WriteLine("[InputBridge] Combo sequence complete!");
                if (ComboSystem.Instance.PlayerDidSuccessfulCombo)
                {
                    // Console.WriteLine("[InputBridge] SUCCESS! Applying damage immediately.");
                    EnemyRhythmController.Instance?.TakeDamage(10f);
                    PlayerAnimationController.Instance?.PlayAttack();
                }
                // else
                // {
                //     Console.WriteLine("[InputBridge] FAILED!");
                // }
            }
        }
        // else
        // {
        //     Console.WriteLine("[InputBridge] WARNING: ComboSystem.Instance is null!");
        // }
    }

    // =========================
    // Invalid Key Handling
    // =========================
    private void CheckInvalidKeys(InputComponent input)
    {
        int[] invalidKeys =
        {
            81, 69, 82, 84, 89, 85, 73, 79, 80,
            70, 71, 72, 74, 75, 76, 90, 88, 67, 86,
            66, 78, 77
        };

        bool anyInvalidDown = false;
        foreach (int keyCode in invalidKeys)
        {
            if (DebugTreatExtraKeyAsValid && keyCode == DebugAllowedExtraKey) continue;
            if (input.IsKeyPressed(keyCode)) { anyInvalidDown = true; break; }
        }

        if (anyInvalidDown && !prevInvalidKeyDown)
        {
            AudioComponent audio = GetAudio();
            audio.PlaySFX("SFX_Invalid_Key");
            Console.WriteLine("[InputBridge] Invalid key pressed");
        }

        prevInvalidKeyDown = anyInvalidDown;
    }
}