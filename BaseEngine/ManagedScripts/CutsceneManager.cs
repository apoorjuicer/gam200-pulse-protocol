using System;
using ScriptAPI;

/// <summary>
/// Cutscene Panel Controller (Paired System)
/// ============================================
/// Attach this script to EVERY panel entity (Panel1 through Panel14).
/// 
/// Layout:
///   Pairs: (1+2), (3+4), (5+6), (7+8), (9+10), (11+12), (13+14)
///   - Odd panel = LEFT side, Even panel = RIGHT side
///   - Panel 14 is solo (last pair has 13 left, 14 right — actually this makes 7 full pairs)
///   
///   Wait — 14 panels = 7 pairs exactly: (1,2) (3,4) (5,6) (7,8) (9,10) (11,12) (13,14)
///
/// Animation flow per pair:
///   1. Both panels start below screen (invisible)
///   2. Both slide UP into view simultaneously (left panel to left position, right to right)
///   3. Hold in place
///   4. Both slide UP and out of view while NEXT pair slides UP into view from below
///
/// Press SPACE to skip. Loads Level1 when done.
/// </summary>
public class CutscenePanel : Script
{
    // =========================
    // Timing Configuration
    // =========================
    public float slideInDuration = 0.8f;     // Time to slide into view
    public float holdDuration = 2.0f;        // Time pair stays fully visible
    public float slideOutDuration = 0.8f;    // Time to slide out (overlaps with next pair sliding in)
    public float overlapDuration = 0.6f;     // How much the exit/enter overlap

    // =========================
    // Position Configuration
    // =========================
    public float leftPanelX = -420f;         // X position for left panel (odd panels)
    public float rightPanelX = 420f;         // X position for right panel (even panels)
    public float centerY = 0f;              // Y position when visible
    public float offScreenBelow = -900f;    // Y position below screen (entry)
    public float offScreenAbove = 900f;     // Y position above screen (exit)

    // =========================
    // Static Shared State
    // =========================
    private static float sharedElapsedTime = 0f;
    private static bool sharedCutsceneFinished = false;
    private static int sharedInitCount = 0;
    private static int totalPairs = 7;
    private static float pairTotalDuration = 0f;  // Computed from timing config

    // =========================
    // Per-Instance State
    // =========================
    private int myPanelIndex = -1;    // 0-based (Panel1=0, Panel14=13)
    private int myPairIndex = -1;     // Which pair this panel belongs to (0-6)
    private bool isLeftPanel = true;  // true = left side, false = right side
    private bool initialized = false;
    private bool isTimerOwner = false;

