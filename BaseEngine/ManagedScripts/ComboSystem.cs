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

    // Timing windows (tune these)
    public float perfectWindow = 0.12f;
    public float greatWindow = 0.25f;

    // =========================
    // Runtime State
    // =========================
    private string targetCombo = "";
    private int progress = 0;
    private bool acceptingInput = false;
    private bool comboCompleted = false;
    private bool lastComboWasSuccessful = false;
    private string enemyWeakness = "";

    private void Awake()
    {
        Instance = this;
        Console.WriteLine("[ComboSystem] ? Initialized");
    }

    private void Update()
    {
        if (!acceptingInput) return;

        // TODO: Replace with your input system hooks
        // Example placeholder: if (Input.GetKeyDown(KeyCode.W)) RegisterInput('W');
    }

    // =========================
    // Public API
    // =========================
    public void BeginCombo(string combo)
    {
        if (string.IsNullOrEmpty(combo))
        {
            Console.WriteLine("[ComboSystem] ?? BeginCombo called with empty combo");
            return;
        }

        // Reset state for new combo
        targetCombo = combo;
        progress = 0;
        acceptingInput = true;
        comboCompleted = false;
        lastComboWasSuccessful = false;

        Console.WriteLine($"[ComboSystem] ?? BeginCombo: {combo}");

        // Show the combo visually
        if (ComboDisplayController.Instance != null)
            ComboDisplayController.Instance.DisplayCombo(combo);

        if (ComboSpriteDisplay.Instance != null)
            ComboSpriteDisplay.Instance.DisplayCombo(combo);
    }

    public void EndCombo()
    {
        acceptingInput = false;
        Console.WriteLine("[ComboSystem] ?? EndCombo");
    }

    public void RegisterInput(char key)
    {
        if (!acceptingInput)
        {
            Console.WriteLine($"[ComboSystem] ?? Input ignored - not accepting input");
            return;
        }

        if (progress >= targetCombo.Length)
        {
            Console.WriteLine($"[ComboSystem] ?? Input ignored - combo already complete");
            return;
        }

        char expected = targetCombo[progress];

        if (char.ToUpper(key) == char.ToUpper(expected))
        {
            progress++;

            Console.WriteLine($"[ComboSystem] ? Correct key {key} | Progress {progress}/{targetCombo.Length}");

            if (ComboDisplayController.Instance != null)
                ComboDisplayController.Instance.UpdateProgress(progress);

            if (ComboSpriteDisplay.Instance != null)
                ComboSpriteDisplay.Instance.UpdateProgress(progress);

            if (progress >= targetCombo.Length)
            {
                acceptingInput = false;
                comboCompleted = true;
                lastComboWasSuccessful = true;
                Console.WriteLine("[ComboSystem] ?? COMBO SUCCESS!");
                ShowJudgement("PERFECT");
            }
        }
        else
        {
            Console.WriteLine($"[ComboSystem] ? Wrong key {key} expected {expected}");
            acceptingInput = false;
            comboCompleted = true;
            lastComboWasSuccessful = false;
            Console.WriteLine("[ComboSystem] ?? COMBO FAILED!");
            ShowJudgement("MISS");
        }
    }

    // =========================
    // Missing Public Methods (Required by other scripts)
    // =========================

    /// <summary>
    /// Returns true if the combo has been completed (either successfully or failed)
    /// </summary>
    public bool HasCompletedCombo()
    {
        return comboCompleted;
    }

    /// <summary>
    /// Returns true if the last combo was completed successfully
    /// </summary>
    public bool PlayerDidSuccessfulCombo()
    {
        return lastComboWasSuccessful;
    }

    /// <summary>
    /// Gets the current expected combo string
    /// </summary>
    public string GetExpectedCombo()
    {
        return targetCombo;
    }

    /// <summary>
    /// Gets the current progress through the combo (0 to length)
    /// </summary>
    public int GetComboProgress()
    {
        return progress;
    }

    /// <summary>
    /// Sets the enemy's weakness combo pattern
    /// </summary>
    public void SetEnemyWeakness(string weakness)
    {
        enemyWeakness = weakness;
        Console.WriteLine($"[ComboSystem] ?? Enemy weakness set to: {weakness}");
    }

    /// <summary>
    /// Gets the enemy's weakness combo pattern
    /// </summary>
    public string GetEnemyWeakness()
    {
        return enemyWeakness;
    }

    /// <summary>
    /// Resets the combo system state - call this before starting a new combo
    /// </summary>
    public void Reset()
    {
        targetCombo = "";
        progress = 0;
        acceptingInput = false;
        comboCompleted = false;
        lastComboWasSuccessful = false;
        Console.WriteLine("[ComboSystem] ?? Reset");
    }

    // =========================
    // Judgement Display
    // =========================
    private void ShowJudgement(string judgement)
    {
        if (ComboDisplayController.Instance != null)
            ComboDisplayController.Instance.ShowJudgement(judgement);

        if (ComboSpriteDisplay.Instance != null)
            ComboSpriteDisplay.Instance.ShowJudgement(judgement);
    }
}