using System;
using ScriptAPI;

public class ComboSystem : Script
{
    // =========================
    // Global Access (Singleton)
    // =========================
    public static ComboSystem Instance { get; private set; }

    // =========================
    // Config - LENIENT TIMING WINDOWS
    // =========================
    public int maxComboLength = 4;

    // CHANGED: Much more lenient timing windows
    public float perfectWindow = 0.5f;   // 0.5 seconds = PERFECT
    public float goodWindow = 1.0f;      // 1.0 seconds = GOOD
    // Anything over 1.0s = MISS

    // =========================
    // State
    // =========================
    private List<char> currentCombo = new List<char>();
    private List<string> hitResults = new List<string>();

    private int perfectStreak = 0;

    public bool PlayerDidSuccessfulCombo { get; private set; }
    private bool comboFailedEarly = false;

    // =========================
    // Beat Timing
    // =========================
    private int lastBeatIndex = -1;
    private float timeSinceLastBeat = 0.0f;

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
        // Set singleton instance
        if (Instance == null)
        {
            Instance = this;
            Console.WriteLine("[ComboSystem] Singleton instance set");
        }

        // First-time initialization
        if (!initialized)
        {
            Console.WriteLine("[ComboSystem] Initializing...");
            Console.WriteLine($"[ComboSystem] Timing windows: PERFECT ?{perfectWindow}s, GOOD ?{goodWindow}s, MISS >{goodWindow}s");
            ResetCombo();
            initialized = true;
        }

        float dt = Time.DeltaTime;
        timeSinceLastBeat += dt;

        if (Conductor.Instance == null)
            return;

        int currentBeat = Conductor.Instance.GetCurrentBeat();
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
        ResetCombo();
        Console.WriteLine($"[ComboSystem] Weakness set to: {weaknessCombo}");
    }

    public void RegisterInput(char key)
    {
        // Sanity check: don't allow input if no weakness set
        if (string.IsNullOrEmpty(currentEnemyWeakness))
        {
            Console.WriteLine("[ComboSystem] ERROR: No weakness set! Cannot register input.");
            return;
        }

        // Use absolute value of time since last beat
        float delta = Math.Abs(timeSinceLastBeat);

        string judgement;
        if (delta <= perfectWindow)
            judgement = "PERFECT";
        else if (delta <= goodWindow)
            judgement = "GOOD";
        else
            judgement = "MISS";

        hitResults.Add(judgement);
        currentCombo.Add(key);

        // Enhanced logging with visual indicator
        string indicator = judgement == "PERFECT" ? "?" : (judgement == "GOOD" ? "?" : "?");
        Console.WriteLine($"[ComboSystem] {indicator} [{key}] Timing: {judgement} (?={delta:F3}s)");

        // === PERFECT STREAK ===
        if (judgement == "PERFECT")
        {
            perfectStreak++;
            if (perfectStreak >= 5)
            {
                Console.WriteLine("[ComboSystem] ?? PERFECT STREAK x5!");
                perfectStreak = 0;
            }
        }
        else
        {
            perfectStreak = 0;
        }

        // Only MISS causes failure now (GOOD is acceptable)
        if (judgement == "MISS")
            comboFailedEarly = true;

        if (currentCombo.Count >= maxComboLength)
            EvaluateCombo();
    }

    public bool HasCompletedCombo()
    {
        return currentCombo.Count >= maxComboLength;
    }

    public void ResetCombo()
    {
        PlayerDidSuccessfulCombo = false;
        currentCombo.Clear();
        hitResults.Clear();
        comboFailedEarly = false;
    }

    // =========================
    // Evaluation
    // =========================
    private void EvaluateCombo()
    {
        string comboString = new string(currentCombo.ToArray());
        bool allGood = !comboFailedEarly;

        // Check if all hits were PERFECT or GOOD (no MISS)
        foreach (string r in hitResults)
        {
            if (r == "MISS")
            {
                allGood = false;
                break;
            }
        }

        Console.WriteLine($"[ComboSystem] Entered: {comboString} | Expected: {currentEnemyWeakness}");

        if (allGood && comboString == currentEnemyWeakness)
        {
            Console.WriteLine("[ComboSystem] ? COMBO SUCCESS!");
            PlayerDidSuccessfulCombo = true;
        }
        else
        {
            if (comboString != currentEnemyWeakness)
                Console.WriteLine($"[ComboSystem] ? COMBO FAILED! Wrong sequence: {comboString} ? {currentEnemyWeakness}");
            else
                Console.WriteLine("[ComboSystem] ? COMBO FAILED! Timing was off (had MISS)");

            PlayerDidSuccessfulCombo = false;
        }
    }

    // =========================
    // Debug/Utility
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

    public List<string> GetHitResults()
    {
        return new List<string>(hitResults); // Return copy
    }
}