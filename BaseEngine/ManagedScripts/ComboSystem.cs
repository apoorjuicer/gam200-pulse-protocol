using System;
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

    public float perfectWindow = 0.05f;
    public float goodWindow = 0.10f;
    public float missWindow = 0.15f;

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
    // Update Loop
    // =========================
    public override void Update()
    {
        if (Instance == null)
            Instance = this;

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
        float delta = Math.Abs(timeSinceLastBeat);

        string judgement;
        if (delta <= perfectWindow)
            judgement = "PERFECT";
        else if (delta <= goodWindow)
            judgement = "GOOD";
        else if (delta <= missWindow)
            judgement = "LATE";
        else
            judgement = "MISS";

        hitResults.Add(judgement);
        currentCombo.Add(key);

        Console.WriteLine($"[ComboSystem] [{key}] Timing: {judgement} (?={delta:F3}s)");

        // === PERFECT STREAK ===
        if (judgement == "PERFECT")
        {
            perfectStreak++;
            if (perfectStreak >= 5)
            {
                Console.WriteLine("[ComboSystem] ? PERFECT STREAK x5!");
                perfectStreak = 0;
            }
        }
        else
        {
            perfectStreak = 0;
        }

        if (judgement == "MISS" || judgement == "LATE")
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

        foreach (string r in hitResults)
        {
            if (r != "PERFECT" && r != "GOOD")
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
            Console.WriteLine("[ComboSystem] ? COMBO FAILED!");
            PlayerDidSuccessfulCombo = false;
        }

        ResetCombo();
    }
}