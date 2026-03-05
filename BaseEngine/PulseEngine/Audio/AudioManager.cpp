/******************************************************************************/
/**
 * @file       AudioManager.cpp
 * @project    Pulse Protocol
 * @author     Ban Kai Wei Benjamin (primary) - 75%
 * @author     Leu Jun Yong (secondary) - 25%
 * 
 * @brief      Manages FMOD audio system for BGM and SFX.
 * 
 * @copyright  Copyright (C) 2026 DigiPen Institute of Technology.
 *             Reproduction or disclosure of this file or its contents without the
 *             prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "AudioManager.h"
#include "Resources/AssetRegistry.h"
#include "AudioSource.h"
#include "CoreEngine/ECS/Coordinator.h"
#include "CoreEngine/ECS/Types.h"
#include "CoreEngine/Core/CoreEngine.h"
#pragma warning(push, 0)
#include "FMOD_API/api/core/inc/fmod.hpp"
#include "FMOD_API/api/core/inc/fmod_errors.h"

#pragma warning(pop)

// #include "../CoreEngine/Core/CoreEngine.h"
// #include "../CoreEngine/Asset/AssetsManager.h"
// #include <filesystem>   // for debug path logs (safe to keep)
//  If you want convenience linking on MSVC, you can uncomment this:
//  #ifdef _MSC_VER
//  #pragma comment(lib, "fmod_vc.lib")     // or fmodL_vc.lib for logging build
//  #endif

// namespace fs = std::filesystem;

extern CoreEngine* Engine;

// Minimal error printer (doesn't throw; just logs)
// better to return bool for early return incase smth fails
static inline bool FMOD_CHECK(FMOD_RESULT r, const char *where)
{
    if (r != FMOD_OK)
    {
        std::cerr << "[FMOD ERROR] " << where << " -> code "
                  << static_cast<int>(r) << " ("
                  // FMOD_ErrorString convert error code to string message
                  << FMOD_ErrorString(r) << ")" << std::endl;
        return false;
    }
    return true;
}
// AudioManager::AudioManager() {}
AudioManager::AudioManager(AssetRegistry *registry) : mRegistry{registry}
{
    // Initialize();
}
AudioManager::~AudioManager()
{
}

void AudioManager::SetRegistry(AssetRegistry *registry)
{
    mRegistry = registry;
}

void AudioManager::RestartBGM()
{
    if (!mSystem)
    {
        return;
    }

    // 1) stop current BGM channel if any
    if (mCurrentBGMCh)
    {
        mCurrentBGMCh->stop();
        mCurrentBGMCh = nullptr;
    }

    // 2) if we have a BGM sound loaded, play it again from the start
    if (mCurrentBGM)
    {
        FMOD_RESULT r = mSystem->playSound(mCurrentBGM, mMusicGroup, false, &mCurrentBGMCh);
        if (r != FMOD_OK || !mCurrentBGMCh)
        {
            return;
        }

        mCurrentBGMCh->setVolume(1.0f);
        // mCurrentBGMCh->setPaused(startPaused);
    }

    // remember current pause state (not using pause as of now)
    /*mPaused = startPaused;*/
}

