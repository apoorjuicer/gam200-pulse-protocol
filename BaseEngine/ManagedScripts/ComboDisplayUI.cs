using System;
using ScriptAPI;

public class ComboDisplayUI : Script
{
    // =========================
    // References
    // =========================
    public TurnManager TurnManagerRef = null;
    public EnemyRhythmController EnemyRef = null;

    // =========================
    // UI Settings
    // =========================
    public float FlashInterval = 0.5f;  // How fast the keys flash
    public float DisplayScale = 1.5f;   // Scale of the combo display

    // UI element entity IDs (you'll need to set these based on your UI system)
    public int ComboTextEntityId = 0;
    public int PhaseTextEntityId = 0;

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private float flashTimer = 0f;
    private bool isVisible = false;
    private TurnPhase lastPhase = TurnPhase.Countdown;

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

        float dt = Time.DeltaTime;

        if (TurnManagerRef == null || EnemyRef == null)
            return;

        TurnPhase currentPhase = TurnManagerRef.GetCurrentPhase();

        // Phase change detection
        if (currentPhase != lastPhase)
        {
            OnPhaseChanged(lastPhase, currentPhase);
            lastPhase = currentPhase;
        }

        // Update display based on phase
        switch (currentPhase)
        {
            case TurnPhase.Countdown:
                ShowCountdown();
                break;

            case TurnPhase.EnemyDisplay:
                ShowEnemyCombo();
                UpdateFlashing(dt);
                break;

            case TurnPhase.PlayerInput:
                ShowPlayerPrompt();
                break;

            case TurnPhase.Resolution:
                ShowResolution();
                break;
        }
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        Console.WriteLine("[ComboDisplayUI] Initialized");
        initialized = true;

        if (TurnManagerRef != null)
        {
            lastPhase = TurnManagerRef.GetCurrentPhase();
        }
    }

    // =========================
    // Phase Change Handler
    // =========================
    private void OnPhaseChanged(TurnPhase oldPhase, TurnPhase newPhase)
    {
        Console.WriteLine($"[ComboDisplayUI] Phase changed: {oldPhase} ? {newPhase}");

        switch (newPhase)
        {
            case TurnPhase.EnemyDisplay:
                flashTimer = 0f;
                isVisible = true;
                break;

            case TurnPhase.PlayerInput:
                isVisible = true;  // Keep showing but stop flashing
                break;

            case TurnPhase.Resolution:
                // Could show success/fail animation here
                break;
        }
    }

    // =========================
    // Display Methods
    // =========================
    private void ShowCountdown()
    {
        if (TurnManagerRef == null)
            return;

        int remaining = TurnManagerRef.GetBeatsRemainingInPhase();
        Console.WriteLine($"[UI] Countdown: {remaining}");

        // Update your UI text here
        // Example: SetUIText(PhaseTextEntityId, $"Get Ready... {remaining}");
    }

    private void ShowEnemyCombo()
    {
        if (EnemyRef == null)
            return;

        string combo = EnemyRef.WeaknessCombo;
        int remaining = TurnManagerRef.GetBeatsRemainingInPhase();

        // Format the combo for display
        string formattedCombo = FormatCombo(combo);

        Console.WriteLine($"[UI] Enemy Combo: {formattedCombo} (beats remaining: {remaining})");

        // Update your UI text here
        // Example: SetUIText(ComboTextEntityId, formattedCombo);
        // Example: SetUIText(PhaseTextEntityId, $"ENEMY TURN ({remaining} beats left)");
    }

    private void ShowPlayerPrompt()
    {
        if (EnemyRef == null)
            return;

        string combo = EnemyRef.WeaknessCombo;
        int remaining = TurnManagerRef.GetBeatsRemainingInPhase();

        string formattedCombo = FormatCombo(combo);

        Console.WriteLine($"[UI] YOUR TURN! Input: {formattedCombo} ({remaining} beats left)");

        // Update your UI text here with different color/style
        // Example: SetUIText(PhaseTextEntityId, $"YOUR TURN! ({remaining} beats left)");
    }

    private void ShowResolution()
    {
        // Show success or failure message
        Console.WriteLine("[UI] Resolving...");
    }

    // =========================
    // Visual Effects
    // =========================
    private void UpdateFlashing(float dt)
    {
        flashTimer += dt;

        if (flashTimer >= FlashInterval)
        {
            flashTimer = 0f;
            isVisible = !isVisible;

            // Toggle visibility of combo display
            // Example: SetUIVisibility(ComboTextEntityId, isVisible);
        }
    }

    private string FormatCombo(string combo)
    {
        if (string.IsNullOrEmpty(combo))
            return "";

        // Add spaces between letters for readability
        string formatted = "";
        for (int i = 0; i < combo.Length; i++)
        {
            formatted += combo[i];
            if (i < combo.Length - 1)
                formatted += " ";
        }

        return formatted;
    }

    // =========================
    // UI Helper Methods
    // =========================
    // You'll need to implement these based on your engine's UI system

    private void SetUIText(int entityId, string text)
    {
        // Placeholder - implement based on your UI system
        // Example using your engine's TextComponent:
        // TextComponent textComp = GetTextComponent(entityId);
        // if (textComp != null)
        //     textComp.Text = text;
    }

    private void SetUIVisibility(int entityId, bool visible)
    {
        // Placeholder - implement based on your UI system
        // Example:
        // TransformComponent tf = GetTransformComponent(entityId);
        // if (tf != null)
        //     tf.IsVisible = visible;
    }

    private void SetUIColor(int entityId, float r, float g, float b, float a)
    {
        // Placeholder - implement based on your UI system
    }
}