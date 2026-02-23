using System;
using ScriptAPI;

public enum TurnPhase
{
    Countdown,      // First 4 beats - enemy idle
    EnemyDisplay,   // Next 4 beats - show combo, player locked
    PlayerInput,    // 4 beats - player inputs combo
    Resolution      // Resolve success/failure
}

public class TurnManager : Script
{
    // =========================
    // Global Access (Singleton)
    // =========================
    public static TurnManager Instance { get; private set; }

    // =========================
    // Config
    // =========================
    public int CountdownBeats = 4;
    public int EnemyDisplayBeats = 4;
    public int PlayerInputBeats = 4;

    // =========================
    // State
    // =========================
    private TurnPhase currentPhase = TurnPhase.Countdown;
    private int phaseStartBeat = 0;
    private int lastBeat = -1;
    private bool initialized = false;

    public bool IsPlayerTurn => currentPhase == TurnPhase.PlayerInput;
    public bool IsInputLocked => currentPhase != TurnPhase.PlayerInput;

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

        if (Conductor.Instance == null)
            return;

        int currentBeat = Conductor.Instance.GetCurrentBeat();

        // Only process on new beats
        if (currentBeat == lastBeat)
            return;

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
                // Move to next enemy turn after resolution
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

        Console.WriteLine("[TurnManager] Initializing...");

        if (Conductor.Instance != null)
        {
            phaseStartBeat = Conductor.Instance.GetCurrentBeat();
            lastBeat = phaseStartBeat;
        }

        currentPhase = TurnPhase.Countdown;
        Console.WriteLine("[TurnManager] Starting countdown phase (4 beats)");

        initialized = true;
    }

    // =========================
    // Phase Transitions
    // =========================
    private void StartEnemyDisplay()
    {
        currentPhase = TurnPhase.EnemyDisplay;
        phaseStartBeat = Conductor.Instance.GetCurrentBeat();

        Console.WriteLine("[TurnManager] === ENEMY TURN ===");
        Console.WriteLine($"[TurnManager] Displaying combo: {EnemyRhythmController.Instance?.WeaknessCombo ?? "UNKNOWN"}");
        Console.WriteLine("[TurnManager] Player input LOCKED");

        // DON'T PAUSE THE GAME!
        // Just set IsInputLocked = true (which is automatic based on currentPhase)
        // The InputBridge checks this flag before processing keys

        // Reset combo system for new attempt
        if (ComboSystem.Instance != null)
        {
            ComboSystem.Instance.ResetCombo();
        }

        // Reset enemy movement for new turn
        if (EnemyRhythmController.Instance != null)
        {
            EnemyRhythmController.Instance.ResetMovementForNewTurn();
        }
    }

    private void StartPlayerInput()
    {
        currentPhase = TurnPhase.PlayerInput;
        phaseStartBeat = Conductor.Instance.GetCurrentBeat();

        Console.WriteLine("[TurnManager] === PLAYER TURN ===");
        Console.WriteLine($"[TurnManager] Input the combo: {EnemyRhythmController.Instance?.WeaknessCombo ?? "UNKNOWN"}");
        Console.WriteLine("[TurnManager] Player input UNLOCKED");

        // Enemy freezes during player input (handled in EnemyRhythmController.HandleBeat)
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
            Console.WriteLine("[TurnManager] ? Player succeeded!");
            Console.WriteLine("[TurnManager] Enemy takes damage!");

            // // PLAY SUCCESS SOUND
            // AudioComponent audio = GetAudio();
            // audio.PlaySFX("SFX_ComboEstablished_1");  // Combo complete sound!



            // Enemy takes damage and might die
            if (EnemyRhythmController.Instance != null)
            {
                EnemyRhythmController.Instance.TakeDamage(10f);
            }
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

        return Conductor.Instance.GetCurrentBeat() - phaseStartBeat;
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