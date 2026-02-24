using System;
using System.Collections.Generic;
using ScriptAPI;

public class ComboSystem : Script
{
    // =========================
    // Global Access (Singleton)
    // =========================
    public static ComboSystem Instance { get; private set; }

    // =========================
    // Config
    // =========================
    public int maxComboLength = 4;

    // Timing windows (seconds)
    // At 120 BPM (0.5s/beat), max delta is 0.25s. Player data shows most presses land 0.08-0.25s.
    // PERFECT ≤ 0.08s | GREAT ≤ 0.20s | MISS > 0.20s
    public float perfectWindow = 0.08f;
    public float greatWindow = 0.20f;

    // DEBUG: Set to true to auto-set a test weakness and accept all input regardless of turn state
    public bool DebugTestMode = false;
    public string DebugTestWeakness = "WASD";

    // =========================
    // State
    // =========================
    private List<char> currentCombo = new List<char>();
    private List<string> hitResults = new List<string>();

    private int perfectStreak = 0;

    public bool PlayerDidSuccessfulCombo { get; private set; }
    private bool comboFailedEarly = false;
    private bool comboCompleted = false;  // Lock out further input after combo is evaluated

    // =========================
    // Beat Timing
    // =========================
    private int lastBeatIndex = -1;
    private float timeSinceLastBeat = 0.0f;

    // Cache beat duration so we can measure distance to NEXT beat
    private float cachedBeatDuration = 0.5f; // fallback default (~120 BPM)

    // =========================
    // Enemy Weakness (injected)
    // =========================
    private string currentEnemyWeakness = "";

    // =========================
    // Initialization tracking
    // =========================
    private bool initialized = false;

    // =========================
    // Update Loop
    // =========================
    public override void Update()
    {
        // ---- Singleton setup with proper cleanup on scene reload ----
        if (Instance != null && Instance != this)
        {
            // A stale instance from a previous scene load exists — replace it.
            Console.WriteLine("[ComboSystem] Replacing stale singleton instance");
            Instance = this;
        }
        else if (Instance == null)
        {
            Instance = this;
            Console.WriteLine("[ComboSystem] Singleton instance set");
        }

        // First-time initialization
        if (!initialized)
        {
            if (!initialized)
            {
                Instance = this;
                this.DebugTestMode = true;
                this.SetEnemyWeakness("WASD");

                initialized = true;
                Console.WriteLine("[ComboSystem] Custom Engine Init: Debug Mode FORCED ON");
                return;
            }
        }

        if (Conductor.Instance == null)
            return;


        cachedBeatDuration = Conductor.Instance.GetBeatDuration();

        float dt = Time.DeltaTime;
        timeSinceLastBeat += dt;

        int currentBeat = Conductor.Instance.GetCurrentBeat();

        // ---- Guard against duplicate beat events ----
        // Only reset the timer if the beat index actually advanced.
        if (currentBeat != lastBeatIndex)
        {
            lastBeatIndex = currentBeat;
            timeSinceLastBeat = 0.0f;
        }
    }

    // =========================
    // Public API
    // =========================
    public void SetEnemyWeakness(string weaknessCombo)
    {
        currentEnemyWeakness = weaknessCombo;
        maxComboLength = weaknessCombo.Length;
        ResetCombo();
        Console.WriteLine($"[ComboSystem] Weakness set to: {weaknessCombo} (length: {maxComboLength})");
    }

