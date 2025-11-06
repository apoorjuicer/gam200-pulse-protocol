/******************************************************************************/
/**
 * @file        AudioManager.h
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin
 *
 * @brief       Provides initialization, playback, control, and shutdown of
 *              FMOD-based audio for BGM and sound effects.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once
// #include "pch/pch.h"
#include "pch/pch_temp.h"
#include "CoreEngine/ECS/System.h"

// Forward-declare FMOD types to avoid heavy includes here
namespace FMOD
{
    class System;
    class Sound;
    class Channel;
    class ChannelGroup;
}

class AssetRegistry; // fwd

// Simple audio system for BGM playback using FMOD Core
class AudioManager : public Systems
{
public:
    AudioManager() = default;
    explicit AudioManager(AssetRegistry *registry);
    ~AudioManager();

    // non-copyable but movable
    AudioManager(AudioManager const &) = delete;
    AudioManager &operator=(AudioManager const &) = delete;
    AudioManager(AudioManager &&) noexcept = default;
    AudioManager &operator=(AudioManager &&) noexcept = default;

    // Systems interface
    // void SetRegistry(AssetRegistry* registry);       // load from audioResource
    void Initialize(); // creates FMOD system and (optionally) starts BGM

    void Update();   // calls FMOD::System::update()
    void Shutdown(); // Exit calls releaseAll releases all FMOD resources

    // BGM controls (optional for other places in engine)
    bool LoadBGM(std::string const &id); // new
    bool LoadAllBGM();
    bool PlayBGM(std::string const &id);
    void StopBGM();
    void SetBGMVolume(float volume); // 0..1
    bool IsBGMPlaying(bool *outPlaying = nullptr) const;
    void RestartBGM();

    void PauseBGM();   // Pause the current BGM
    void ResumeBGM();  // Resume paused BGM

    // Pause and Play
    void SetPaused(bool paused);
    bool IsPaused() const;

    // gets id from map and plays it at wanted volume, default volume is 1.0f
    bool LoadSFX(const std::string &id);
    bool LoadAllSFX();
    void PlaySound(const std::string &id);

    // checks how far into the song the music has played

    double GetBGMPositionSeconds() const;
    bool UnloadAudio(std::string const &id);

    // in FMod bgm and sfx behave differently

    void SetRegistry(AssetRegistry* registry);

private:
    FMOD::System *mSystem = nullptr;

    FMOD::Sound *mCurrentBGM = nullptr;     // single background music
    FMOD::Channel *mCurrentBGMCh = nullptr; // playback channel for BGM
    std::string mCurrentBGMID;
    std::unordered_map<std::string, FMOD::Sound *> mBGMCache; // single background music
    std::unordered_map<std::string, FMOD::Channel *> mBGMChannels;

    // FMOD::Sound* mInputSFX = nullptr;
    bool mPaused = true;

    void releaseAll(); // stop channel, release sound/system

    // registry
    std::unordered_map<std::string, FMOD::Sound *> mSfx;        // id -> sound
    std::unordered_map<std::string, FMOD::Channel *> mChannels; // id -> active Channel
    AssetRegistry *mRegistry = nullptr;                         // link to asset registry

    // for sliders
    FMOD::ChannelGroup *mMasterGroup = nullptr;
    FMOD::ChannelGroup *mMusicGroup = nullptr;
    FMOD::ChannelGroup *mSfxGroup = nullptr;
    float mMasterVol = 1.f, mMusicVol = 1.f, mSfxVol = 1.f;

    void applyAllVolumes();
    void applyMasterVolume();
    void applyMusicVolume();
    void applySfxVolume();
    static float clampto0and1(float v);
};
