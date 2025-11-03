/******************************************************************************/
/**
 * @file       AudioManager.cpp
 * @project    Pulse Protocol
 * @author     Leu Jun Yong
 * @author     Ban Kai Wei Benjamin
 * 
 * @brief      Manages FMOD audio system for BGM and SFX.
 * @copyright  Copyright (C) 2025 DigiPen Institute of Technology.
 *             Reproduction or disclosure of this file or its contents without the
 *             prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/


#include "AudioManager.h"
#include "../Resources/AssetRegistry.h" 

#pragma warning(push, 0)
#include "../MyEngine/FMOD_API/api/core/inc/fmod.hpp"
#include "../MyEngine/FMOD_API/api/core/inc/fmod_errors.h"
#pragma warning(pop)

//#include "../CoreEngine/Core/CoreEngine.h"
//#include "../CoreEngine/Asset/AssetsManager.h"
//#include <filesystem>   // for debug path logs (safe to keep)
// If you want convenience linking on MSVC, you can uncomment this:
// #ifdef _MSC_VER
// #pragma comment(lib, "fmod_vc.lib")     // or fmodL_vc.lib for logging build
// #endif

//namespace fs = std::filesystem;


    // Minimal error printer (doesn't throw; just logs)
//better to return bool for early return incase smth fails
    static inline bool  FMOD_CHECK(FMOD_RESULT r, const char* where)
    {
        if (r != FMOD_OK) {
            std::cerr << "[FMOD ERROR] " << where << " -> code "
                << static_cast<int>(r) << " ("
                //FMOD_ErrorString convert error code to string message
                << FMOD_ErrorString(r) << ")" << std::endl;
            return false;
        }
        return true;
    }
    //AudioManager::AudioManager() {}
    AudioManager::AudioManager(AssetRegistry* registry) :mRegistry{ registry } { Initialize(); }
    AudioManager::~AudioManager() { Shutdown(); }

    void AudioManager::Initialize() {
        //registry should be set before this function call
        if (!mRegistry) {
            std::cerr << "[AudioManager] Initialize called without Asset Registry.\n";
            return;
        }

        // 1. Create the FMOD system object
        FMOD_RESULT result = FMOD::System_Create(&mSystem);
        if (!FMOD_CHECK(result, "System_Create") || !mSystem) {
            return; //cant create system  return
        }

        //2. Initialize FMOD system Object
        //init(maxchannels, flags, extradriverdata)
        //allocate internal buffer for 32 voices (maxchannel)
        // channel is where we play audio clip, up to 32 sounds
        // can be played at the same time
        result = mSystem->init(32, FMOD_INIT_NORMAL, nullptr);
        if (!FMOD_CHECK(result, "System::init")) {
            return; // failed to initialize FMOD engine
        }

        std::cout << "[AudioManager] FMOD initialized successfully.\n";


        // 3) Get the master channel group
        //Master Channel Group is the top-level mixer fader for everything
        //don’t route individual sounds to it on purpose
        //route to your own submix groups
        // set mMasterGroup as MASTER CHANNEL GROUP!!
        if (mSystem->getMasterChannelGroup(&mMasterGroup) != FMOD_OK) {
            mMasterGroup = nullptr;  // fallback, prevents invalid deref
        }


        // 4) Create Music and SFX channel groups
        result = mSystem->createChannelGroup("Music", &mMusicGroup);
        FMOD_CHECK(result, "createChannelGroup(Music)");

        result = mSystem->createChannelGroup("SFX", &mSfxGroup);
        FMOD_CHECK(result, "createChannelGroup(SFX)");

        // 5) Attach Music & SFX groups to Master (top-level mixer)
        if (mMasterGroup && mMusicGroup) {
            result = mMasterGroup->addGroup(mMusicGroup);
            FMOD_CHECK(result, "master->addGroup(Music)");
        }
        if (mMasterGroup && mSfxGroup) {
            result = mMasterGroup->addGroup(mSfxGroup);
            FMOD_CHECK(result, "master->addGroup(SFX)");
        }

        //apply volume sliders
        applyAllVolumes();

        //load and play bgm
        if (!LoadBGM("bgm")) {
            std::cerr << "[AudioManager] No 'bgm' entry found in registry.\n";
        }

        std::cout << "[AudioManager] AudioManager initialization completed with mixers.\n";

    }// end init

    void AudioManager::Update() {
        if (mSystem)
            mSystem->update();
    }

    void AudioManager::Shutdown() {
        releaseAll();
    }

    void AudioManager::releaseAll()
    {
        if (!mSystem) return; //early return no system

        // 1) Stop all playback through master group

        if (FMOD::ChannelGroup* master = nullptr; //C++17 if statement with an initializer
            mSystem->getMasterChannelGroup(&master) == FMOD_OK && master) {
            FMOD_RESULT stopResult = master->stop();
            FMOD_CHECK(stopResult, "master->stop"); //debug
        }


        // 2) release any SFX loaded
        mChannels.clear(); // channels already stopped by master group call earlier

        for (auto& [id, sound] : mSfx) {
            if (sound) {
                FMOD_RESULT res = sound->release();
                FMOD_CHECK(res, ("sound->release [" + id + "]").c_str()); //debug
            }
        }
        mSfx.clear();

        // 3) release BGM
        if (mBGM) {
            FMOD_RESULT res = mBGM->release();
            FMOD_CHECK(res, "BGM->release");
            mBGM = nullptr; //set to nullptr
        }

        // 4) Close and release FMOD system
        FMOD_RESULT closeRes = mSystem->close();
        FMOD_CHECK(closeRes, "System::close");

        // mSystem->release(); invalidates all child FMOD objects internally
        // however we also release own for safety
        FMOD_RESULT relRes = mSystem->release();
        FMOD_CHECK(relRes, "System::release");

        mSystem = nullptr;

        std::cout << "[AudioManager] FMOD shutdown complete.\n";
    }

    //// ---------------------- BGM controls ----------------------
    /*     // BGM controls (optional for other places in engine)*/
    bool AudioManager::LoadBGM(std::string const& id) {
        if (!mSystem || !mRegistry) { return false; }
        // Stop and release previous BGM
        if (mBGMCh) { mBGMCh->stop(); mBGMCh = nullptr; }
        if (mBGM) { mBGM->release(); mBGM = nullptr; }

        //get bgm from registry, pointer to a read-only AudioResource
        AudioResource const* bgm = mRegistry->FindAudio(id);
        if (!bgm) {
            std::cerr << "[AudioManager] LoadBGM(): No entry found for id '" << id << "'.\n";
            return false;
        }
        else {
            //mode is playback mode, bitwise operator combines FMOD flag
            FMOD_MODE mode = FMOD_DEFAULT | (bgm->loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
            //createSound(cstring_audipath, playbackmode, extra_info parameter, output parameter sound)
            FMOD_RESULT result = mSystem->createSound(bgm->path.c_str(), mode, nullptr, &mBGM);

            if (!FMOD_CHECK(result, ("createSound(" + id + ")").c_str())) { return false; }

            // Route to Music group if available, else master
            FMOD::ChannelGroup* target = mMusicGroup ? mMusicGroup : nullptr;
            //playSound (sound, channelgroup, paused, channel)
            //passing nullptr in channel group puts it in MasterChannelGroup(top level mixer)
            result = mSystem->playSound(mBGM, target, false, &mBGMCh);
            if (!FMOD_CHECK(result, ("playSound(" + id + ")").c_str()) || !mBGMCh) { return false; }

            mBGMCh->setVolume(clampto0and1(bgm->volume));

            // Sync with engine pause (if present)
            //if (Engine && Engine->IsPaused()) {
            //    mBGMCh->setPaused(true);
            //    std::cout << "[AudioManager] BGM loaded and paused (engine paused).\n";
            //}
            //else {
            //    std::cout << "[AudioManager] BGM playing: " << bgm->path << "\n";
            //}
        }

        return true;
    }
    void AudioManager::StopBGM() {
        if (mBGMCh) {
            FMOD_RESULT result = mBGMCh->stop();
            FMOD_CHECK(result, "StopBGM(): ");
            mBGMCh = nullptr;
        }
        std::cout << "[AudioManager] BGM stopped.\n";
    }
    void AudioManager::SetBGMVolume(float volume) {
        if (mBGMCh) {
            mBGMCh->setVolume(clampto0and1(volume));
        }
    }
    bool AudioManager::IsBGMPlaying(bool* outPlaying) const {
        if (!mBGMCh) {
            if (outPlaying) { *outPlaying = false; }
            return false;
        }

        bool playing = false;
        FMOD_RESULT result = mBGMCh->isPlaying(&playing);
        FMOD_CHECK(result, "IsBGMPlaying");

        if (outPlaying) { *outPlaying = playing; }
        return playing;
    }

    //pause
    void AudioManager::SetPaused(bool paused) {
        if (mPaused == paused) return;
        mPaused = paused; // should be set to false to play

        // Pause or unpause all audio groups (global behavior)
        // control master channel group set it to paused value
        if (mMasterGroup) {
            FMOD_RESULT result = mMasterGroup->setPaused(paused);
            FMOD_CHECK(result, paused ? "mMasterGroup->setPaused(true)" : "mMasterGroup->setPaused(false)");
        }
        std::cout << "[AudioManager] " << (paused ? "Paused all audio." : "Resumed audio.") << "\n";
    }
    bool AudioManager::IsPaused() const { return mPaused; }

    /*Sound Effects*/
    bool AudioManager::LoadSFX(std::string const& id) {
        if (!mSystem || !mRegistry) return false;

        // Avoid reloading if already loaded
        if (mSfx.find(id) != mSfx.end()) { return true; }


        // Get sound resource metadata from asset registry (audioResource)
        AudioResource const* sfx = mRegistry->FindAudio(id);
        if (!sfx) {
            std::cerr << "[AudioManager] LoadSFX(): No entry found for id : '" << id << "'.\n";
            return false;
        }

        // Create sound (reusable sound, not looping)
        FMOD_MODE mode = FMOD_DEFAULT | (sfx->loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
        FMOD::Sound* sound = nullptr; //destroyed after function call
        FMOD_RESULT result = mSystem->createSound(sfx->path.c_str(), mode, nullptr, &sound);
        if (!FMOD_CHECK(result, ("createSound(" + id + ")").c_str()) || !sound) { return false; }

        // Store in map
        mSfx[id] = sound; //assigned temp ptr to sfx map 
        std::cout << "[AudioManager] Loaded SFX '" << id << "' (" << sfx->path << ")\n";
        return true;

    }
    void AudioManager::PlaySound(std::string const& id) {
        if (!mSystem) return;

        // Find preloaded sound
        auto it = mSfx.find(id);
        if (it == mSfx.end()) {
            std::cerr << "[AudioManager] PlaySound(): Sound '" << id << "' not loaded. Attempting to load.\n";
            if (!LoadSFX(id)) { return; }
            it = mSfx.find(id);
            if (it == mSfx.end()) { return; }
        }


        //get sound value
        FMOD::Sound* sound = it->second; //same as mSfx[id]
        FMOD::Channel* channel = nullptr;

        // Play sound through SFX channel group
        FMOD_RESULT result = mSystem->playSound(sound, mSfxGroup, false, &channel);
        if (!FMOD_CHECK(result, ("playSound(" + id + ")").c_str()) || !channel) {
            return;
        }

        // Apply SFX volume
        // internally, calculates effective_gain = master * sfxGroup * channel
        if (auto* res = mRegistry->FindAudio(id)) {
            channel->setVolume(clampto0and1(res->volume));
        }



        // Track the channel ( for stopping later)
        mChannels[id] = channel;

        std::cout << "[AudioManager] Playing SFX: " << id << "\n";
    }


    /**/

    double AudioManager::GetBGMPositionSeconds() const {
        if (!mBGMCh) return 0.0;

        unsigned int ms = 0; // position in milliseconds
        FMOD_RESULT result = mBGMCh->getPosition(&ms, FMOD_TIMEUNIT_MS);
        FMOD_CHECK(result, "getPosition(BGM)");

        return static_cast<double>(ms) / 1000.0; // convert to seconds
    }
    bool  AudioManager::UnloadAudio(std::string const& id) {
        auto it = mSfx.find(id);
        if (it != mSfx.end()) {

            // Stop any channels currently playing this SFX
            auto chIt = mChannels.find(id);
            if (chIt != mChannels.end()) {
                FMOD::Channel* ch = chIt->second;
                if (ch) {
                    FMOD_RESULT stopRes = ch->stop();
                    FMOD_CHECK(stopRes, ("UnloadAudio stop channel '" + id + "'").c_str());
                }
                mChannels.erase(chIt);
            }


            //release sound
            if (it->second) {
                FMOD_RESULT res = it->second->release();
                FMOD_CHECK(res, ("releaseSound(" + id + ")").c_str());
            }
            mSfx.erase(it);
            std::cout << "[AudioManager] Unloaded SFX: " << id << "\n";
            return true;
        }

        // Otherwise, check if it’s the currently loaded BGM
        if (mBGM && id == "bgm") {
            if (mBGMCh) { mBGMCh->stop(); mBGMCh = nullptr; }
            FMOD_RESULT res = mBGM->release();
            FMOD_CHECK(res, "releaseSound(bgm)");
            mBGM = nullptr;
            std::cout << "[AudioManager] Unloaded BGM.\n";
            return true;
        }

        std::cerr << "[AudioManager] UnloadAudio(): No audio with id '" << id << "' found.\n";
        return false;
    }


    /*For Audio Slider*/

    void AudioManager::applyAllVolumes() { applyMasterVolume(); applyMusicVolume(); applySfxVolume(); }
    void AudioManager::applyMasterVolume() { if (mMasterGroup) { mMasterGroup->setVolume(clampto0and1(mMasterVol)); } }
    void AudioManager::applyMusicVolume() { if (mMusicGroup) mMusicGroup->setVolume(clampto0and1(mMusicVol)); }
    void AudioManager::applySfxVolume() { if (mSfxGroup)   mSfxGroup->setVolume(clampto0and1(mSfxVol)); }
    float AudioManager::clampto0and1(float v) {
        return (v < 0) ? 0 :
            ((v > 1) ? 1 : v);
    }





   




 /*
    Every FMOD function returns an FMOD_RESULT
    init FMOD : FMOD::System_Create(*system_var);

 */


