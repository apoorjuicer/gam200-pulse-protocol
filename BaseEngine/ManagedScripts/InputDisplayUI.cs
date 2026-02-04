using System;
using ScriptAPI;

/// <summary>
/// Displays player key inputs on screen in real-time.
/// Limited to printing 5 times per player turn to reduce console spam.
/// </summary>
public class InputDisplayUI : Script
{
    // =========================
    // Global Access (Singleton)
    // =========================
    public static InputDisplayUI Instance { get; private set; }

    // =========================
    // Config
    // =========================
    public float DisplayX = 100f;
    public float DisplayY = 100f;
    public float LineSpacing = 40f;
    public int MaxPrintsPerTurn = 5;  // Limit console prints

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private string currentInputDisplay = "";
    private string timingFeedbackDisplay = "";
    private string expectedComboDisplay = "";
    private float feedbackTimer = 0f;
    private const float feedbackDuration = 1.0f;

    // NEW: Track prints per turn
    private int printsThisTurn = 0;
    private TurnPhase lastPhase = TurnPhase.Countdown;
    private bool hasPrintedFinalResult = false;

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

        // Update feedback timer
        if (feedbackTimer > 0f)
        {
            feedbackTimer -= Time.DeltaTime;
            if (feedbackTimer <= 0f)
            {
                timingFeedbackDisplay = "";
            }
        }

        // Check for phase changes to reset print counter
        if (TurnManager.Instance != null)
        {
            TurnPhase currentPhase = TurnManager.Instance.GetCurrentPhase();

            if (currentPhase != lastPhase)
            {
                // Phase changed - reset counter
                if (currentPhase == TurnPhase.PlayerInput)
                {
                    printsThisTurn = 0;
                    hasPrintedFinalResult = false;
                    Console.WriteLine("\n[InputDisplayUI] === NEW PLAYER TURN - Ready for input ===\n");
                }

                lastPhase = currentPhase;
            }
        }

        // Update display based on ComboSystem state
        UpdateDisplay();
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        if (Instance == null)
            Instance = this;

        Console.WriteLine("[InputDisplayUI] Initialized (Max {0} prints per turn)", MaxPrintsPerTurn);
        initialized = true;
    }

    // =========================
    // Display Update
    // =========================
    private void UpdateDisplay()
    {
        if (ComboSystem.Instance == null)
        {
            currentInputDisplay = "ComboSystem not ready...";
            return;
        }

        // Get current combo progress
        string currentCombo = ComboSystem.Instance.GetCurrentComboString();
        int progress = ComboSystem.Instance.GetComboProgress();
        int maxLength = ComboSystem.Instance.maxComboLength;

        // Build display string with visual indicators
        string[] keys = new string[maxLength];
        for (int i = 0; i < maxLength; i++)
        {
            if (i < progress)
            {
                keys[i] = currentCombo[i].ToString();
            }
            else
            {
                keys[i] = "_";
            }
        }

        currentInputDisplay = string.Join(" ", keys);

        // Get expected combo
        string expected = ComboSystem.Instance.GetExpectedCombo();
        if (!string.IsNullOrEmpty(expected))
        {
            string[] expectedKeys = new string[expected.Length];
            for (int i = 0; i < expected.Length; i++)
            {
                expectedKeys[i] = expected[i].ToString();
            }
            expectedComboDisplay = $"Expected: {string.Join(" ", expectedKeys)}";
        }

        // Get timing feedback for each key
        var hitResults = ComboSystem.Instance.GetHitResults();
        if (hitResults.Count > 0)
        {
            string[] timings = new string[hitResults.Count];
            for (int i = 0; i < hitResults.Count; i++)
            {
                string indicator = hitResults[i] == "PERFECT" ? "⭐" :
                                  (hitResults[i] == "GOOD" ? "✓" : "❌");
                timings[i] = $"{indicator}{hitResults[i]}";
            }
            timingFeedbackDisplay = string.Join(" | ", timings);
        }
    }

    // =========================
    // Public API - Called by InputBridge
    // =========================
    public void ShowKeyPress(char key, string timing)
    {
        feedbackTimer = feedbackDuration;

        // Only print if we haven't exceeded limit
        if (printsThisTurn < MaxPrintsPerTurn)
        {
            Console.WriteLine($"[InputDisplayUI] 🎮 Key {key} pressed - {timing}");
            PrintDisplay();
            printsThisTurn++;
        }
        else if (printsThisTurn == MaxPrintsPerTurn)
        {
            // Print one final message that we're suppressing further output
            Console.WriteLine($"[InputDisplayUI] ... (suppressing further output this turn to reduce spam)");
            printsThisTurn++; // Increment so this message only prints once
        }
    }

    public void ShowComboResult(bool success)
    {
        // Always print the final result, regardless of print limit
        if (!hasPrintedFinalResult)
        {
            Console.WriteLine("\n"); // Add spacing

            /*if (success)
            {
                Console.WriteLine("╔════════════════════════════════════════╗");
                Console.WriteLine("║          ✅ COMBO SUCCESS!             ║");
                Console.WriteLine("╠════════════════════════════════════════╣");
                Console.WriteLine($"║ Sequence: {currentInputDisplay,-28}║");
                Console.WriteLine($"║ Timing:   {timingFeedbackDisplay,-28}║");
                Console.WriteLine("╚════════════════════════════════════════╝");
            }
            else
            {
                Console.WriteLine("╔════════════════════════════════════════╗");
                Console.WriteLine("║          ❌ COMBO FAILED!              ║");
                Console.WriteLine("╠════════════════════════════════════════╣");
                Console.WriteLine($"║ Sequence: {currentInputDisplay,-28}║");
                Console.WriteLine($"║ Timing:   {timingFeedbackDisplay,-28}║");
                Console.WriteLine("╚════════════════════════════════════════╝");
            }*/

            Console.WriteLine("\n"); // Add spacing
            hasPrintedFinalResult = true;
        }
    }

    // =========================
    // Display Rendering
    // =========================
    /*private void PrintDisplay()
    {
        // Only print if in player turn phase
        if (TurnManager.Instance == null)
            return;

        TurnPhase phase = TurnManager.Instance.GetCurrentPhase();

        /*if (phase == TurnPhase.PlayerInput)
        {
            Console.WriteLine("╔════════════════════════════════════════╗");
            Console.WriteLine("║         KEY INPUT DISPLAY              ║");
            Console.WriteLine("╠════════════════════════════════════════╣");
            Console.WriteLine($"║ Current:  {currentInputDisplay,-30}║");
            Console.WriteLine($"║ {expectedComboDisplay,-40}║");
            Console.WriteLine($"║ Timing:   {timingFeedbackDisplay,-30}║");
            Console.WriteLine("╚════════════════════════════════════════╝");
        }
    }*/
}