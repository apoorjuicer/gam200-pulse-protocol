using System;
using ScriptAPI;

/// <summary>
/// Displays current turn phase: "ENEMY TURN" or "PLAYER TURN"
/// Updates automatically based on TurnManager state
/// 
/// SETUP:
/// - Attach this to a UI text entity
/// - Position it at the top of the screen
/// - Text will update automatically each phase
/// </summary>
public class TurnIndicatorUI : Script
{
    // =========================
    // Global Access
    // =========================
    public static TurnIndicatorUI Instance { get; private set; }

    // =========================
    // Config
    // =========================
    public float TextScale = 60f;
    public bool AnimateTransition = true;
    public float TransitionDuration = 0.5f;

    // Position
    public float ScreenTopY = 400f;  // Y position near top of screen

    // Colors for different phases
    public bool UsePhaseColors = true;
    
    // Enemy phase: Red
    private float enemyR = 1.0f, enemyG = 0.3f, enemyB = 0.3f;
    
    // Player phase: Green
    private float playerR = 0.3f, playerG = 1.0f, playerB = 0.3f;
    
    // Countdown: Yellow
    private float countdownR = 1.0f, countdownG = 1.0f, countdownB = 0.3f;

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private TurnPhase lastPhase = TurnPhase.Countdown;
    private bool isVisible = false;
    private string currentText = "";

    // Animation
    private float currentScale = 1.0f;
    private float targetScale = 1.0f;
    private float transitionTimer = 0f;
    private float currentAlpha = 1.0f;

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

        // Check for phase changes
        if (TurnManager.Instance == null)
            return;

        TurnPhase currentPhase = TurnManager.Instance.GetCurrentPhase();

        if (currentPhase != lastPhase)
        {
            lastPhase = currentPhase;
            OnPhaseChanged(currentPhase);
        }

        // Update animations
        if (AnimateTransition && transitionTimer > 0f)
        {
            UpdateTransitionAnimation();
        }

        // Update display
        UpdateDisplay(currentPhase);
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        if (Instance == null)
            Instance = this;

        Console.WriteLine("[TurnIndicatorUI] ═══════════════════════════════");
        Console.WriteLine("[TurnIndicatorUI] Initializing Turn Indicator...");
        Console.WriteLine("[TurnIndicatorUI] ═══════════════════════════════");

        // Position at top of screen
        TransformComponent tf = GetTransform();
        tf.Y = ScreenTopY;
        tf.ScaleX = TextScale;
        tf.ScaleY = TextScale;

        Console.WriteLine($"[TurnIndicatorUI] ✓ Position: (X={tf.X}, Y={tf.Y})");
        Console.WriteLine($"[TurnIndicatorUI] ✓ Text scale: {TextScale}");
        Console.WriteLine($"[TurnIndicatorUI] ✓ Animation: {(AnimateTransition ? "ON" : "OFF")}");

        // Start visible
        Show();

        Console.WriteLine("[TurnIndicatorUI] ═══════════════════════════════");
        Console.WriteLine("[TurnIndicatorUI] Initialization complete!");
        Console.WriteLine("[TurnIndicatorUI] ═══════════════════════════════");

