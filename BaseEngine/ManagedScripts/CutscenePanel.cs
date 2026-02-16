using System;
using ScriptAPI;

/// <summary>
/// Cutscene Panel Controller
/// ===========================
/// Attach this SINGLE script to EVERY panel entity (Panel1 through Panel14).
/// Each panel auto-detects its index from its entity name.
/// 
/// Panels share a static timer so they stay synchronized.
/// Each panel slides in from alternating sides, holds, then fades/slides out.
/// 
/// Flow per panel:
///   1. Start off-screen and invisible
///   2. When it's this panel's turn: become visible, slide in from left or right
///   3. Hold in center for a moment
///   4. Fade out (via alpha) while next panel slides in
///   5. Become invisible, stay off-screen
///
/// Press SPACE to skip the entire cutscene.
/// After all panels, loads Level1.
/// </summary>
public class CutscenePanel : Script
{
    // =========================
    // Configuration (shared across all panels)
    // =========================
    public float panelHoldDuration = 1.5f;   // How long panel stays fully visible in center
    public float slideInDuration = 0.8f;     // How long the slide-in takes
    public float fadeOutDuration = 0.6f;     // How long the fade-out takes
    public float slideDistance = 1600.0f;    // How far off-screen panels start

    // =========================
    // Static Shared State (all panels share this)
    // =========================
    private static float sharedElapsedTime = 0f;
    private static int sharedCurrentPanel = 0;
    private static bool sharedCutsceneFinished = false;
    private static bool sharedTimerStarted = false;
    private static int sharedInitCount = 0;
    private static int totalPanels = 14;

    // Per-panel duration = slideIn + hold + fadeOut
    private static float panelTotalDuration = 0f;

    // =========================
    // Per-Instance State
    // =========================
    private int myPanelIndex = -1;    // 0-based (Panel1 = 0, Panel14 = 13)
    private bool initialized = false;
    private bool isTimerOwner = false; // Only one panel advances the shared timer

    // Positions
    private float targetX = 0f;       // Center position (where panel displays)
    private float targetY = 0f;
    private float offScreenX = 0f;    // Starting off-screen position

