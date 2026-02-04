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
    public bool ShowDuringEnemyTurn = true;
    public bool ShowDuringPlayerTurn = false;
    public float FlashDuration = 0.2f;  // How long each key "flashes"

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private string currentComboToDisplay = "";
    private int currentKeyIndex = 0;
    private float flashTimer = 0f;
    private bool isFlashing = false;
    private int lastBeat = -1;

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

        // Handle flashing animation
        if (isFlashing)
        {
            flashTimer -= dt;
            if (flashTimer <= 0)
            {
                isFlashing = false;
            }
        }

        // Check for beat changes during Enemy Display
        if (Conductor.Instance != null && TurnManager.Instance != null)
        {
            int currentBeat = Conductor.Instance.GetCurrentBeat();

            if (currentBeat != lastBeat)
            {
                lastBeat = currentBeat;
                HandleBeatChange();
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

        Console.WriteLine("[ComboDisplayUI] Initialized");
        initialized = true;
    }

    // =========================
    // Display Logic
    // =========================
    private void HandleBeatChange()
    {
        if (TurnManager.Instance.GetCurrentPhase() != TurnPhase.EnemyDisplay)
        {
            // Hide UI when not in enemy display phase
            HideComboDisplay();
            return;
        }

        // During enemy display phase, show one key per beat
        int beatInPhase = TurnManager.Instance.GetBeatsInCurrentPhase();

        if (EnemyRhythmController.Instance != null)
        {
            currentComboToDisplay = EnemyRhythmController.Instance.WeaknessCombo;

            if (beatInPhase < currentComboToDisplay.Length)
            {
                // Flash the current key
                char keyToShow = currentComboToDisplay[beatInPhase];
                FlashKey(keyToShow);
            }
        }
    }

    private void FlashKey(char key)
    {
        isFlashing = true;
        flashTimer = FlashDuration;
        currentKeyIndex++;

        Console.WriteLine($"[ComboUI] ?? FLASH: {key} ({currentKeyIndex}/{currentComboToDisplay.Length})");

        // TODO: Update actual UI elements here
        // For example:
        // if (key == 'W') highlightWButton.SetActive(true);
        // if (key == 'A') highlightAButton.SetActive(true);
        // etc.
    }

    private void HideComboDisplay()
    {
        if (currentKeyIndex > 0)
        {
            Console.WriteLine("[ComboUI] Hiding combo display");
            currentKeyIndex = 0;
            currentComboToDisplay = "";
            isFlashing = false;
        }
    }

    // =========================
    // Player Feedback
    // =========================
    public void ShowInputFeedback(char key, string timing)
    {
        Console.WriteLine($"[ComboUI] Player pressed {key} - {timing}");

        // TODO: Show visual feedback for player input
        // For example, briefly highlight the pressed key with color based on timing:
        // - PERFECT = Gold
        // - GOOD = Green  
        // - LATE/MISS = Red
    }

    public void ShowComboResult(bool success)
    {
        if (success)
        {
            Console.WriteLine("[ComboUI] ? COMBO SUCCESS!");
            // TODO: Show success animation
        }
        else
        {
            Console.WriteLine("[ComboUI] ? COMBO FAILED!");
            // TODO: Show failure animation
        }
    }

    // =========================
    // Public API
    // =========================
    public bool IsCurrentlyFlashing()
    {
        return isFlashing;
    }

    public string GetCurrentCombo()
    {
        return currentComboToDisplay;
    }
}