        initialized = true;
    }

    // =========================
    // Phase Change Handling
    // =========================
    private void OnPhaseChanged(TurnPhase newPhase)
    {
        Console.WriteLine($"[TurnIndicatorUI] 🔄 Phase changed to: {newPhase}");

        // Trigger transition animation
        if (AnimateTransition)
        {
            transitionTimer = TransitionDuration;
            targetScale = 1.3f;  // Pulse up
            currentAlpha = 0.5f;  // Fade out then in
        }

        // Update text
        UpdatePhaseText(newPhase);

        // Update color
        if (UsePhaseColors)
        {
            UpdatePhaseColor(newPhase);
        }
    }

    private void UpdatePhaseText(TurnPhase phase)
    {
        switch (phase)
        {
            case TurnPhase.Countdown:
                currentText = "GET READY!";
                break;

            case TurnPhase.EnemyDisplay:
                currentText = "ENEMY TURN";
                break;

            case TurnPhase.PlayerInput:
                currentText = "PLAYER TURN";
                break;

            case TurnPhase.Resolution:
                currentText = "RESOLVING...";
                break;
        }

        Console.WriteLine($"[TurnIndicatorUI] 📝 Text: \"{currentText}\"");

        // TODO: Update actual text rendering when available
        // GetText().SetText(currentText);
    }

    private void UpdatePhaseColor(TurnPhase phase)
    {
        SpriteComponent sprite = GetSprite();
        if (!sprite.HasSprite())
            return;

        float r, g, b;

        switch (phase)
        {
            case TurnPhase.Countdown:
                r = countdownR;
                g = countdownG;
                b = countdownB;
                break;

            case TurnPhase.EnemyDisplay:
                r = enemyR;
                g = enemyG;
                b = enemyB;
                break;

            case TurnPhase.PlayerInput:
                r = playerR;
                g = playerG;
                b = playerB;
                break;

            case TurnPhase.Resolution:
                r = 1.0f;
                g = 1.0f;
                b = 1.0f;
                break;

            default:
                r = g = b = 1.0f;
                break;
        }

        sprite.SetTint(r, g, b, currentAlpha);
        Console.WriteLine($"[TurnIndicatorUI] 🎨 Color: RGB({r:F2}, {g:F2}, {b:F2})");
    }

    // =========================
    // Display Update
    // =========================
    private void UpdateDisplay(TurnPhase phase)
    {
        // Keep text updated with beats remaining
        if (TurnManager.Instance == null)
            return;

        int beatsRemaining = TurnManager.Instance.GetBeatsRemainingInPhase();

        // Optionally add beat counter to display
        // For example: "ENEMY TURN (3 beats left)"
        // But we'll keep it simple for now
    }

    private void UpdateTransitionAnimation()
    {
        float dt = Time.DeltaTime;
        transitionTimer -= dt;

        if (transitionTimer <= 0f)
        {
            transitionTimer = 0f;
            currentScale = 1.0f;
            currentAlpha = 1.0f;
        }
        else
        {
            // Calculate progress (0 to 1)
            float progress = 1.0f - (transitionTimer / TransitionDuration);

            // Ease scale back to 1.0
            currentScale = Lerp(targetScale, 1.0f, EaseOutQuad(progress));

            // Fade alpha back to 1.0
            currentAlpha = Lerp(0.5f, 1.0f, EaseInQuad(progress));
        }

        // Apply scale
        TransformComponent tf = GetTransform();
        tf.ScaleX = TextScale * currentScale;
        tf.ScaleY = TextScale * currentScale;

        // Apply alpha
        SpriteComponent sprite = GetSprite();
        if (sprite.HasSprite())
        {
            // Get current color
            // sprite.SetTint(r, g, b, currentAlpha);
            // For now, just update alpha without changing color
        }
    }

    // =========================
    // Helper Functions
    // =========================
    private float Lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    private float EaseOutQuad(float t)
    {
        return 1f - (1f - t) * (1f - t);
    }

    private float EaseInQuad(float t)
    {
        return t * t;
    }

    // =========================
    // Show/Hide
    // =========================
    private void Show()
    {
        isVisible = true;
        TransformComponent tf = GetTransform();
        tf.IsVisible = true;
    }

    private void Hide()
    {
        isVisible = false;
        TransformComponent tf = GetTransform();
        tf.IsVisible = false;
    }

    // =========================
    // Public API
    // =========================
    public bool IsCurrentlyVisible()
    {
        return isVisible;
    }

    public string GetCurrentText()
    {
        return currentText;
    }

    public void SetPosition(float x, float y)
    {
        TransformComponent tf = GetTransform();
        tf.X = x;
        tf.Y = y;
        ScreenTopY = y;
    }

    // Allow external customization of colors
    public void SetEnemyPhaseColor(float r, float g, float b)
    {
        enemyR = r;
        enemyG = g;
        enemyB = b;
    }

    public void SetPlayerPhaseColor(float r, float g, float b)
    {
        playerR = r;
        playerG = g;
        playerB = b;
    }

    public void SetCountdownPhaseColor(float r, float g, float b)
    {
        countdownR = r;
        countdownG = g;
        countdownB = b;
    }
}
