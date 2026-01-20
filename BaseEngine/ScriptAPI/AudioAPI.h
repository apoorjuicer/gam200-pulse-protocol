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

        // Optional helpers (still useful)
        void PlaySFX(String^ name);
        void PlayBGM(String^ name);
        void StopBGM();
        void PauseBGM();
        void ResumeBGM();
        void SetBGMVolume(float volume);
    };
} 
