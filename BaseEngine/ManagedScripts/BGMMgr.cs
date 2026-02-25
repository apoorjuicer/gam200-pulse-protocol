using System;
using ScriptAPI;

// ============================================
// USE THIS FOR: MainMenu, Settings
// ============================================
public class GeneralBGM : Script
{
    private bool started = false;
    private bool wasPaused = false;

    public override void Update()
    {
        AudioComponent audio = GetAudio();

        if (!started)
        {
            // Don't set volume here - the MusicKnob slider handles it
            audio.PlayBGM("BGM_GENERAL");
            BGMManager.CurrentTrack = "General";
            Console.WriteLine("BGM: Playing General music");
            started = true;
        }

        // Pause / Resume BGM based on game pause state
        bool isPaused = Application.IsPaused(); // If your API uses another getter, swap this line

        if (isPaused && !wasPaused)
        {
            audio.PauseBGM();   // If your API uses another name, replace here
            Console.WriteLine("BGM: Paused");
        }
        else if (!isPaused && wasPaused)
        {
            audio.ResumeBGM();  // If your API uses another name, replace here
            Console.WriteLine("BGM: Resumed");
        }

        wasPaused = isPaused;
    }
}

// ============================================
// USE THIS FOR: Level1
// ============================================
public class LevelBGM : Script
{
    private bool started = false;
    private bool wasPaused = false;

    public override void Update()
    {
        // Wait until gameplay is ready before starting level BGM
        if (!GameStart.Ready) return;

        AudioComponent audio = GetAudio();

        if (!started)
        {
            // Don't set volume here - the MusicKnob slider handles it
            audio.PlayBGM("BGM_L1");
            BGMManager.CurrentTrack = "Level";
            Console.WriteLine("BGM: Playing Level music");

            // Reset Conductor clock so beat tracking starts at t=0
            Conductor.Instance?.ResetClock();

            started = true;
        }

        // Pause / Resume BGM based on game pause state
        bool isPaused = Application.IsPaused(); // If your API uses another getter, swap this line

        if (isPaused && !wasPaused)
        {
            audio.PauseBGM();   // If your API uses another name, replace here
            Console.WriteLine("BGM: Paused");
        }
        else if (!isPaused && wasPaused)
        {
            audio.ResumeBGM();  // If your API uses another name, replace here
            Console.WriteLine("BGM: Resumed");
        }

        wasPaused = isPaused;
    }
}

// ============================================
// STATIC MANAGER - Tracks state across scenes
// ============================================
public static class BGMManager
{
    public static string CurrentTrack = "";
}