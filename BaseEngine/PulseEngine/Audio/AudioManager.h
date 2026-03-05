/******************************************************************************/
/**
 * @file        AudioManager.h
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin (primary) - 75%
 * @author      Leu Jun Yong (secondary) - 25%
 * @brief       Provides initialization, playback, control, and shutdown of
 *              FMOD-based audio for BGM and sound effects.
 *
 * @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once
#include "pch/pch_temp.h"
#include "CoreEngine/ECS/System.h"
#include "CoreEngine/Core/ImportExport.h"

// Forward-declare FMOD types to avoid heavy includes here
namespace FMOD
{
    class System;
    class Sound;
    class Channel;
    class ChannelGroup;
}
struct AudioSource; //fwd
class AssetRegistry; // fwd

// Simple audio system for BGM playback using FMOD Core
class DLL_API AudioManager : public Systems
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
    bool IsBGMPlaying(bool *outPlaying = nullptr) const ;
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

    // checks how far into the song the music has played

    double GetBGMPositionSeconds() const;
    bool UnloadAudio(std::string const &id);

    // in FMod bgm and sfx behave differently

    void SetRegistry(AssetRegistry* registry);

    //audiosource
    void PlayFromAudioSource(AudioSource& src);
    void StopPlayingAllAudio();


    // ======================= Rhythm Timing API =======================
    // These functions provide a single source-of-truth timing model for rhythm gameplay.
    // - Song time is derived from FMOD's DSP clock (sample-accurate, stable vs frame hitches).
    // - BPM is supplied by the level/chart (FMOD does not know BPM).
    // - Optional audio offset (ms) lets designers calibrate "Perfect feels early/late" per setup.
    //
    // Units:
    // - Time values are in seconds (double) unless otherwise stated.
    // - Beat is measured in "beats" where 1.0 = one quarter note at current BPM.
    // - Steps are subdivisions of beats (e.g., stepsPerBeat=2 => 8th notes, 4 => 16th notes).

    /**
     * @brief Returns current song playback time in seconds, aligned to audio output.
     * @details Uses FMOD DSP clock and subtracts the configured audio offset (ms).
     *          This is the time gameplay should use for judging note hits.
     * @return Song time in seconds. Returns 0 if no BGM is playing.
     */
    double GetSongTimeSec() const;

    /**
     * @brief Starts/resumes the current BGM at an exact DSP clock timestamp.
     * @details The BGM channel should be created paused (playSound(..., paused=true)).
     *          This schedules it to begin at dspStart for stable sync.
     * @param dspStart DSP clock time (in mixer samples) when audio should start.
     */
    void StartBGMAtDsp(unsigned long long dspStart);

    /**
     * @brief Starts BGM using estimated output latency compensation.
     * @details Schedules BGM start at "now + estimated output latency" so song time better
     *          matches what the player actually hears (helps fix "must hit early" feel).
     */
    void StartBGMWithLatencyComp();

    /**
     * @brief Converts song time (sec) to fractional beat position.
     * @details beatFloat = songTimeSec * (BPM / 60). Example: BPM=120 => 2 beats/sec.
     * @param songTimeSec Song time in seconds (use GetSongTimeSec()).
     * @return Fractional beat index (0.0 at start, 1.0 at beat 1, etc.).
     */
    double GetBeatFloat(double songTimeSec) const;

    /**
     * @brief Returns the current integer beat index (0-based) at a given song time.
     * @param songTimeSec Song time in seconds.
     * @return Beat number (floor of GetBeatFloat()).
     */
    int GetBeatIndex(double songTimeSec) const;

    /**
     * @brief Converts an integer beat index to time (sec) from song start.
     * @param beatIndex 0-based beat index.
     * @return Time in seconds when that beat occurs.
     */
    double GetTimeOfBeatSec(int beatIndex);

    /**
     * @brief Converts song time (sec) to fractional step position within the beat grid.
     * @details stepsPerBeat=2 => 8th notes, 4 => 16th notes, etc.
     * @param songTimeSec Song time in seconds.
     * @param stepsPerBeat Subdivision count per beat (must be > 0).
     * @return Fractional step index (0.0 at start).
     */
    double GetStepFloat(double songTimeSec, int stepsPerBeat) const;

    /**
     * @brief Returns the current integer step index (0-based) at a given song time.
     * @param songTimeSec Song time in seconds.
     * @param stepsPerBeat Subdivision count per beat (must be > 0).
     * @return Step number (floor of GetStepFloat()).
     */
    int GetStepIndex(double songTimeSec, int stepsPerBeat) const;

    /**
     * @brief Converts an integer step index to time (sec) from song start.
     * @param stepIndex 0-based step index.
     * @param stepsPerBeat Subdivision count per beat (must be > 0).
     * @return Time in seconds when that step occurs.
     */
    double GetTimeOfStepSec(int stepIndex, int stepsPerBeat) const;

    /**
     * @brief Snaps a time (sec) to the nearest step on the beat grid.
     * @details Useful for quantizing note placements or aligning VFX to the beat.
     * @param timeSec Input time in seconds.
     * @param stepsPerBeat Subdivision count per beat (must be > 0).
     * @return Quantized time in seconds.
     */
    double QuantizeTimeSec(double timeSec, int stepsPerBeat) const;

    /**
     * @brief Computes signed timing error of an input vs a target note time (ms).
     * @details Negative = input early, Positive = input late.
     * @param inputTimeSec Time of player input in seconds (typically GetSongTimeSec at input).
     * @param noteTimeSec Scheduled note time in seconds (from chart).
     * @return Error in milliseconds.
     */
    double GetHitErrorMs(double inputTimeSec, double noteTimeSec) const;

    /**
     * @brief Sets a global audio calibration offset (ms).
     * @details Apply this when "Perfect feels early/late". Larger values delay judged time.
     *          Rule of thumb: if players must hit early, increase this offset.
     * @param ms Offset in milliseconds.
     */
    void SetAudioOffsetMs(double ms);

    /**
     * @brief Returns estimated output latency based on FMOD DSP buffer settings (ms).
     * @details This is an estimate (bufferLen * numBuffers / mixRate). Real device latency can vary.
     *          Useful for debug UI and initial calibration defaults.
     * @return Estimated latency in milliseconds.
     */
    double GetEstimatedOutputLatencyMs();

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
    std::unordered_map<std::string, FMOD::Sound *> mSfxCache;        // id -> sound
    std::unordered_map<std::string, FMOD::Channel *> mChannels; // id -> active Channel
    AssetRegistry *mRegistry = nullptr;                         // link to asset registry


    void ProcessAudioSources();
    void ClearAudioSourceChannelsMatching(FMOD::Channel* dead);

    //obtain in Init
    unsigned int mDSPBufferLength = 0;
    int mDSPNumBuffers = 0;
    int mMixSampleRate = 0;
    unsigned long long mSongDspStart = 0;
    bool mBgmPlaying = false;
    int mBpm = 180;
    double mAudioOffsetMs = 0.0; // calibration: +ms means player must hit later
};

/*
    FMOD caches Audio as FMOD::Sound*

    FMOD plays Audio from FMOD::Channel*
*/