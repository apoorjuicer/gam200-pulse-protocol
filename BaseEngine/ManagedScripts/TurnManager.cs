using System;
using ScriptAPI;

public enum TurnPhase
{
    Countdown,      // First 4 beats - enemy idle
    EnemyDisplay,   // Next 4 beats - show combo, player locked
    PlayerInput,    // 5 effective beats - player inputs combo
    Resolution      // Resolve success/failure
}

public class TurnManager : Script
{
    // =========================
    // Global Access (Singleton)
    // =========================
    public static TurnManager? Instance { get; private set; }

    // =========================
    // Config
    // =========================
    public int CountdownBeats = 4;
    public int EnemyDisplayBeats = 4;
    public int PlayerInputBeats = 6;  // 6 because StartPlayerInput() back-dates phaseStartBeat by -1,
                                      // so beatsInPhase starts at 1 instead of 0.
                                      // Effective beat slots = PlayerInputBeats - 1 = 5.

    // =========================
    // State
    // =========================
    private TurnPhase currentPhase = TurnPhase.Countdown;
    private int phaseStartBeat = -1;
    private int lastBeat = -1;
    private bool initialized = false;
    private bool gameStarted = false;
    private bool prevGameReady = false; // used to detect new play sessions on reused script objects

    public bool IsPlayerTurn => currentPhase == TurnPhase.PlayerInput;
    public bool IsInputLocked => currentPhase != TurnPhase.PlayerInput;

    // =========================
    // Update Loop
    // =========================
    public override void Update()
    {
        // Session restart detection for reused script objects (engine does not always
        // recreate C# Script instances on Stop→Play). GameStart.Ready flips from true
        // (end of last session) to false (LevelCountdown resets it at start of new session).
        // That flip means a new play session has begun and we must re-initialize.
        if (initialized && prevGameReady && !GameStart.Ready)
        {
            Console.WriteLine("[TurnManager] New play session detected — resetting state");
            initialized = false;
        }

        prevGameReady = GameStart.Ready;

        if (!initialized)
        {
            Initialize();
            return;
        }

        if (Conductor.Instance == null)
            return;

        int currentBeat = Conductor.Instance!.GetCurrentBeat();

        // Only process on new beats
        if (currentBeat == lastBeat)
            return;

        if (!GameStart.Ready) return;

        // Anchor the phase start to the first beat the game actually becomes ready,
        // so beatsInPhase doesn't inherit stale counts from before GameStart.
        if (!gameStarted)
        {
            gameStarted = true;
            lastBeat = currentBeat;
            StartEnemyDisplay();
            return;
        }

        lastBeat = currentBeat;
        int beatsInPhase = currentBeat - phaseStartBeat;

        // Phase transitions
        switch (currentPhase)
        {
            case TurnPhase.Countdown:
                if (beatsInPhase >= CountdownBeats)
                {
                    StartEnemyDisplay();
                    beatsInPhase = currentBeat - phaseStartBeat;
                    goto case TurnPhase.EnemyDisplay;
                }
                break;

            case TurnPhase.EnemyDisplay:
                if (beatsInPhase >= EnemyDisplayBeats)
                {
                    StartPlayerInput();
                    beatsInPhase = currentBeat - phaseStartBeat;
                    goto case TurnPhase.PlayerInput;
                }
                else
                {
                    Console.WriteLine($"[TurnManager] Enemy Display Beat {beatsInPhase + 1}/{EnemyDisplayBeats}");
                }
                break;

            case TurnPhase.PlayerInput:
                if (beatsInPhase >= PlayerInputBeats)
                {
                    ResolvePlayerInput();
                }
                else
                {
                    Console.WriteLine($"[TurnManager] Player Input Beat {beatsInPhase + 1}/{PlayerInputBeats}");
                }
                break;

            case TurnPhase.Resolution:
                // Wait for the death animation to finish before advancing.
                // IsDying stays true for 2.5s after Die() fires; only becomes null
                // once the animation completes and the spawner is notified.
                if (EnemyRhythmController.Instance == null || EnemyRhythmController.Instance!.IsDying)
                {
                    Console.WriteLine("[TurnManager] Resolution: waiting for death animation / next enemy...");
                    break;
                }
                StartEnemyDisplay();
                beatsInPhase = currentBeat - phaseStartBeat;
                goto case TurnPhase.EnemyDisplay;
        }
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        // Set singleton (handle scene reloads)
        if (Instance != null && Instance != this)
        {
            Console.WriteLine("[TurnManager] Replacing stale singleton instance");
        }
        Instance = this;

        // Full state reset — GameStart.Ready is a static bool that persists between
        // play sessions. If it's stale-true, TurnManager would skip the countdown.
        // Resetting it here forces LevelCountdown to re-run its 3-2-1-GO sequence.
        GameStart.Ready = false;
        currentPhase = TurnPhase.Countdown;
        phaseStartBeat = -1;
        lastBeat = -1;
        gameStarted = false;

        // Clear ComboSystem state so lastInputBeat (from a previous session) can't
        // accidentally match a beat in the new session and block the first key press.
        ComboSystem.Instance?.ResetCombo();

        Console.WriteLine("[TurnManager] Initializing...");

        if (Conductor.Instance != null)
        {
            phaseStartBeat = Conductor.Instance!.GetCurrentBeat();
            lastBeat = phaseStartBeat;
        }

        Console.WriteLine("[TurnManager] Starting countdown phase (4 beats)");

        initialized = true;
    }

