using System;
using ScriptAPI;

// ============================================
// USE THIS FOR: MainMenu, Settings
// ============================================
public class GeneralBGM : Script
{
    private bool started = false;

    public override void Update()
    {
        if (!started)
        {
            AudioComponent audio = GetAudio();
            // Don't set volume here - the MusicKnob slider handles it
            audio.PlayBGM("BGM_GENERAL");
            BGMManager.CurrentTrack = "General";
            Console.WriteLine("BGM: Playing General music");
            
            started = true;
        }
    }
}

// ============================================
// USE THIS FOR: Level1
// ============================================
public class LevelBGM : Script
{
    private bool started = false;

    public override void Update()
    {
        if (!started)
        {
            AudioComponent audio = GetAudio();
            // Don't set volume here - the MusicKnob slider handles it
            audio.PlayBGM("BGM_L1");
            BGMManager.CurrentTrack = "Level";
            Console.WriteLine("BGM: Playing Level music");

            // Reset Conductor clock so beat tracking starts at t=0
            // in sync with the audio, not with when Conductor initialised.
            Conductor.Instance?.ResetClock();

            started = true;
        }
    }
}

// ============================================
// STATIC MANAGER - Tracks state across scenes
// ============================================
public static class BGMManager
{
    public static string CurrentTrack = "";
}