    private float myTargetX = 0f;     // Where this panel sits when visible

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            return;
        }

        if (sharedCutsceneFinished)
            return;

        // Timer owner advances time and handles skip
        if (isTimerOwner)
        {
            InputComponent input = GetInput();
            if (input.IsKeyTriggered(32)) // SPACE
            {
                Console.WriteLine("[Cutscene] Skipped!");
                sharedCutsceneFinished = true;
                Scene.LoadScene("Level1");
                return;
            }

            sharedElapsedTime += Time.DeltaTime;

            // Check if cutscene is complete
            float totalDuration = (totalPairs - 1) * (pairTotalDuration - overlapDuration) + pairTotalDuration;
            if (sharedElapsedTime >= totalDuration)
            {
                sharedCutsceneFinished = true;
                Console.WriteLine("[Cutscene] Complete! Loading Level1...");
                Scene.LoadScene("Level1");
                return;
            }
        }

        AnimatePanel();
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        // Auto-detect panel index from initialization order
        myPanelIndex = sharedInitCount;
        sharedInitCount++;

        // Compute pair info
        // Panel 0,1 = pair 0 | Panel 2,3 = pair 1 | ... | Panel 12,13 = pair 6
        myPairIndex = myPanelIndex / 2;
        isLeftPanel = (myPanelIndex % 2) == 0; // Even index = left (Panel1,3,5...)

        // Compute timing
        pairTotalDuration = slideInDuration + holdDuration + slideOutDuration;

        // Set target X based on side
        myTargetX = isLeftPanel ? leftPanelX : rightPanelX;

        // First panel to init becomes timer owner and resets shared state
        if (myPanelIndex == 0)
        {
            sharedElapsedTime = 0f;
            sharedCutsceneFinished = false;
            isTimerOwner = true;

            float totalDuration = (totalPairs - 1) * (pairTotalDuration - overlapDuration) + pairTotalDuration;
            Console.WriteLine($"[Cutscene] Timer owner: Panel {myPanelIndex + 1}");
            Console.WriteLine($"[Cutscene] Pair duration: {pairTotalDuration}s (slide-in:{slideInDuration} hold:{holdDuration} slide-out:{slideOutDuration})");
            Console.WriteLine($"[Cutscene] Overlap: {overlapDuration}s | Total pairs: {totalPairs}");
            Console.WriteLine($"[Cutscene] Total cutscene: {totalDuration:F1}s");
        }

        // Start off-screen below and invisible
        TransformComponent tf = GetTransform();
        tf.X = myTargetX;
        tf.Y = offScreenBelow;
        tf.IsVisible = false;

        string side = isLeftPanel ? "LEFT" : "RIGHT";
        Console.WriteLine($"[CutscenePanel] Panel {myPanelIndex + 1} -> Pair {myPairIndex + 1} ({side}) at X={myTargetX}");

        initialized = true;
    }

    // =========================
    // Animation
    // =========================
    private void AnimatePanel()
    {
        if (myPanelIndex < 0) return;

        TransformComponent tf = GetTransform();
        SpriteComponent sprite = GetSprite();

        // Calculate when this pair starts
        // Each pair starts (pairTotalDuration - overlapDuration) after the previous one
        float pairStartTime = myPairIndex * (pairTotalDuration - overlapDuration);
        float localTime = sharedElapsedTime - pairStartTime;

        // Phase boundaries
        float slideInEnd = slideInDuration;
        float holdEnd = slideInDuration + holdDuration;
        float slideOutEnd = pairTotalDuration;

        // Before this pair's turn
        if (localTime < 0f)
        {
            tf.IsVisible = false;
            tf.X = myTargetX;
            tf.Y = offScreenBelow;
            SetAlpha(sprite, 0f);
            return;
        }

        // After this pair is fully done
        if (localTime >= slideOutEnd)
        {
            tf.IsVisible = false;
            tf.X = myTargetX;
            tf.Y = offScreenAbove;
            SetAlpha(sprite, 1f);
            return;
        }

        // === Panel is active ===
        tf.IsVisible = true;
        tf.X = myTargetX;

        if (localTime < slideInEnd)
        {
            // === PHASE 1: SLIDE UP INTO VIEW (from below) ===
            float t = localTime / slideInDuration;
            float eased = EaseOutCubic(t);

            tf.Y = Lerp(offScreenBelow, centerY, eased);
            SetAlpha(sprite, Lerp(0.3f, 1f, eased));
        }
        else if (localTime < holdEnd)
        {
            // === PHASE 2: HOLD (fully visible, in position) ===
            tf.Y = centerY;
            SetAlpha(sprite, 1f);
        }
        else
        {
            // === PHASE 3: SLIDE UP AND OUT (exit upward) ===
            float t = (localTime - holdEnd) / slideOutDuration;
            float eased = EaseInCubic(t);

            tf.Y = Lerp(centerY, offScreenAbove, eased);
            SetAlpha(sprite, Lerp(1f, 0f, eased));
        }
    }

    // =========================
    // Alpha Helper
    // =========================
    private void SetAlpha(SpriteComponent sprite, float alpha)
    {
        if (sprite.HasSprite())
        {
            sprite.TintA = alpha;
        }
    }

    // =========================
    // Math Helpers
    // =========================
    private float Lerp(float a, float b, float t)
    {
        if (t < 0f) t = 0f;
        if (t > 1f) t = 1f;
        return a + (b - a) * t;
    }

    // Smooth deceleration (for slide-in)
    private float EaseOutCubic(float t)
    {
        float f = 1f - t;
        return 1f - f * f * f;
    }

    // Smooth acceleration (for slide-out)
    private float EaseInCubic(float t)
    {
        return t * t * t;
    }
}