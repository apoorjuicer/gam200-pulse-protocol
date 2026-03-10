using System;
using System.Collections.Generic;
using ScriptAPI;

public class ComboSystem : Script
{
    // =========================
    // Global Access (Singleton)
    // =========================
    public static ComboSystem? Instance { get; private set; }

    // =========================
    // Config
    // =========================
    public int maxComboLength = 4;

    // Timing windows (seconds): Distance to the nearest beat boundary.
    // At 120 BPM (beatDuration = 0.5s), max possible delta is 0.25s.
    // Setting greatWindow < 0.25s creates a genuine MISS dead-zone in the middle of each beat.
    // PERFECT ≤ 0.06s (~3-4 frames @ 60fps)
    // GREAT   ≤ 0.125s (~7-8 frames @ 60fps)
    // MISS    >  0.125s  (the central 50% of each beat interval pressing mid-beat) :(
    public float perfectWindow = 0.08f;
    public float greatWindow   = 0.15f;


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
    private bool comboCompleted = false;  // Lock out further input after combo is evaluated

    // =========================
    // Beat Timing
    // =========================
    private int lastBeatIndex = -1;

    // Cache beat duration so we can measure distance to NEXT beat
    private float cachedBeatDuration = 0.5f; // fallback default (~120 BPM)

    private int lastInputBeat = -1;

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
        // Console.WriteLine($"[CS:DEBUG] initialized={initialized}, Instance==this:{Instance == this}, weakness='{currentEnemyWeakness}'");

   
        if (Instance != null && Instance != this && !initialized)
        {
            string preservedWeakness = Instance.currentEnemyWeakness;
            Instance = this;
            if (!string.IsNullOrEmpty(preservedWeakness))
            {
                currentEnemyWeakness = preservedWeakness;
                maxComboLength = preservedWeakness.Length;
                Console.WriteLine($"[ComboSystem] Preserved weakness from old instance: {preservedWeakness}");
            }
        }

        if (Instance == null)
        {
            Instance = this;
            Console.WriteLine("[ComboSystem] Singleton instance set");
        }

        // First-time initialization
        if (!initialized)
        {
            Instance = this;
     
            initialized = true;
            Console.WriteLine("[ComboSystem] Initialized");
            return;
        }

        
        if (Instance != this)
            return;

        if (Conductor.Instance == null)
            return;


        cachedBeatDuration = Conductor.Instance!.GetBeatDuration();

        int currentBeat = Conductor.Instance!.GetCurrentBeat();
        if (currentBeat != lastBeatIndex)
            lastBeatIndex = currentBeat;
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

        float beatDuration = Conductor.Instance!.GetBeatDuration();
        // Back-date by half a frame: InputBridge detects taps one frame late (prev/cur polling),
        // so the press likely happened ~0.5 frames ago on average.
        float currentSongTime = Conductor.Instance!.GetAdjustedSongTime();

        float timeSince = (currentSongTime % beatDuration + beatDuration) % beatDuration;
        float timeUntilNext = beatDuration - timeSince;
        
        float delta = Math.Min(timeSince, timeUntilNext);

        // Clamp to zero in case of floating-point drift
        if (delta < 0f) delta = 0f;

        // =========================
        // One-key-per-beat gate
        // =========================
        
        int targetBeat = (timeSince <= timeUntilNext)
            ? Conductor.Instance!.GetCurrentBeat()
            : Conductor.Instance!.GetCurrentBeat() + 1;

        if (targetBeat == lastInputBeat)
        {
            Console.WriteLine($"[ComboSystem] [{key}] IGNORED — beat slot {targetBeat} already used (one key per beat)");
            return;
        }
        lastInputBeat = targetBeat;

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

        // =========================
        // MISS rejection
        // =========================
        
        if (judgement == "MISS")
        {
            Console.WriteLine($"[ComboSystem] [{key}] MISS — pressed too far from beat (Δ={delta:F3}s). Slot {targetBeat} freed.");
            lastInputBeat = -1; // free the slot so they can retry on the same beat window
            if (ComboDisplayManager.Instance != null)
                ComboDisplayManager.Instance.ShowJudgement("MISS");
            VisualBeatSystem.Instance?.FlashJudgement("MISS");
            // Camera shake feedback for a bad press can be added here via VFXAPI.ShakeCamera / VFXAPI.SpawnParticles
            return;
        }

        hitResults.Add(judgement);
        currentCombo.Add(key);

        Console.WriteLine($"[ComboSystem] [{key}] Timing: {judgement} (Δ={delta:F3}s, sinceBeat={timeSince:F3}s, untilNext={timeUntilNext:F3}s)");
        Console.WriteLine($"[ComboSystem]   Progress: {currentCombo.Count}/{maxComboLength} | Expected: {(currentCombo.Count <= currentEnemyWeakness.Length ? currentEnemyWeakness[currentCombo.Count - 1].ToString() : "?")} | Got: {key}");

        // === VISUAL FEEDBACK ===
        // If the key is wrong, always show MISS visually regardless of timing, if not might mess up w the timing
        string visualJudgement = judgement;
        if (currentCombo.Count <= currentEnemyWeakness.Length)
        {
            char expectedKey = currentEnemyWeakness[currentCombo.Count - 1];
            if (key != expectedKey)
                visualJudgement = "MISS";
        }

        if (ComboDisplayManager.Instance != null)
        {
            ComboDisplayManager.Instance.ShowJudgement(visualJudgement);
        }
        else
        {
            Console.WriteLine("[ComboSystem] WARNING: ComboDisplayManager not found!");
        }

        VisualBeatSystem.Instance?.FlashJudgement(visualJudgement);

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
        comboCompleted = false;
        lastInputBeat = -1;
    }

    // =========================
    // Evaluation
    // =========================
    private void EvaluateCombo()
    {
        string comboString = new string(currentCombo.ToArray());
        bool sequenceCorrect = (comboString == currentEnemyWeakness);
        bool timingOk = true;

        foreach (string r in hitResults)
        {
            if (r == "MISS")
            {
                timingOk = false;
                break;
            }
        }

        Console.WriteLine($"[ComboSystem] Entered: {comboString} | Expected: {currentEnemyWeakness}");

        if (sequenceCorrect && timingOk)
        {
            Console.WriteLine("[ComboSystem] ✅ COMBO SUCCESS!");
            PlayerDidSuccessfulCombo = true;
        }
        else
        {
            Console.WriteLine("[ComboSystem] ❌ COMBO FAILED!");
            Console.WriteLine($"[ComboSystem] Sequence match: {sequenceCorrect}, Timing OK: {timingOk}");
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