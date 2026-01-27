#pragma once
using namespace System;

namespace ScriptAPI
{
    public value struct AudioComponent
    {
        int EntityID;

        AudioComponent(int id);

        // Plays whatever is set on the entity's AudioSource in the Inspector
        void Play();

        // Existing helpers
        void PlaySFX(String^ name);
        void PlayBGM(String^ name);
        void StopBGM();
        void PauseBGM();
        void ResumeBGM();
        void SetBGMVolume(float volume);

        void SetMasterVolume(float volume01);
        void SetMusicVolume(float volume01);
        void SetSfxVolume(float volume01);

        // (optional) readback if you want debug / show numbers
        float GetMasterVolume();
        float GetMusicVolume();
        float GetSfxVolume();
    };
}
