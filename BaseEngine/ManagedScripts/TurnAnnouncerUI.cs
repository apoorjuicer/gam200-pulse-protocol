using System;
using ScriptAPI;

/// <summary>
/// Shows "Enemy's Turn!" for the entire EnemyDisplay phase and
/// "Your Turn!" for the entire PlayerInput phase.
/// Text stays visible until the phase ends — no flash/auto-hide.
///
/// SETUP:
/// - Attached to the "Turn Announcer" entity in Level1.json (has a TextComponent).
/// </summary>
public class TurnAnnouncerUI : Script
{
    public static TurnAnnouncerUI Instance { get; private set; }

    public float FontSize = 80f;

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private TurnPhase lastPhase = TurnPhase.Countdown;

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            return;
        }

        if (TurnManager.Instance == null)
            return;

        TurnPhase phase = TurnManager.Instance.GetCurrentPhase();

        // Only act on phase transitions
        if (phase == lastPhase)
            return;

        lastPhase = phase;
        OnPhaseChanged(phase);
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        Instance = this;

        var text = GetTextComponent();
        text.SetFontSize(FontSize);
        text.SetAlignment(1); // center
        text.SetColor(1.0f, 1.0f, 1.0f); // white

        Hide();

        initialized = true;
        Console.WriteLine("[TurnAnnouncerUI] Initialized");
    }

    // =========================
    // Phase Change Handler
    // =========================
    private void OnPhaseChanged(TurnPhase phase)
    {
        switch (phase)
        {
            case TurnPhase.EnemyDisplay:
                Show("Enemy's Turn!");
                break;
            case TurnPhase.PlayerInput:
                Show("Your Turn!");
                break;
            default:
                Hide();
                break;
        }
    }

    // =========================
    // Show / Hide
    // =========================
    private void Show(string message)
    {
        var text = GetTextComponent();
        text.SetText(message);
        text.SetColor(1.0f, 1.0f, 1.0f); // always white
        text.SetVisible(true);
        Console.WriteLine($"[TurnAnnouncerUI] >> {message}");
    }

    private void Hide()
    {
        var text = GetTextComponent();
        text.SetVisible(false);
    }
}
