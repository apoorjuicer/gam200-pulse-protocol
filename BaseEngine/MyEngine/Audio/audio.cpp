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
    const char* defaultBGM = "../../MyEngine/Assets/Music/Background/GameMusic.mp3";
    PlayBGM(defaultBGM, 0.0f);

    // preloads sfx sound for beat
    const char* defaultInputSFX = "../../MyEngine/Assets/SFX/Rhythm_Feedback/beat_placeholder.wav";
    if (!LoadInputSFX(defaultInputSFX)) {
        std::cerr << "[AudioManager] Failed to load default input SFX: " << defaultInputSFX << "\n";
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

bool AudioManager::PlayBGM(const std::string& path, float volume)
{
    if (!initCore()) return false;

#ifdef _DEBUG
    try {
        std::cerr << "[AudioManager] Try BGM: " << path
            << "  exists? " << (fs::exists(path) ? "YES" : "NO") << "\n";
    }
    catch (...) {}
#endif

    // Stop & release old BGM if needed
    StopBGM();
    if (mBGM) { mBGM->release(); mBGM = nullptr; }

    // Stream + loop (2D)
    FMOD_MODE mode = FMOD_DEFAULT | FMOD_CREATESTREAM | FMOD_LOOP_NORMAL | FMOD_2D;

    FMOD_RESULT r = mSystem->createSound(path.c_str(), mode, nullptr, &mBGM);
    FMOD_CHECK(r, "createSound(BGM)");
    if (r != FMOD_OK || !mBGM) {
        std::cerr << "[AudioManager] Failed to load BGM (bad path or unsupported format?).\n";
        return false;
    }

    r = mSystem->playSound(mBGM, nullptr, false, &mBGMCh);
    FMOD_CHECK(r, "playSound(BGM)");
    if (r != FMOD_OK || !mBGMCh) {
        std::cerr << "[AudioManager] Failed to play BGM.\n";
        return false;
    }

    SetBGMVolume(volume);
    std::cout << "[AudioManager] BGM started: " << path << " (vol=" << volume << ")\n";
    return true;
}

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
void AudioManager::PlayInputSFX(float volume) {
    if (!mSystem || !mInputSFX) return;

    FMOD::Channel* channel = nullptr;
    FMOD_RESULT result = mSystem->playSound(mInputSFX, 0, false, &channel);
    if (result == FMOD_OK && channel) {
        channel->setVolume(volume); // applies only to this SFX
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
