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
    // References
    // =========================
    public Conductor ConductorRef = null;
    public ComboSystem ComboSystemRef = null;
    public EnemyRhythmController EnemyRef = null;

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

        if (ConductorRef == null)
            return;

        int currentBeat = ConductorRef.GetCurrentBeat();

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
                }
                break;

            case TurnPhase.EnemyDisplay:
                if (beatsInPhase >= EnemyDisplayBeats)
                {
                    StartPlayerInput();
                }
                else
                {
                    // Enemy moves during display
                    if (EnemyRef != null)
                    {
                        // Enemy movement is handled in EnemyRhythmController
                        Console.WriteLine($"[TurnManager] Enemy Display Beat {beatsInPhase + 1}/{EnemyDisplayBeats}");
                    }
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
                break;
        }
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        Console.WriteLine("[TurnManager] Initializing...");

        if (ConductorRef != null)
        {
            phaseStartBeat = ConductorRef.GetCurrentBeat();
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
        phaseStartBeat = ConductorRef.GetCurrentBeat();

        Console.WriteLine("[TurnManager] === ENEMY TURN ===");
        Console.WriteLine($"[TurnManager] Displaying combo: {EnemyRef?.WeaknessCombo ?? "UNKNOWN"}");
        Console.WriteLine("[TurnManager] Player input LOCKED");

        // Reset combo system for new attempt
        if (ComboSystemRef != null)
        {
            ComboSystemRef.ResetCombo();
        }

        // UI should display the enemy combo here
        // You'll need to trigger your UI system to show the combo
    }

    private void StartPlayerInput()
    {
        currentPhase = TurnPhase.PlayerInput;
        phaseStartBeat = ConductorRef.GetCurrentBeat();

        Console.WriteLine("[TurnManager] === PLAYER TURN ===");
        Console.WriteLine($"[TurnManager] Input the combo: {EnemyRef?.WeaknessCombo ?? "UNKNOWN"}");
        Console.WriteLine("[TurnManager] Player input UNLOCKED");

        // Enemy freezes during player input
        if (EnemyRef != null)
        {
            // Enemy's canMove flag should be set to false
            // This is handled in EnemyRhythmController
        }
    }

    private void ResolvePlayerInput()
    {
        currentPhase = TurnPhase.Resolution;

        bool success = false;
        if (ComboSystemRef != null)
        {
            success = ComboSystemRef.PlayerDidSuccessfulCombo;
        }

        Console.WriteLine($"[TurnManager] === RESOLUTION ===");

        if (success)
        {
            Console.WriteLine("[TurnManager] ? Player succeeded!");
            Console.WriteLine("[TurnManager] Enemy takes damage!");

            // Enemy takes damage and might die
            if (EnemyRef != null)
            {
                EnemyRef.TakeDamage(10f); // Adjust damage as needed
            }
        }
        else
        {
            Console.WriteLine("[TurnManager] ? Player failed!");
            Console.WriteLine("[TurnManager] Enemy attacks and moves remaining steps!");

            // Enemy attacks player
            if (EnemyRef != null)
            {
                // Enemy should attack the player here
                // Then move remaining steps
                Console.WriteLine("[TurnManager] Player takes damage!");
            }
        }

        // Reset for next turn
        if (ComboSystemRef != null)
        {
            ComboSystemRef.ResetCombo();
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
        if (ConductorRef == null)
            return 0;

        return ConductorRef.GetCurrentBeat() - phaseStartBeat;
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
}