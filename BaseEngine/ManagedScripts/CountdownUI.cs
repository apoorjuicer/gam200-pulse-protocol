using System;
using ScriptAPI;

/// <summary>
/// Displays countdown text (3... 2... 1... GO!) before game starts
/// 
/// SETUP:
/// - Attach this to a UI text entity
/// - Position it in the center of the screen
/// - The text will automatically appear during the Countdown phase
/// </summary>
public class CountdownUI : Script
{
    // =========================
    // Global Access
    // =========================
    public static CountdownUI Instance { get; private set; }

    // =========================
    // Config
    // =========================
    public float TextScale = 100f;  // Size of countdown text
    public bool AnimateScale = true;  // Pulse animation
    public float PulseAmount = 1.3f;  // How much to scale up during pulse

    // Colors for each number (optional)
    public bool UseColorGradient = true;

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private int lastBeat = -1;
    private int lastDisplayedNumber = -1;
    private bool isVisible = false;
    
    // Animation
    private float currentScale = 1.0f;
    private float targetScale = 1.0f;
    private float scaleVelocity = 0.0f;

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

        // Check current phase
        if (TurnManager.Instance == null || Conductor.Instance == null)
            return;

        TurnPhase currentPhase = TurnManager.Instance.GetCurrentPhase();
        int currentBeat = Conductor.Instance.GetCurrentBeat();

        // Only show during countdown phase
        if (currentPhase == TurnPhase.Countdown)
        {
            if (!isVisible)
            {
                Show();
            }

            // Update display on beat change
            if (currentBeat != lastBeat)
            {
                lastBeat = currentBeat;
                UpdateCountdownDisplay();
            }

            // Animate scale
            if (AnimateScale)
            {
                AnimatePulse();
            }
        }
        else
        {
            if (isVisible)
            {
                Hide();
            }
        }
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        if (Instance == null)
            Instance = this;

        Console.WriteLine("[CountdownUI] Initialized");
        Console.WriteLine($"[CountdownUI] Text scale: {TextScale}");
        Console.WriteLine($"[CountdownUI] Animation: {(AnimateScale ? "ON" : "OFF")}");

        // Start hidden
        Hide();

        initialized = true;
    }

    // =========================
    // Display Logic
    // =========================
    private void UpdateCountdownDisplay()
    {
        if (TurnManager.Instance == null)
            return;

        int beatsInPhase = TurnManager.Instance.GetBeatsInCurrentPhase();
        int beatsRemaining = TurnManager.Instance.GetBeatsRemainingInPhase();

        // Map beats to countdown numbers
        // Beat 0 = "3"
        // Beat 1 = "2"
        // Beat 2 = "1"
        // Beat 3 = "GO!"
        
        string displayText = "";
        float r = 1.0f, g = 1.0f, b = 1.0f;

        switch (beatsInPhase)
        {
            case 0:
                displayText = "3";
                if (UseColorGradient) { r = 1.0f; g = 0.3f; b = 0.3f; } // Red
                break;
            case 1:
                displayText = "2";
                if (UseColorGradient) { r = 1.0f; g = 0.8f; b = 0.2f; } // Orange
                break;
            case 2:
                displayText = "1";
                if (UseColorGradient) { r = 1.0f; g = 1.0f; b = 0.2f; } // Yellow
                break;
            case 3:
                displayText = "GO!";
                if (UseColorGradient) { r = 0.2f; g = 1.0f; b = 0.2f; } // Green
                break;
        }

        if (beatsInPhase != lastDisplayedNumber)
        {
            lastDisplayedNumber = beatsInPhase;
            
            Console.WriteLine($"[CountdownUI] 🎯 Displaying: {displayText}");

            // Update text (if your engine supports text rendering)
            // For now, we'll just log it
            // TODO: Implement actual text rendering when available
            // GetText().SetText(displayText);

            // Update color
            SpriteComponent sprite = GetSprite();
            if (sprite.HasSprite())
            {
                sprite.SetTint(r, g, b, 1.0f);
            }

            // Trigger pulse animation
            if (AnimateScale)
            {
                targetScale = PulseAmount;
            }
        }
    }

    private void AnimatePulse()
    {
        float dt = Time.DeltaTime;

        // Smooth damp towards target scale
        float smoothTime = 0.15f;
        currentScale = SmoothDamp(currentScale, targetScale, ref scaleVelocity, smoothTime, dt);

        // Apply scale
        TransformComponent tf = GetTransform();
        tf.ScaleX = TextScale * currentScale;
        tf.ScaleY = TextScale * currentScale;

        // Reset target scale after reaching it
        if (Math.Abs(currentScale - targetScale) < 0.01f)
        {
            targetScale = 1.0f;
        }
    }

    // Simple smooth damp implementation
    private float SmoothDamp(float current, float target, ref float velocity, float smoothTime, float deltaTime)
    {
        smoothTime = Math.Max(0.0001f, smoothTime);
        float omega = 2.0f / smoothTime;
        float x = omega * deltaTime;
        float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
        float change = current - target;
        float originalTarget = target;
        
        float maxChange = float.MaxValue * smoothTime;
        change = Math.Max(-maxChange, Math.Min(change, maxChange));
        target = current - change;
        
        float temp = (velocity + omega * change) * deltaTime;
        velocity = (velocity - omega * temp) * exp;
        float output = target + (change + temp) * exp;
        
        if (originalTarget - current > 0.0f == output > originalTarget)
        {
            output = originalTarget;
            velocity = (output - originalTarget) / deltaTime;
        }
        
        return output;
    }

    // =========================
    // Show/Hide
    // =========================
    private void Show()
    {
        isVisible = true;
        TransformComponent tf = GetTransform();
        tf.IsVisible = true;
        
        // Reset animation
        currentScale = 1.0f;
        targetScale = PulseAmount;
        scaleVelocity = 0.0f;

        Console.WriteLine("[CountdownUI] 👁 Countdown visible");
    }

    private void Hide()
    {
        isVisible = false;
        TransformComponent tf = GetTransform();
        tf.IsVisible = false;

        lastDisplayedNumber = -1;

        Console.WriteLine("[CountdownUI] 🙈 Countdown hidden");
    }

    // =========================
    // Public API
    // =========================
    public bool IsCurrentlyVisible()
    {
        return isVisible;
    }
}
