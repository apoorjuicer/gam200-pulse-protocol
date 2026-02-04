using System;
using ScriptAPI;

/// <summary>
/// Attach this script to the JudgementDisplay entity
/// Shows PERFECT/GREAT/MISS feedback
/// </summary>
public class ComboJudgementDisplay : Script
{
    // Configuration
    public string PerfectSprite = "perfect.png";
    public string GreatSprite = "great.png";
    public string MissSprite = "miss.png";

    public float DisplayDuration = 0.6f;

    // State
    private float displayTimer = 0f;
    private bool isShowing = false;
    private bool hasRegistered = false;

    public override void Update()
    {
        // Register with manager on first update
        if (!hasRegistered)
        {
            ComboDisplayManager.RegisterJudgement(this);

            // Hide initially
            Hide();

            hasRegistered = true;
            Console.WriteLine("[JudgementDisplay] Initialized and hidden");
        }

        // Continue with normal update
        if (isShowing)
        {
            displayTimer -= Time.DeltaTime;

            if (displayTimer <= 0)
            {
                Hide();
            }
        }
    }

    public void ShowJudgement(string judgement)
    {
        var sprite = GetSprite();
        if (!sprite.HasSprite())
        {
            Console.WriteLine("[JudgementDisplay] ERROR: No sprite component!");
            return;
        }

        // Set the appropriate texture
        string texturePath = GetSpriteForJudgement(judgement);
        sprite.Texture = texturePath;

        // Make visible
        var transform = GetTransform();
        transform.IsVisible = true;

        // Start timer
        isShowing = true;
        displayTimer = DisplayDuration;

        Console.WriteLine($"[JudgementDisplay] ⭐ Showing {judgement}: {texturePath}");
    }

    public void Hide()
    {
        var transform = GetTransform();
        transform.IsVisible = false;
        isShowing = false;
    }

    private string GetSpriteForJudgement(string judgement)
    {
        switch (judgement.ToUpper())
        {
            case "PERFECT": return PerfectSprite;
            case "GOOD":
            case "GREAT": return GreatSprite;
            case "MISS":
            case "LATE": return MissSprite;
            default:
                Console.WriteLine($"[JudgementDisplay] Unknown judgement: {judgement}");
                return MissSprite;
        }
    }
}