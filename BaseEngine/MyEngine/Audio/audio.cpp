// ==================================================================================
// audio.cpp
// Author: Leu Jun Yong
// ==================================================================================

#include "audio.h"
#include <iostream>
#include <filesystem>   // for debug path logs (safe to keep)
#include "fmod.hpp"

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
AudioManager::~AudioManager() { Shutdown(); }

bool AudioManager::initCore()
{
    if (mSystem) return true; // already initialized

    FMOD_RESULT r = FMOD::System_Create(&mSystem);
    FMOD_CHECK(r, "System_Create");
    if (r != FMOD_OK || !mSystem) return false;

    unsigned int version = 0;
    r = mSystem->getVersion(&version);
    FMOD_CHECK(r, "getVersion");

    // 32 channels is plenty for now
    r = mSystem->init(32, FMOD_INIT_NORMAL, nullptr);
    FMOD_CHECK(r, "init");
    if (r != FMOD_OK) return false;

    Framework::AssetsManager::Instance().SetFMODSystem(mSystem); 
    Framework::AssetsManager::Instance().LoadAllSound();

#ifdef _DEBUG
    try {
        std::cerr << "[AudioManager] FMOD initialized. CWD = " << fs::current_path().string() << "\n";
    }
    catch (...) {}
#endif
    return true;
}

void AudioManager::releaseAll()
{
    StopBGM();

    if (mBGM) {
        mBGM->release();
        mBGM = nullptr;
    }
    if (mSystem) {
        mSystem->close();
        mSystem->release();
        mSystem = nullptr;
    }
}

void AudioManager::Initialize()
{
    if (!initCore()) {
        std::cerr << "[AudioManager] Init FAILED (check fmod.dll next to the exe and correct architecture).\n";
        return;
    }

    std::cout << "[AudioManager] Successfully initialized.\n";

   

    // Auto-start BGM here so your CoreEngine::Init() "just works".
    // Make sure this path exists relative to your exe working directory:
    //   build\MyEngine\Assets\MainMenuMusic.wav
    // If your assets live elsewhere, update this string accordingly.
    //const char* defaultBGM = "../../MyEngine/Assets/Music/Background/GameMusic.mp3";
    //PlayBGM(defaultBGM, 0.30f);



    auto defaultBGM = Framework::AssetsManager::Instance().GetSound("gamemusic");

    if (defaultBGM) { 
        mSystem->playSound(defaultBGM, nullptr, false, nullptr); 
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

void AudioManager::PlaySound(const std::string& key, float volume) {
    if (!mSystem) return;

    if (FMOD::Sound* s = Framework::AssetsManager::Instance().GetSound(key)) {
        FMOD::Channel* ch = nullptr;
        FMOD_RESULT r = mSystem->playSound(s, nullptr, false, &ch);
        if (r == FMOD_OK && ch) ch->setVolume(volume);
        else std::cerr << "[Audio] playSound failed: " << FMOD_ErrorString(r) << "\n";
    }
    else {
        std::cerr << "[Audio] Sound not found: " << key << "\n";
    }
}

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
