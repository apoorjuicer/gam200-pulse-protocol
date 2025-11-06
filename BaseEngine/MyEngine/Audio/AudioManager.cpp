/******************************************************************************/
/**
 * @file       audio.cpp
 * @project    Pulse Protocol
 * @author     Leu Jun Yong
 * @brief      Manages FMOD audio system for BGM and SFX.
 * @copyright  Copyright (C) 2025 DigiPen Institute of Technology.
 *             Reproduction or disclosure of this file or its contents without the
 *             prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "pch/pch.h"
#include <filesystem>   // for debug path logs (safe to keep)

// If you want convenience linking on MSVC, you can uncomment this:
// #ifdef _MSC_VER
// #pragma comment(lib, "fmod_vc.lib")     // or fmodL_vc.lib for logging build
// #endif

namespace fs = std::filesystem;

// Minimal error printer (doesn't throw; just logs)
static inline void FMOD_CHECK(FMOD_RESULT r, const char* where)
{
    if (r != FMOD_OK) {
        std::cerr << "[FMOD ERROR] " << where << " -> code " << static_cast<int>(r) << std::endl;
    }
}

AudioManager::AudioManager() {}
AudioManager::~AudioManager() {}

void AudioManager::RestartBGM()
{
    if (!mSystem) return;

    // 1) stop current BGM channel if any
    if (mBGMCh) {
        mBGMCh->stop();
        mBGMCh = nullptr;
    }

    // 2) if we have a BGM sound loaded, play it again from the start
    if (mBGM) {
        FMOD_RESULT r = mSystem->playSound(mBGM, nullptr, false, &mBGMCh);
        if (r == FMOD_OK && mBGMCh) {
            mBGMCh->setVolume(1.0f);
            /*mBGMCh->setPaused(startPaused);*/
        }
    }

    // remember current pause state (not using pause as of now)
    /*mPaused = startPaused;*/
}

void AudioManager::PauseBGM()
{
    if (mBGMCh) {
        mBGMCh->setPaused(true);
    }
}

void AudioManager::releaseAll()
{
    if (!mSystem) return;

    // 1) Stop anything that might still be playing
    if (FMOD::ChannelGroup* master = nullptr;
        mSystem->getMasterChannelGroup(&master) == FMOD_OK && master) {
        master->stop();
    }

    // 2) Unload all sounds (does not require the system to be closed yet)
    Framework::AssetsManager::Instance().UnloadAllSound();

    // 3) Tear down FMOD
    mSystem->close();
    mSystem->release();
    mSystem = nullptr;
}


void AudioManager::Initialize()
{
    // Create and initialize the FMOD system
    FMOD::System_Create(&mSystem);
    if (mSystem) {
        mSystem->init(32, FMOD_INIT_NORMAL, nullptr);
    }
    else {
        std::cerr << "FMOD system initialization failed." << std::endl;
    }

    Framework::AssetsManager::Instance().SetFMODSystem(mSystem);
    Framework::AssetsManager::Instance().LoadAllSound();

    std::cout << "[AudioManager] Successfully initialized.\n";

    // Auto-start BGM here so your CoreEngine::Init() "just works".
    auto defaultBGM = Framework::AssetsManager::Instance().GetSound("gamemusic");

    if (defaultBGM) {
        // Set to loop
        defaultBGM->setMode(FMOD_LOOP_NORMAL);

        // Store the sound and get the channel
        mBGM = defaultBGM;
        FMOD_RESULT r = mSystem->playSound(defaultBGM, nullptr, false, &mBGMCh);

        if (r == FMOD_OK && mBGMCh) {
            mBGMCh->setVolume(1.0f);
            mBGMCh->setPaused(true);
            std::cout << "[AudioManager] BGM loaded (paused).\n";

            // if engine is already paused at startup, pause FMOD too
            //if (Engine && Engine->IsPaused()) {
            //    mBGMCh->setPaused(true);
            //}

        }
        else {
            std::cerr << "[AudioManager] Failed to play BGM: "
                << FMOD_ErrorString(r) << "\n";
        }
    }
}


void AudioManager::Update()
{
    if (mSystem) {
        mSystem->update();
    }

}

void AudioManager::Shutdown()
{
    releaseAll();
    std::cout << "[AudioManager] Shutdown completed\n";
}

// ---------------------- BGM controls ----------------------

void AudioManager::StopBGM()
{
    if (mBGMCh) {
        mBGMCh->stop();
        mBGMCh = nullptr;
    }
}

void AudioManager::SetBGMVolume(float volume)
{
    if (!mBGMCh) return;
    if (volume < 0.f) volume = 0.f;
    if (volume > 1.f) volume = 1.f;
    mBGMCh->setVolume(volume);
}

bool AudioManager::IsBGMPlaying(bool* outPlaying) const
{
    if (!mBGMCh) {
        if (outPlaying) *outPlaying = false;
        return false;
    }
    bool playing = false;
    mBGMCh->isPlaying(&playing);
    if (outPlaying) *outPlaying = playing;
    return playing;
}

bool AudioManager::LoadInputSFX(const std::string& path) {
    if (!mSystem) return false;
    return (mSystem->createSound(path.c_str(), FMOD_DEFAULT, 0, &mInputSFX) == FMOD_OK);
}

void AudioManager::PlaySound(const std::string& key, float volume)
{
    if (!mSystem) return;

    if (FMOD::Sound* s = Framework::AssetsManager::Instance().GetSound(key)) {
        FMOD::Channel* ch = nullptr;
        FMOD_RESULT r = mSystem->playSound(s, nullptr, false, &ch);
        if (r == FMOD_OK && ch) {
            ch->setVolume(volume);

            /*if (mPaused) {
                ch->setPaused(true);
            }*/
        }
        else {
            std::cerr << "[Audio] playSound failed: " << FMOD_ErrorString(r) << "\n";
        }
    }
    else {
        std::cerr << "[Audio] Sound not found: " << key << "\n";
    }
}


//void AudioManager::SetPaused(bool paused)
//{
//    mPaused = paused;
//
//    if (!mSystem)
//        return;
//
//    FMOD::ChannelGroup* master = nullptr;
//    if (mSystem->getMasterChannelGroup(&master) == FMOD_OK && master) {
//        master->setPaused(paused);
//    }
//
//    if (mBGMCh) {
//        mBGMCh->setPaused(paused);
//    }
//}



double AudioManager::GetMusicPosition() const
{
    if (!mBGMCh) return 0.0;

    unsigned int position = 0;
    mBGMCh->getPosition(&position, FMOD_TIMEUNIT_MS);
    return position / 1000.0;  // this converts to seconds
}

void AudioManager::StopMusic() {
    if (mBGMCh) {
        FMOD_RESULT result = mBGMCh->stop();
        if (result != FMOD_OK) {
            std::cerr << "[GAME OVER..] Stop demo music." << std::endl;
        }
        else {
            std::cout << "[Audio] Music stopped." << std::endl;
        }
    }
}