    // =========================
    // Phase Transitions
    // =========================
    private void StartEnemyDisplay()
    {
        currentPhase = TurnPhase.EnemyDisplay;
        phaseStartBeat = Conductor.Instance!.GetCurrentBeat();

        Console.WriteLine("[TurnManager] === ENEMY TURN ===");
        Console.WriteLine($"[TurnManager] Displaying combo: {EnemyRhythmController.Instance?.WeaknessCombo ?? "UNKNOWN"}");
        Console.WriteLine("[TurnManager] Player input LOCKED");

       
        if (ComboSystem.Instance != null)
        {
            ComboSystem.Instance.ResetCombo();
        }

        // Reset enemy movement for new turn
        if (EnemyRhythmController.Instance != null)
        {
            EnemyRhythmController.Instance.ResetMovementForNewTurn();
        }

        // BEAT REVEAL!
        if (ComboDisplayManager.Instance != null && EnemyRhythmController.Instance != null)
            ComboDisplayManager.Instance.StartComboReveal(EnemyRhythmController.Instance.WeaknessCombo);
    }

    private void StartPlayerInput()
    {
        currentPhase = TurnPhase.PlayerInput;
        // Subtract 1 so the transition beat counts as beat 1 of the input window.
        // This guarantees the player can press on the same beat the phase opens,
        // regardless of whether InputBridge or TurnManager updates first.
        phaseStartBeat = Conductor.Instance!.GetCurrentBeat() - 1;

        Console.WriteLine("[TurnManager] === PLAYER TURN ===");
        Console.WriteLine($"[TurnManager] Input the combo: {EnemyRhythmController.Instance?.WeaknessCombo ?? "UNKNOWN"}");
        Console.WriteLine("[TurnManager] Player input UNLOCKED");

        // Enemy freezes during player input (handled in EnemyRhythmController.HandleBeat)

        // this will trigger the fade out, and then clear for player input phase
        if (ComboDisplayManager.Instance != null)
            ComboDisplayManager.Instance.StopComboReveal();
    }

    private void ResolvePlayerInput()
    {
        currentPhase = TurnPhase.Resolution;

        bool success = false;
        if (ComboSystem.Instance != null)
        {
            success = ComboSystem.Instance.PlayerDidSuccessfulCombo;
        }

        Console.WriteLine($"[TurnManager] === RESOLUTION ===");

        if (success)
        {
            // Damage was already applied immediately in InputBridge.OnKeyPressed()
            // when the combo was completed — no need to apply it again here.
            Console.WriteLine("[TurnManager] Player succeeded! (damage applied at combo completion)");

            // // PLAY SUCCESS SOUND
            // AudioComponent audio = GetAudio();
            // audio.PlaySFX("SFX_ComboEstablished_1");
        }
        else
        {
            Console.WriteLine("[TurnManager] ? Player failed!");
            Console.WriteLine("[TurnManager] Enemy attacks and moves remaining steps!");

            // // PLAY FAILURE SOUND
            // AudioComponent audio = GetAudio();
            // audio.PlaySFX("SFX_ComboStreakBroken");  // Failure sound!


            // Enemy executes punishment
            if (EnemyRhythmController.Instance != null)
            {
                EnemyRhythmController.Instance.ExecuteFailurePunishment();
            }
        }

        // Reset for next turn
        if (ComboSystem.Instance != null)
        {
            ComboSystem.Instance.ResetCombo();
        }
    }

    // =========================
    // Public API
    // =========================
    public TurnPhase GetCurrentPhase()
    {
        return currentPhase;
    }

    public int GetBeatsInCurrentPhase()
    {
        if (Conductor.Instance == null)
            return 0;

        return Conductor.Instance!.GetCurrentBeat() - phaseStartBeat;
    }

    public int GetBeatsRemainingInPhase()
    {
        int beatsInPhase = GetBeatsInCurrentPhase();

        switch (currentPhase)
        {
            case TurnPhase.Countdown:
                return CountdownBeats - beatsInPhase;
            case TurnPhase.EnemyDisplay:
                return EnemyDisplayBeats - beatsInPhase;
            case TurnPhase.PlayerInput:
                return PlayerInputBeats - beatsInPhase;
            default:
                return 0;
        }
    }

    public void OnDestroy()
    {
        if (Instance == this)
            Instance = null;
    }
}