    public void RegisterInput(char key)
    {
        // Block input if combo already completed (waiting for TurnManager to reset)
        if (comboCompleted)
        {
            Console.WriteLine($"[ComboSystem] [{key}] IGNORED — combo already completed, waiting for reset");
            return;
        }

        if (string.IsNullOrEmpty(currentEnemyWeakness))
        {
            Console.WriteLine("[ComboSystem] ERROR: No weakness set! Cannot register input.");
            return;
        }

        float currentSongTime = Conductor.Instance.GetSongTime();
        float beatDuration = Conductor.Instance.GetBeatDuration();
    
        float timeSince = currentSongTime % beatDuration;
        float timeUntilNext = beatDuration - timeSince;
        float delta = Math.Min(timeSince, timeUntilNext);

        // Clamp to zero in case of floating-point drift
        if (delta < 0f) delta = 0f;

        // =========================
        // Judgement
        // =========================
        string judgement;
        if (delta <= perfectWindow)
            judgement = "PERFECT";
        else if (delta <= greatWindow)
            judgement = "GREAT";
        else
            judgement = "MISS";

        hitResults.Add(judgement);
        currentCombo.Add(key);

        Console.WriteLine($"[ComboSystem] [{key}] Timing: {judgement} (Δ={delta:F3}s, sinceBeat={timeSince:F3}s, untilNext={timeUntilNext:F3}s)");
        Console.WriteLine($"[ComboSystem]   Progress: {currentCombo.Count}/{maxComboLength} | Expected: {(currentCombo.Count <= currentEnemyWeakness.Length ? currentEnemyWeakness[currentCombo.Count - 1].ToString() : "?")} | Got: {key}");

        // === VISUAL FEEDBACK ===
        if (ComboDisplayManager.Instance != null)
        {
            ComboDisplayManager.Instance.ShowJudgement(judgement);
        }
        else
        {
            Console.WriteLine("[ComboSystem] WARNING: ComboDisplayManager not found!");
        }

        // === PERFECT STREAK ===
        if (judgement == "PERFECT")
        {
            perfectStreak++;
            if (perfectStreak >= 5)
            {
                Console.WriteLine("[ComboSystem] 🔥 PERFECT STREAK x5!");
                perfectStreak = 0;
            }
        }
        else
        {
            perfectStreak = 0;
        }

        // Any MISS causes failure
        if (judgement == "MISS")
            comboFailedEarly = true;

        if (currentCombo.Count >= maxComboLength)
            EvaluateCombo();
    }

    public bool HasCompletedCombo()
    {
        return comboCompleted;
    }

    public void ResetCombo()
    {
        PlayerDidSuccessfulCombo = false;
        currentCombo.Clear();
        hitResults.Clear();
        comboFailedEarly = false;
        comboCompleted = false;
    }

    // =========================
    // Evaluation
    // =========================
    private void EvaluateCombo()
    {
        string comboString = new string(currentCombo.ToArray());
        bool allGood = !comboFailedEarly;

        foreach (string r in hitResults)
        {
            if (r != "PERFECT" && r != "GREAT")
            {
                allGood = false;
                break;
            }
        }

        Console.WriteLine($"[ComboSystem] Entered: {comboString} | Expected: {currentEnemyWeakness}");

        if (allGood && comboString == currentEnemyWeakness)
        {
            Console.WriteLine("[ComboSystem] ✅ COMBO SUCCESS!");
            PlayerDidSuccessfulCombo = true;
        }
        else
        {
            Console.WriteLine("[ComboSystem] ❌ COMBO FAILED!");
            Console.WriteLine($"[ComboSystem] Sequence match: {comboString == currentEnemyWeakness}, Timing OK: {!comboFailedEarly}");
            PlayerDidSuccessfulCombo = false;
        }

        // Lock out further input until TurnManager calls ResetCombo()
        comboCompleted = true;

        // In debug test mode, auto-reset so we can test again immediately
        if (DebugTestMode)
        {
            Console.WriteLine("[ComboSystem] [DEBUG] Auto-resetting combo for next test...");
            ResetCombo();
        }
    }

    // =========================
    // Debug / Utility
    // =========================
    public string GetCurrentComboString()
    {
        return new string(currentCombo.ToArray());
    }

    public int GetComboProgress()
    {
        return currentCombo.Count;
    }

    public string GetExpectedCombo()
    {
        return currentEnemyWeakness;
    }


    public void OnDestroy()
    {
        if (Instance == this)
            Instance = null;
    }
}