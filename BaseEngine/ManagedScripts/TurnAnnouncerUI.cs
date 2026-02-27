using System;
using ScriptAPI;

public class TurnAnnouncerUI : Script
{
    public static TurnAnnouncerUI? Instance { get; private set; }

    public float FontSize = 80f;

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private TurnPhase lastPhase = TurnPhase.Countdown;

    public override void Update()
    {
        //yes it is
        //Console.WriteLine("IS THIS EVEN WORKING?");

        if (!initialized)
        {
            Initialize();
            return;
        }

        // Hide during pause overlay
        if (NavigationButtons.ShowPauseOverlay)
        {
            Hide();
            return;
        }

        if (TurnManager.Instance == null)
            return;

        TurnPhase phase = TurnManager.Instance.GetCurrentPhase();

        // Only act on phase transitions
        if (phase == lastPhase)
            return;
        var text = GetTextComponent();
   
        lastPhase = phase;
        OnPhaseChanged(phase);
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        // Reset old instance if it's stale
        Instance = this;

        // Null out on re-init so stale references don't persist
        lastPhase = TurnPhase.Countdown;
        initialized = false; // will be set true at end

        var text = GetTextComponent();
        text.SetFontSize(FontSize);
        text.SetAlignment(1);

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
                ShowEnemy("MEMORIZE!");
                break;
            case TurnPhase.PlayerInput:
                ShowPlayer("INPUT NOW!");
                break;
            default:
                Hide();
                break;
        }
    }

    // =========================
    // Show / Hide
    // =========================
    private void ShowPlayer(string message)
    {
        var text = GetTextComponent();
        text.SetText(message);
    //    text.SetColor(0.1333f, 0.8275f, 0.9333f); 

        text.SetVisible(true);
        Console.WriteLine($"[TurnAnnouncerUI] >> {message}");
    }

    private void ShowEnemy(string message)
    {
        var text = GetTextComponent();
        text.SetText(message);
        // = (0.4863, 0.2275, 0.9294)
        // = (0.4275, 0.1569, 0.8510)
        // = (0.6549, 0.5451, 0.9804)
        // = (0.7529, 0.1490, 0.8275)
        // text.SetColor(0.7529f, 0.5176f, 0.9882f); 

        text.SetVisible(true);
        Console.WriteLine($"[TurnAnnouncerUI] >> {message}");
    }

    private void Hide()
    {
        var text = GetTextComponent();
        text.SetVisible(false);
    }
}
