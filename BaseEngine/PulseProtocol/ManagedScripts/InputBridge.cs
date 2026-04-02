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

    // PS4 gamepad button constants (GLFW_GAMEPAD_BUTTON_*)
    // Triangle(Y=3)=W, Circle(X=2)=A, Cross(A=0)=S, Square(B=1)=D
    private const int GP_TRIANGLE = 3; // W
    private const int GP_CIRCLE = 2; // A
    private const int GP_SQUARE = 1; // D
    private const int GP_CROSS  = 0; // S


    // =========================
    // State
    // =========================
    private bool initialized = false;
    private bool prevW = false;
    private bool prevA = false;
    private bool prevS = false;
    private bool prevD = false;
    private bool prevInvalidKeyDown = false;
    private bool breakSfxPlayed = false; // ensures break SFX only fires once per turn

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

        // Keyboard OR gamepad face buttons (additive — either input works)
        bool curW = input.IsKeyPressed(KeyW) || input.IsGamepadButtonPressed(GP_TRIANGLE);
        bool curA = input.IsKeyPressed(KeyA) || input.IsGamepadButtonPressed(GP_SQUARE);
        bool curS = input.IsKeyPressed(KeyS) || input.IsGamepadButtonPressed(GP_CROSS);
        bool curD = input.IsKeyPressed(KeyD) || input.IsGamepadButtonPressed(GP_CIRCLE);

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
        if (ComboSystem.Instance == null) return;

        AudioComponent audio = GetAudio();

        // If the combo is not yet completed, this is a fresh turn — reset the break flag.
        // More reliable than tracking the lock→unlock transition, because it doesn't depend
        // on entity update order.
        if (!ComboSystem.Instance.HasCompletedCombo())
            breakSfxPlayed = false;

        // Keys 1-3: play click instantly. Last key: play establish/break instead (no click).
        bool isLastKey = !ComboSystem.Instance.HasCompletedCombo() &&
                         ComboSystem.Instance.GetComboProgress() + 1 >= ComboSystem.Instance.GetExpectedCombo().Length;
        if (!isLastKey)
            audio.PlaySFX("SFX_W_Key");

        ComboSystem.Instance.RegisterInput(key);

        if (ComboSystem.Instance.HasCompletedCombo())
        {
            if (ComboSystem.Instance.PlayerDidSuccessfulCombo)
            {
                audio.PlaySFX("SFX_ComboEstablished_1");
                float playerDamage = PlayerHealth.Instance != null ? PlayerHealth.Instance.GetHealth().damage : 10;
                EnemyRhythmController.Instance?.TakeDamage(playerDamage);
                PlayerAnimationController.Instance?.PlayAttack(ComboSystem.Instance.GetExpectedCombo());
            }
            else if (!breakSfxPlayed)
            {
                audio.PlaySFX("SFX_ComboStreakBroken");
                breakSfxPlayed = true;
            }
        }
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