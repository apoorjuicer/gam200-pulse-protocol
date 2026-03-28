/******************************************************************************/
/**
* @file        HoverableButton.cs
* @project     Pulse Protocol
* @author      Chloe Lau Rey En (primary) - 70%
               Leu Jun Yong (secondary) - 30%
* @brief       Script for hovering of buttons, and transition of scenes and showing overlay for all 4 buttons
*
* @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/

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
            // Apply saved user mix at scene start so startup and settings-open behavior match.
            audio.SetMasterVolume(UserSettingsStore.GetSliderValue("Volume", 1.0f));
            audio.SetMusicVolume(UserSettingsStore.GetSliderValue("Music", 1.0f));
            audio.SetSfxVolume(UserSettingsStore.GetSliderValue("SFX", 0.25f));
            audio.PlayBGM();
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
            // Apply saved user mix at scene start so startup and settings-open behavior match.
            audio.SetMasterVolume(UserSettingsStore.GetSliderValue("Volume", 1.0f));
            audio.SetMusicVolume(UserSettingsStore.GetSliderValue("Music", 1.0f));
            audio.SetSfxVolume(UserSettingsStore.GetSliderValue("SFX", 0.25f));

            audio.PlayBGM();
            //audio.PlayBGM("LVL 1 BGM 90 MILI OFF TEST");
            BGMManager.CurrentTrack = "Level";
            Console.WriteLine("BGM: Playing Level music");

            // Reset Conductor clock so beat tracking starts at t=0 to uh hopefully be in sync with audio if not i kms.
          
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
