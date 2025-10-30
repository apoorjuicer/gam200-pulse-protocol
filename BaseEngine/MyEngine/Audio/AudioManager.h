/******************************************************************************/
/**
 * @file        audio.h
 * @project     Pulse Protocol
 * @author      Leu Jun Yong
 * @brief       Provides initialization, playback, control, and shutdown of
                FMOD-based audio for BGM and sound effects.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#pragma once
#include "pch/pch.h"
#include <GLFW/glfw3.h>
#include "CoreEngine/ECS/System.h"
#include "CoreEngine/ECS/SystemManager.h"
#include <string>

extern FMOD::System* mSystem;

// Forward-declare FMOD types to avoid heavy includes here
namespace FMOD {
    class System;
    class Sound;
    class Channel;
}

// Simple audio system for BGM playback using FMOD Core
class AudioManager : public Systems
{
public:
    AudioManager();
    ~AudioManager();

    // Systems interface
    void Initialize();     // creates FMOD system and (optionally) starts BGM
    void Update();         // calls FMOD::System::update()
    void Shutdown();       // releases all FMOD resources

    // BGM controls (optional for other places in engine)
    void StopBGM();
    void SetBGMVolume(float volume);                 // 0..1
    bool IsBGMPlaying(bool* outPlaying = nullptr) const;

    // Pause and Play 
    void SetPaused(bool paused);
    bool IsPaused() const { return mPaused; }

    // gets path from source file and plays it at wanted volume, default volume is 1.0f
    bool LoadInputSFX(const std::string& path);
    void PlaySound(const std::string& key, float volume);
    //void PlayInputSFX(float volume = 1.0f);

    // checks how far into the song the music has played
    double GetMusicPosition() const;
    void StopMusic();

private:
    FMOD::System* mSystem = nullptr;
    FMOD::Sound* mBGM = nullptr;
    FMOD::Channel* mBGMCh = nullptr;
    FMOD::Sound* mInputSFX = nullptr;
    bool mPaused = false;

    void releaseAll(); // stop channel, release sound/system

    // non-copyable
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
};
