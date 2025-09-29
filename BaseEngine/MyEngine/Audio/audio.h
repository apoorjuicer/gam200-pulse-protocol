// ==================================================================================
// audio.h
// Author: Leu Jun Yong
// ==================================================================================

#pragma once
#include "pch/pch.h"
#include <GLFW/glfw3.h>
#include "Core/System.h"
#include "Core/SystemManager.h"
#include <string>

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
    bool PlayBGM(const std::string& path, float volume = 0.35f);
    void StopBGM();
    void SetBGMVolume(float volume);                 // 0..1
    bool IsBGMPlaying(bool* outPlaying = nullptr) const;

private:
    FMOD::System* mSystem = nullptr;
    FMOD::Sound* mBGM = nullptr;
    FMOD::Channel* mBGMCh = nullptr;

    bool initCore();   // create + init FMOD system
    void releaseAll(); // stop channel, release sound/system

    // non-copyable
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
};
