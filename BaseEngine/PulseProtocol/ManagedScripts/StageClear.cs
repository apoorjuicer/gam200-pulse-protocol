using System;
using ScriptAPI;

public class StageClearElement : Script
{
    // =========================
    // GLOBAL STAGE CLEAR STATE
    // =========================
    public static bool IsCleared { get; private set; } = false;

    public static void StageClear()
    {
        NavigationButtons.SetStageClearPaused(true);
        NavigationButtons.ReportLevelCleared(Scene.GetCurrentScene());
        IsCleared = true;
        Console.WriteLine("[StageClear]  StageClear() called");
    }

    public static void ResetStageClear()
    {
        NavigationButtons.SetStageClearPaused(false);
        IsCleared = false;
        Console.WriteLine("[StageClear]  ResetStageClear()");
    }

    // =========================
    // PER-ENTITY SETTINGS
    // =========================
    public bool HideOnStart = true;

    private bool initialized = false;
    private bool shown = false;

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            return;
        }

        // When StageClear is triggered, show THIS entity
        if (!shown && IsCleared)
        {
            Show();
        }
    }

    private void Initialize()
    {
        if (HideOnStart && !IsCleared)
        {
            var tr = GetTransform();
            tr.IsVisible = false;
        }

        shown = IsCleared;
        if (shown) Show();

        initialized = true;
    }

    private void Show()
    {
        var tr = GetTransform();
        tr.IsVisible = true;
        shown = true;
        Console.WriteLine("[StageClear] Showing Stage Clear UI element");
    }
}
