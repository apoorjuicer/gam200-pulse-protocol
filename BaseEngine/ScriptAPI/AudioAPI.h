/******************************************************************************/
/**
 * @file        AudioAPI.h
 * @project     Pulse Protocol
 * @author      Leu Jun Yong (primary) - 100%
 *
 * @brief       Declared the ScriptAPI audio functions/classes used by C# scripts.
 *
 * @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
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

        // for removing the latency completely cos System::setDSPBufferSize is for reducing it ony :/
        float GetBGMPosition();
    };
}