void AudioManager::Initialize()
{
    // registry should be set before this function call
    if (!mRegistry)
    {
        std::cerr << "[AudioManager] Initialize called without Asset Registry.\n";
        return;
    }

    // 1. Create the FMOD system object
    FMOD_RESULT result = FMOD::System_Create(&mSystem);
    if (!FMOD_CHECK(result, "System_Create") || !mSystem)
    {
        return; // cant create system  return
    }

    // 2. Initialize FMOD system Object
    // init(maxchannels, flags, extradriverdata)
    // allocate internal buffer for 32 voices (maxchannel)
    //  channel is where we play audio clip, up to 32 sounds
    //  can be played at the same time
    result = mSystem->init(32, FMOD_INIT_NORMAL, nullptr);
    if (!FMOD_CHECK(result, "System::init"))
    {
        return; // failed to initialize FMOD engine
    }

    std::cout << "[AudioManager] FMOD initialized successfully.\n";

    // 3) Get the master channel group
    // Master Channel Group is the top-level mixer fader for everything
    // dont route individual sounds to it on purpose
    // route to your own submix groups
    // set mMasterGroup as MASTER CHANNEL GROUP!!
    if (mSystem->getMasterChannelGroup(&mMasterGroup) != FMOD_OK)
    {
        mMasterGroup = nullptr; // fallback, prevents invalid deref
    }

    // 4) Create Music and SFX channel groups
    result = mSystem->createChannelGroup("Music", &mMusicGroup);
    FMOD_CHECK(result, "createChannelGroup(Music)");

    result = mSystem->createChannelGroup("SFX", &mSfxGroup);
    FMOD_CHECK(result, "createChannelGroup(SFX)");

    // 5) Attach Music & SFX groups to Master (top-level mixer)
    if (mMasterGroup && mMusicGroup)
    {
        result = mMasterGroup->addGroup(mMusicGroup);
        FMOD_CHECK(result, "master->addGroup(Music)");
    }
    if (mMasterGroup && mSfxGroup)
    {
        result = mMasterGroup->addGroup(mSfxGroup);
        FMOD_CHECK(result, "master->addGroup(SFX)");
    }

    // apply volume sliders
    applyAllVolumes();

    // load and play bgm
    // if (!LoadBGM("bgm")) {
    //     std::cerr << "[AudioManager] No 'bgm' entry found in registry.\n";
    // }
    // else {
    //     std::cout << "[AudioManager] 'bgm' loaded from registry.\n";
    // }

    // if (!LoadSFX("beat_placeholder")) {
    //     std::cerr << "[AudioManager] No 'beat_placeholder' entry found in registry.\n";
    // }
    // else {
    //     std::cout << "[AudioManager] 'beat_placeholder' loaded from registry.\n";
    // }

    if (!LoadAllBGM())
    {
        std::cerr << "[AudioManager] load all BGM failed.\n";
    }
    else
    {
        std::cout << "[AudioManager] load all BGM success.\n";
    }

    if (!LoadAllSFX())
    {
        std::cerr << "[AudioManager] load all SFX failed.\n";
    }
    else
    {
        std::cout << "[AudioManager] load all SFX success.\n";
    }

    std::string tf = IsBGMPlaying() ? "true" : "false";
    //std::cout << "[AudioManager] Playing bgm? " << tf << "\n";
    // PlaySound("beat_placeholder");
    std::cout << "[AudioManager] AudioManager initialization completed with mixers.\n";

} // end init

void AudioManager::Update()
{
   // std::cout << "Processing AudioManager Update\n";
    if (!mSystem)
        return;
    // ecs, audio component
    // Audio Component -> Audio Source
    // if (!Engine || !Engine->IsPlaying()) {
    //     if (!Engine || !Engine->IsPlaying()) return;
    // }
    // else {
    //     ProcessAudioSources(); // 
    //     mSystem->update();
    // }
    mSystem->update(); 
    if (!Engine || !Engine->IsPlaying()) return;
    ProcessAudioSources();
}

void AudioManager::Shutdown()
{
    releaseAll();
}

void AudioManager::releaseAll()
{
    if (!mSystem)
        return; // early return no system

    // 1) Stop all playback through master group

    if (FMOD::ChannelGroup *master = nullptr; // C++17 if statement with an initializer
        mSystem->getMasterChannelGroup(&master) == FMOD_OK && master)
    {
        FMOD_RESULT stopResult = master->stop();
        FMOD_CHECK(stopResult, "master->stop"); // debug
    }

    // 2) release any SFX loaded
    mChannels.clear(); // channels already stopped by master group call earlier
    mBGMChannels.clear();
    mCurrentBGMCh = nullptr;


    for (auto &[id, sound] : mSfxCache)
    {
        if (sound)
        {
            FMOD_RESULT res = sound->release();
            FMOD_CHECK(res, ("sound->release SFX:[" + id + "]").c_str()); // debug
        }
    }
    mSfxCache.clear();

    // 3) release BGM
    // if (mCurrentBGM)
    // {
    //     FMOD_RESULT res = mCurrentBGM->release();
    //     FMOD_CHECK(res, "BGM->release");
    //     mCurrentBGM = nullptr; // set to nullptr
    // }

    // 4) Release BGM sounds (avoid double-release if mCurrentBGM is also in the cache)
    for (auto& [id, sound] : mBGMCache)
    {
        if (sound)
        {
            FMOD_RESULT res = sound->release();
            FMOD_CHECK(res, ("sound->release BGM:[" + id + "]").c_str());
        }
    }
    mBGMCache.clear();
    //reset ptrs again
    mCurrentBGM = nullptr;
    mCurrentBGMID.clear();

    // mBGMChannels.clear();
    // mCurrentBGMCh = nullptr;

    // 4) Close and release FMOD system
    FMOD_RESULT closeRes = mSystem->close();
    FMOD_CHECK(closeRes, "System::close");

    // mSystem->release(); invalidates all child FMOD objects internally
    // however we also release own for safety
    FMOD_RESULT relRes = mSystem->release();
    FMOD_CHECK(relRes, "System::release");
    mMasterGroup = mMusicGroup = mSfxGroup = nullptr;
    mSystem = nullptr;
    
    std::cout << "[AudioManager] FMOD shutdown complete.\n";
}