    // Slide direction: true = comes from left, false = comes from right
    private bool slidesFromLeft = true;

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            return;
        }

        if (sharedCutsceneFinished)
            return;

        // Only the timer owner advances time and checks for skip
        if (isTimerOwner)
        {
            // Check for skip (SPACE key)
            InputComponent input = GetInput();
            if (input.IsKeyTriggered(32))
            {
                Console.WriteLine("[Cutscene] Skipped!");
                sharedCutsceneFinished = true;
                Scene.LoadScene("Level1");
                return;
            }

            // Advance shared timer
            sharedElapsedTime += Time.DeltaTime;

            // Update which panel should be active
            int newPanel = (int)(sharedElapsedTime / panelTotalDuration);
            if (newPanel != sharedCurrentPanel && newPanel < totalPanels)
            {
                sharedCurrentPanel = newPanel;
                Console.WriteLine($"[Cutscene] Panel {newPanel + 1}/{totalPanels}");
            }

            // Check if cutscene is complete
            if (sharedElapsedTime >= totalPanels * panelTotalDuration)
            {
                sharedCutsceneFinished = true;
                Console.WriteLine("[Cutscene] Complete! Loading Level1...");
                Scene.LoadScene("Level1");
                return;
            }
        }

        // Animate this panel
        AnimatePanel();
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        // Auto-detect panel index from entity name
        // Try to find this panel's name by checking known panel patterns
        myPanelIndex = DetectPanelIndex();

        if (myPanelIndex < 0)
        {
            Console.WriteLine("[CutscenePanel] ERROR: Could not determine panel index!");
            initialized = true;
            return;
        }

        // Compute per-panel total duration
        panelTotalDuration = slideInDuration + panelHoldDuration + fadeOutDuration;

        // The first panel to initialize becomes the timer owner
        sharedInitCount++;
        if (sharedInitCount == 1)
        {
            // Reset all shared state (in case of scene restart)
            sharedElapsedTime = 0f;
            sharedCurrentPanel = 0;
            sharedCutsceneFinished = false;
            sharedTimerStarted = true;
            isTimerOwner = true;
            Console.WriteLine($"[Cutscene] Timer owner: Panel {myPanelIndex + 1}");
            Console.WriteLine($"[Cutscene] Per-panel duration: {panelTotalDuration}s (slide:{slideInDuration} + hold:{panelHoldDuration} + fade:{fadeOutDuration})");
            Console.WriteLine($"[Cutscene] Total duration: {totalPanels * panelTotalDuration}s");
        }

        // Store the target (center) position from where the panel is placed in the editor
        // We want all panels to display at screen center
        targetX = 0f;   // Center of screen
        targetY = 0f;

        // Odd panels (1,3,5...) slide from left, even (2,4,6...) from right
        int panelNumber = myPanelIndex + 1;
        slidesFromLeft = (panelNumber % 2) == 1;

        if (slidesFromLeft)
            offScreenX = -slideDistance;
        else
            offScreenX = slideDistance;

        // Start off-screen and invisible
        TransformComponent tf = GetTransform();
        tf.X = offScreenX;
        tf.Y = targetY;
        tf.IsVisible = false;

        string dir = slidesFromLeft ? "LEFT" : "RIGHT";
        Console.WriteLine($"[CutscenePanel] Panel {myPanelIndex + 1} initialized - slides from {dir}");

        initialized = true;
    }

    // =========================
    // Panel Index Detection
    // =========================
    private int DetectPanelIndex()
    {
        // Since we can't easily get entity name from script,
        // use the initialization order combined with static counter.
        // Panels are processed in hierarchy order (Panel1 first, Panel14 last).
        // We use a simple static counter that increments per initialization.
        
        // Actually, let's use a smarter approach:
        // The sharedInitCount tells us which panel we are (1st to init = Panel1, etc.)
        // This works because the engine processes entities in hierarchy order.
        return sharedInitCount; // This will be 0 for first, 1 for second, etc.
                                // (incremented AFTER this call in Initialize())
    }

    // =========================
    // Animation Logic
    // =========================
    private void AnimatePanel()
    {
        if (myPanelIndex < 0) return;

        TransformComponent tf = GetTransform();
        SpriteComponent sprite = GetSprite();

        // Calculate this panel's local time
        float panelStartTime = myPanelIndex * panelTotalDuration;
        float localTime = sharedElapsedTime - panelStartTime;

        // Phase boundaries
        float slideInEnd = slideInDuration;
        float holdEnd = slideInDuration + panelHoldDuration;
        float fadeOutEnd = panelTotalDuration;

        // Before this panel's turn
        if (localTime < 0f)
        {
            tf.IsVisible = false;
            tf.X = offScreenX;
            ResetAlpha(sprite);
            return;
        }

        // After this panel is done
        if (localTime >= fadeOutEnd)
        {
            tf.IsVisible = false;
            tf.X = offScreenX;
            ResetAlpha(sprite);
            return;
        }

        // Panel is active
        tf.IsVisible = true;

        if (localTime < slideInEnd)
        {
            // === PHASE 1: SLIDE IN ===
            float t = localTime / slideInDuration;
            float eased = EaseOutCubic(t);

            tf.X = Lerp(offScreenX, targetX, eased);
            tf.Y = targetY;

            // Fade in during slide
            float alpha = Lerp(0f, 1f, eased);
            SetAlpha(sprite, alpha);
        }
        else if (localTime < holdEnd)
        {
            // === PHASE 2: HOLD (fully visible, centered) ===
            tf.X = targetX;
            tf.Y = targetY;
            SetAlpha(sprite, 1f);
        }
        else
        {
            // === PHASE 3: FADE OUT ===
            float t = (localTime - holdEnd) / fadeOutDuration;
            float eased = EaseInQuad(t);

            // Stay in place but fade out
            tf.X = targetX;
            tf.Y = targetY;

            float alpha = Lerp(1f, 0f, eased);
            SetAlpha(sprite, alpha);
        }
    }

    // =========================
    // Alpha / Tint Helpers
    // =========================
    private void SetAlpha(SpriteComponent sprite, float alpha)
    {
        if (sprite.HasSprite())
        {
            sprite.TintA = alpha;
        }
    }

    private void ResetAlpha(SpriteComponent sprite)
    {
        if (sprite.HasSprite())
        {
            sprite.TintA = 1f;
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

    private float EaseOutCubic(float t)
    {
        float f = 1f - t;
        return 1f - f * f * f;
    }

    private float EaseInQuad(float t)
    {
        return t * t;
    }

    private float EaseOutQuad(float t)
    {
        return 1f - (1f - t) * (1f - t);
    }
}
