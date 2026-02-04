using System;
using System.Collections.Generic;
using ScriptAPI;

/// <summary>
/// Displays the player's combo input on screen in real-time
/// Shows what they need to type and what they've typed so far
/// </summary>
public class ComboInputUI : Script
{
    // =========================
    // Global Access (Singleton)
    // =========================
    public static ComboInputUI Instance { get; private set; }

    // =========================
    // Display Settings
    // =========================
    public float TargetTextX = 0f;
    public float TargetTextY = 200f;
    public float InputTextX = 0f;
    public float InputTextY = 100f;

    public float TextScale = 2.0f;

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private string targetCombo = "";
    private string currentInput = "";
    private string lastPrintedState = "";  // FIXED: Moved to instance variable

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

        // Update display based on ComboSystem state
        if (ComboSystem.Instance != null)
        {
            targetCombo = ComboSystem.Instance.GetExpectedCombo();
            currentInput = ComboSystem.Instance.GetCurrentComboString();
        }

        // Update text display
        UpdateDisplay();
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        // Set singleton
        if (Instance == null)
        {
            Instance = this;
        }
        else if (Instance != this)
        {
            Console.WriteLine("[ComboInputUI] WARNING: Multiple ComboInputUI instances detected!");
            return;
        }

        Console.WriteLine("[ComboInputUI] Initialized");
        initialized = true;
    }

    // =========================
    // Display Update
    // =========================
    private void UpdateDisplay()
    {
        // This is a placeholder implementation
        // You'll need to actually render text using your engine's text rendering system

        // For now, we just log to console when combo changes
        // In a real implementation, you would:
        // 1. Get TextComponent or similar from your engine
        // 2. Set the text content
        // 3. Position it at TargetTextX/Y and InputTextX/Y

        // Example of what you'd do if your engine had TextComponent:
        /*
        TextComponent targetText = GetTextComponent("TargetText");
        targetText.SetText($"Target: {targetCombo}");
        targetText.SetPosition(TargetTextX, TargetTextY);
        targetText.SetScale(TextScale);

        TextComponent inputText = GetTextComponent("InputText");
        inputText.SetText($"Input: {currentInput}");
        inputText.SetPosition(InputTextX, InputTextY);
        inputText.SetScale(TextScale);

        // Color the input based on correctness
        if (currentInput.Length > 0)
        {
            bool isCorrectSoFar = targetCombo.StartsWith(currentInput);
            inputText.SetColor(isCorrectSoFar ? Color.Green : Color.Red);
        }
        */

        // For debugging, print to console when combo state changes
        string currentState = $"{currentInput}/{targetCombo}";

        if (currentState != lastPrintedState)
        {
            Console.WriteLine($"[ComboInputUI] Display: {currentInput} / {targetCombo}");
            lastPrintedState = currentState;
        }
    }

    // =========================
    // Public API
    // =========================
    public void SetPosition(float targetX, float targetY, float inputX, float inputY)
    {
        TargetTextX = targetX;
        TargetTextY = targetY;
        InputTextX = inputX;
        InputTextY = inputY;
    }

    public void SetTextScale(float scale)
    {
        TextScale = scale;
    }
}