//// ---------------------- BGM controls ----------------------
/*     // BGM controls (optional for other places in engine)*/
bool AudioManager::LoadBGM(std::string const &id)
{
    if (!mSystem || !mRegistry)
    {
        return false;
    }
    // Stop and release previous BGM
    //  if (mBGMCh) { mBGMCh->stop(); mBGMCh = nullptr; }
    // if (mBGM) { mBGM->release(); mBGM = nullptr; }

    if (mBGMCache.count(id))
        return true; // already cached

    // get bgm from registry, pointer to a read-only AudioResource
    AudioResource const *bgm = mRegistry->FindAudio(id);
    if (!bgm)
    {
        std::cerr << "[AudioManager] LoadBGM(): No entry found for id '" << id << "'.\n";
        return false;
    }
    else
    {
        // mode is playback mode, bitwise operator combines FMOD flag
        FMOD_MODE mode = FMOD_DEFAULT | (bgm->loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
        // createSound(cstring_audipath, playbackmode, extra_info parameter, output parameter sound)
        // FMOD_RESULT result = mSystem->createSound(bgm->path.c_str(), mode, nullptr, &mBGM);

        FMOD::Sound *s = nullptr;
        //FMOD_RESULT result = mSystem->createStream(bgm->path.c_str(), mode, nullptr, &s);
        std::string resolvedPath = mRegistry->GetAudioItemPath(id);
        FMOD_RESULT result = mSystem->createStream(resolvedPath.c_str(), mode, nullptr, &s);

        if (!FMOD_CHECK(result, ("createStream(" + id + ")").c_str()))
        {
            return false;
        }

        // Route to Music group if available, else master
        // FMOD::ChannelGroup* target = mMusicGroup ? mMusicGroup : nullptr;
        ////playSound (sound, channelgroup, paused, channel)
        ////passing nullptr in channel group puts it in MasterChannelGroup(top level mixer)
        // result = mSystem->playSound(mCurrentBGM, target, false, &mCurrentBGMCh);
        // if (!FMOD_CHECK(result, ("playSound(" + id + ")").c_str()) || !mCurrentBGMCh) { return false; }

        // mCurrentBGMCh->setVolume(clampto0and1(bgm->volume));

        //// Sync with engine pause (if present)
        // if (Engine && Engine->IsPaused()) {
        //     mBGMCh->setPaused(true);
        //     std::cout << "[AudioManager] BGM loaded and paused (engine paused).\n";
        // }
        // else {
        //     std::cout << "[AudioManager] BGM playing: " << bgm->path << "\n";
        // }

        mBGMCache[id] = s;  // map OWNS the sound
        //Load != Play!!
        //mCurrentBGMID = id; // optional: mark as loaded current
        //mCurrentBGM = s;    // pointer to current bgm
    }

    return true;
}

bool AudioManager::LoadAllBGM()
{
    if (!mSystem || !mRegistry)
        return false;

    bool all_ok = true;
    for (auto const &[id, ar] : mRegistry->getAudioContainer())
    {
        if (ar.audiotype == "BGM")
        {
            if (!LoadBGM(id))
                all_ok = false;
        }
    }
    return all_ok;
}

bool AudioManager::PlayBGM(const std::string &id)
{
    if (!mSystem)
        return false;
    if (!mBGMCache.count(id) && !LoadBGM(id))
        return false;

    // stop previous current channel (no crossfade here; add one later if want)
    if (mCurrentBGMCh)
    {
        mCurrentBGMCh->stop();
        mCurrentBGMCh = nullptr;
    }

    FMOD::ChannelGroup *target = mMusicGroup ? mMusicGroup : nullptr;
    FMOD::Channel *ch = nullptr;
    FMOD_RESULT r = mSystem->playSound(mBGMCache[id], target, false, &ch);
    if (!FMOD_CHECK(r, ("playSound(" + id + ")").c_str()) || !ch)
        return false;

    // per-track volume 
    float vol = 1.0f;
    if (const auto* ar = mRegistry->FindAudio(id)) {
        vol = ar->volume;
    }
       
    ch->setVolume(clampto0and1(vol * mMusicVol));

    // book-keeping
    mBGMChannels[id] = ch;
    mCurrentBGMID = id;
    mCurrentBGM = mBGMCache[id];
    mCurrentBGMCh = ch;
    return true;
}

void AudioManager::StopBGM()
{
    if (mCurrentBGMCh)
    {
        FMOD_RESULT result = mCurrentBGMCh->stop();
        FMOD_CHECK(result, "StopBGM(): ");
        mCurrentBGMCh = nullptr;
    }
    std::cout << "[AudioManager] BGM stopped.\n";
}
void AudioManager::SetBGMVolume(float volume)
{
    if (mCurrentBGMCh)
    {
        mCurrentBGMCh->setVolume(clampto0and1(volume));
    }
}
bool AudioManager::IsBGMPlaying(bool *outPlaying) const
{
    if (!mCurrentBGMCh)
    {
        if (outPlaying)
        {
            *outPlaying = false;
        }
        return false;
    }

    bool playing = false;
    FMOD_RESULT result = mCurrentBGMCh->isPlaying(&playing);
    FMOD_CHECK(result, "IsBGMPlaying");

    if (outPlaying)
    {
        *outPlaying = playing;
    }
    return playing;
}

// pause
void AudioManager::SetPaused(bool paused)
{
    if (mPaused == paused)
        return;
    mPaused = paused; // should be set to false to play

    // Pause or unpause all audio groups (global behavior)
    // control master channel group set it to paused value
    if (mMasterGroup)
    {
        FMOD_RESULT result = mMasterGroup->setPaused(paused);
        FMOD_CHECK(result, paused ? "mMasterGroup->setPaused(true)" : "mMasterGroup->setPaused(false)");
    }
    //std::cout << "[AudioManager] " << (paused ? "Paused all audio." : "Resumed audio.") << "\n";
}
bool AudioManager::IsPaused() const { return mPaused; }

/*Sound Effects*/

bool AudioManager::LoadSFX(std::string const &id)
{
    if (!mSystem || !mRegistry)
        return false;

    // Avoid reloading if already loaded
    if (mSfxCache.find(id) != mSfxCache.end())
    {
        return true;
    }

    // Get sound resource metadata from asset registry (audioResource)
    AudioResource const *soundefx = mRegistry->FindAudio(id);
    if (!soundefx)
    {
        std::cerr << "[AudioManager] LoadSFX(): No entry found for id : '" << id << "'.\n";
        return false;
    }
    if (id != soundefx->audio_id)
    {
        return false;
    }

    {
        // Create sound (reusable sound, not looping)
        FMOD_MODE mode = FMOD_DEFAULT | (soundefx->loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
        FMOD::Sound *sound = nullptr; // destroyed after function call
        
        //FMOD_RESULT result = mSystem->createSound(soundefx->path.c_str(), mode, nullptr, &sound);
        std::string resolvedPath = mRegistry->GetAudioItemPath(id);
        FMOD_RESULT result = mSystem->createSound(resolvedPath.c_str(), mode, nullptr, &sound);
        
        if (!FMOD_CHECK(result, ("createSound(" + id + ")").c_str()) || !sound)
        {
            return false;
        }
        // Store in map
        mSfxCache[soundefx->audio_id] = sound; // assigned temp ptr to sfx map
    }

    std::cout << "[AudioManager] Loaded SFX '" << id << "' (" << soundefx->path << ")\n";
    return true;
}

bool AudioManager::LoadAllSFX()
{
    if (!mSystem || !mRegistry)
        return false;

    bool all_ok = true;
    // however you expose the table:
    auto const &table = mRegistry->getAudioContainer(); // unordered_map<string, AudioResource>
    for (auto const &[id, ar] : table)
    {
        if (ar.audiotype == "SFX")
        {
            if (!LoadSFX(id))
            {
                all_ok = false;
            }
        }
    }
    return all_ok;
}

void AudioManager::PlaySound(std::string const &id)
{
    if (!mSystem)
        return;

    // Find preloaded sound
    auto it = mSfxCache.find(id);
    if (it == mSfxCache.end())
    {
        std::cerr << "[AudioManager] PlaySound(): Sound '" << id << "' not loaded. Attempting to load.\n";
        if (!LoadSFX(id))
        {
            return;
        }
        it = mSfxCache.find(id);
        if (it == mSfxCache.end())
        {
            return;
        }
    }

    // get sound value
    FMOD::Sound *sound = it->second; // same as mSfx[id]
    FMOD::Channel *channel = nullptr;

    // Play sound through SFX channel group
    FMOD_RESULT result = mSystem->playSound(sound, mSfxGroup, false, &channel);
    if (!FMOD_CHECK(result, ("playSound(" + id + ")").c_str()) || !channel)
    {
        return;
    }

    // Apply SFX volume
    // internally, calculates effective_gain = master * sfxGroup * channel
    if (auto *res = mRegistry->FindAudio(id))
    {
        channel->setVolume(clampto0and1(res->volume));
    }

    // Track the channel ( for stopping later)
    mChannels[id] = channel;

    //std::cout << "[AudioManager] Playing SFX: " << id << "\n";
}

/**/

double AudioManager::GetBGMPositionSeconds() const
{
    if (!mCurrentBGMCh)
        return 0.0;

    unsigned int ms = 0; // position in milliseconds
    FMOD_RESULT result = mCurrentBGMCh->getPosition(&ms, FMOD_TIMEUNIT_MS);
    FMOD_CHECK(result, "getPosition(BGM)");

    return static_cast<double>(ms) / 1000.0; // convert to seconds
}
bool AudioManager::UnloadAudio(std::string const &id)
{
    // -------- SFX --------
    auto sfxIt  = mSfxCache.find(id);
    if (sfxIt  != mSfxCache.end())
    {

        // Stop any channels currently playing this SFX
        auto chIt = mChannels.find(id);
        if (chIt != mChannels.end())
        {
            FMOD::Channel *ch = chIt->second;
            if (ch)
            {
                FMOD_RESULT stopRes = ch->stop();
                FMOD_CHECK(stopRes, ("UnloadAudio stop channel '" + id + "'").c_str());
                ClearAudioSourceChannelsMatching(ch);
            }
            mChannels.erase(chIt);
        }

        // release sound
        if (sfxIt ->second)
        {
            FMOD_RESULT res = sfxIt ->second->release();
            FMOD_CHECK(res, ("releaseSound(" + id + ")").c_str());
        }
        mSfxCache.erase(sfxIt );
        std::cout << "[AudioManager] Unloaded SFX: " << id << "\n";
        return true;
    }

    // ---------------- BGM ----------------
    auto bgmIt = mBGMCache.find(id);
    if (bgmIt != mBGMCache.end())
    {
        // Stop tracked channel for this BGM id (if any)
        auto bgmChIt = mBGMChannels.find(id);
        if (bgmChIt != mBGMChannels.end())
        {
            FMOD::Channel* ch = bgmChIt->second;
            if (ch)
            {
                FMOD_RESULT stopRes = ch->stop();
                FMOD_CHECK(stopRes, ("UnloadAudio stop BGM channel '" + id + "'").c_str());
                ClearAudioSourceChannelsMatching(ch);
            }
            mBGMChannels.erase(bgmChIt);
        }

        // If this is the current BGM, clear current pointers too
        if (mCurrentBGMID == id)
        {
            if (mCurrentBGMCh)
            {
                FMOD_RESULT stopRes = mCurrentBGMCh->stop();
                FMOD_CHECK(stopRes, ("UnloadAudio stop current BGM '" + id + "'").c_str());
                ClearAudioSourceChannelsMatching(mCurrentBGMCh);
            }
            mCurrentBGMCh = nullptr;
            mCurrentBGM = nullptr;      // alias only
            mCurrentBGMID.clear();
        }

        // Release the OWNED sound from cache
        if (bgmIt->second)
        {
            FMOD_RESULT res = bgmIt->second->release();
            FMOD_CHECK(res, ("releaseSound(BGM " + id + ")").c_str());
        }

        mBGMCache.erase(bgmIt);
        std::cout << "[AudioManager] Unloaded BGM: " << id << "\n";
        return true;
    }

    std::cerr << "[AudioManager] UnloadAudio(): No audio with id '" << id << "' found.\n";
    return false;
}

/*For Audio Slider*/

void AudioManager::applyAllVolumes()
{
    applyMasterVolume();
    applyMusicVolume();
    applySfxVolume();
}
void AudioManager::applyMasterVolume()
{
    if (mMasterGroup)
    {
        mMasterGroup->setVolume(clampto0and1(mMasterVol));
    }
}
void AudioManager::applyMusicVolume()
{
    if (mMusicGroup)
        mMusicGroup->setVolume(clampto0and1(mMusicVol));
}
void AudioManager::applySfxVolume()
{
    if (mSfxGroup)
        mSfxGroup->setVolume(clampto0and1(mSfxVol));
}
float AudioManager::clampto0and1(float v)
{
    return (v < 0) ? 0 : ((v > 1) ? 1 : v);
}

void AudioManager::PauseBGM()
{
    if (mCurrentBGMCh)
    {
        mCurrentBGMCh->setPaused(true);
        std::cout << "[AudioManager] BGM Paused\n";
    }
}

void AudioManager::ResumeBGM()
{
    if (mCurrentBGMCh)
    {
        mCurrentBGMCh->setPaused(false);
        std::cout << "[AudioManager] BGM Resumed\n";
    }
}
//====================================================

void AudioManager::PlayFromAudioSource(AudioSource& src) {

    //std::cout << "playing from audio source\n";
    if (!mSystem || !mRegistry) {
        return; //return if no FMOD or registry
    }
     
    if (src.audio_id.empty()) {
        return; // return if no KEY. audio id
    }
       
    AudioResource const* res = mRegistry->FindAudio(src.audio_id);
    if (!res)
    {
        std::cerr << "[AudioManager] PlayFromSource: No AudioResource for id '"
            << src.audio_id << "'\n";
        return;
    }
    // --------------------------------------------------------
    // BGM case 
    // --------------------------------------------------------
    if (res->audiotype == "BGM")
    {
        // Route to BGM pipeline, use existing bgm in registry
        if(!PlayBGM(src.audio_id))
        {
            std::cerr << "[AudioManager] PlayFromSource: PlayBGM failed for '"
                << src.audio_id << "'\n";
            return;
        }

        // apply per-source volume override + per-track
        auto it = mBGMChannels.find(src.audio_id);
        if (it != mBGMChannels.end())
        {
            FMOD::Channel* ch = it->second;
            src.channel = ch;   //playOnAwake guard 

            if (ch)
            {
                float baseVol = res->volume;              // volume from registry
                float finalVol = clampto0and1(baseVol * mMusicVol * src.volume);
                ch->setVolume(finalVol);
                ch->setMode(src.loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
            }
        }        
        return; 
    }
    // --------------------------------------------------------
    // SFX case 
    // --------------------------------------------------------
    if (res->audiotype == "SFX") //redundant statement but more readable
    {
        PlaySound(src.audio_id);

        // Get the channel we just played for this id
        auto it = mChannels.find(src.audio_id);
        if (it != mChannels.end())
        {
            FMOD::Channel* ch = it->second;
            src.channel = ch;

            if (ch)
            {
                // Start from registry volume and apply per-source and SFX group
                float baseVol = res->volume;  // base SFX volume from JSON
                float finalVol = clampto0and1(baseVol * mSfxVol * src.volume);
                ch->setVolume(finalVol);
                ch->setMode(src.loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
            }
        }
    }
}

//private
void AudioManager::ProcessAudioSources()
{
    //std::cout << "Processing Audio componenets\n";
    auto g_coordinator = Coordinator::GetInstance();

    if (!g_coordinator || !mRegistry) return;

    // IMPT!!!! ONLY PROCESS IF GAME IS ACTUALLY PLAYING
    //if (!Engine || !Engine->IsPlaying()) return;

    for (Entity entity = 0; entity < MaxEntity; ++entity)
    {
        if (!g_coordinator->HasComponent<AudioSource>(entity))
            continue;

        auto& src = g_coordinator->GetComponent<AudioSource>(entity);

        // -------- PLAY ON AWAKE (PLAY ONLY ONCE) --------
        if (src.playOnAwake && src.channel == nullptr && !src.audio_id.empty())
        {

            PlayFromAudioSource(src);  // sets src.channel for SFX *and* BGM
            //src.playOnAwake = false;   // prevent replay every frame
        }

        if (src.audio_id.empty())
            continue;

        AudioResource const* res = mRegistry->FindAudio(src.audio_id);
        if (!res) continue;
        // -------- ACTIVE CHANNEL (SFX or BGM) --------
        if (src.channel)
        {
            bool playing = false;
            src.channel->isPlaying(&playing);

            if (!playing) // Channel has stopped; clean up all references to it.
            {
                //dead becomes invalid w hen fmod channel stopped
                //fmod internally destroys or recycles channel
                //when channel->isPlaying(&playing), playing == false
                FMOD::Channel* dead = src.channel;  //hold dead as oldptr
                src.channel = nullptr; //clear Component side ptr

                if (res->audiotype == "SFX")
                {
                    // SFX cleanup: remove from mChannels if pointing to this channel
                    auto sfxIt = mChannels.find(src.audio_id);
                    if (sfxIt != mChannels.end() && sfxIt->second == dead)
                        mChannels.erase(sfxIt);
                }
                // BGM cleanup: only if this resource is BGM
                if (res->audiotype == "BGM")
                {
                    // remove from mBGMChannels if it points to the same channel
                    auto it = mBGMChannels.find(src.audio_id);
                    if (it != mBGMChannels.end() && it->second == dead)
                        mBGMChannels.erase(it);

                    // clear mCurrentBGMCh if it was this channel
                    if (mCurrentBGMCh == dead)
                        mCurrentBGMCh = nullptr;
                }

                continue; // go to next entity
            }
            else
            {
                float base = res->volume;
                float user = std::clamp(src.volume, 0.0f, 1.0f);

                // Pick the correct group volume
                float groupVol = 1.0f;
                if (res->audiotype == "BGM")
                    groupVol = mMusicVol;
                else if (res->audiotype == "SFX")
                    groupVol = mSfxVol;

                float final = clampto0and1(base * groupVol * user);

                src.channel->setVolume(final);
                src.channel->setMode(src.loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);

            }
            continue; // go to next entity
        }   
    }
}

void  AudioManager::ClearAudioSourceChannelsMatching(FMOD::Channel* dead)
{
    if (!dead) return;

    auto g = Coordinator::GetInstance();
    if (!g) return;

    for (Entity e = 0; e < MaxEntity; ++e)
    {
        if (!g->HasComponent<AudioSource>(e))
            continue;

        auto& src = g->GetComponent<AudioSource>(e);

        if (src.channel == dead)
            src.channel = nullptr;
    }
}

void AudioManager::StopPlayingAllAudio() {

    if(!mSystem)
        return;

    if (mMasterGroup)
    {
        mMasterGroup->stop();  // FMOD will stop all child groups & channels
        return;
    }

    // if mMasterGroup==nullptr:ask FMOD again for the master group and stop that
    FMOD::ChannelGroup* master = nullptr;
    if (mSystem->getMasterChannelGroup(&master) == FMOD_OK && master)
    {
        master->stop();
    }
}

/*
   Every FMOD function returns an FMOD_RESULT
   init FMOD : FMOD::System_Create(*system_var);

   fmod internally destroys or recycles channel
   when channel->isPlaying(&playing), playing == false
*/

