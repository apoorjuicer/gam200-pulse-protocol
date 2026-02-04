using System;
using ScriptAPI;

/// <summary>
/// Displays the enemy's combo pattern during the Enemy Display phase.
/// Shows visual feedback for each key in the sequence.
/// </summary>
public class ComboDisplayUI : Script
{
    // =========================
    // Global Access (Singleton)
    // =========================
    public static ComboDisplayUI Instance { get; private set; }

    // =========================
    // Config
    // =========================
    public int maxComboLength = 4;

    // Sprite paths (relative to Assets/Textures/)
    public string WSprite = "W.png";
    public string ASprite = "A.png";
    public string SSpr = "S.png";
    public string DSprite = "D.png";

    public string PerfectSprite = "perfect.png";
    public string GreatSprite = "great.png";
    public string MissSprite = "miss.png";

    // =========================
    // State
    // =========================
    private string currentCombo = "";
    private int currentProgress = 0;

    private void Awake()
    {
        Instance = this;
        Console.WriteLine("[ComboDisplayUI] ? Initialized");
    }

    public void DisplayCombo(string comboString)
    {
        currentCombo = comboString;
        currentProgress = 0;

        Console.WriteLine($"[ComboDisplayUI] ?? Displaying combo: {comboString}");

        // TODO: Implement actual UI rendering (slots, sprite assignment, visibility)
    }

    public void UpdateProgress(int progress)
    {
        currentProgress = progress;
        Console.WriteLine($"[ComboDisplayUI] Progress: {progress}/{currentCombo.Length}");

        // TODO: Implement UI feedback for progress
    }

    public void ShowJudgement(string judgement)
    {
        string spritePath = GetSpriteForJudgement(judgement);

        Console.WriteLine($"[ComboDisplayUI] ? {judgement} -> {spritePath}");

        // TODO: Assign judgement sprite + show
    }

    private string GetSpriteForJudgement(string judgement)
    {
        if (string.IsNullOrEmpty(judgement)) return MissSprite;

        switch (judgement.ToUpper())
        {
            case "PERFECT": return PerfectSprite;
            case "GOOD":
            case "GREAT": return GreatSprite;
            case "MISS":
            case "LATE": return MissSprite;
            default: return MissSprite;
        }
    }
}