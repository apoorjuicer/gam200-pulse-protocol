/******************************************************************************/
/**
 * @file        AssetsManager.cpp
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin (primary)
 * @author      Leu Jun Yong (secondary)
 * @author      Goh Pin Kai (secondary)
 * @brief       Handles loading, retrieving, and unloading of audio (FMOD)
                and texture (OpenGL) assets.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "AssetsManager.h"
#include "../Graphics/GraphicsManager.h"
#include "../Audio/AudioManager.h"
// namespace Framework {



AssetsManager::~AssetsManager()
{
    Shutdown();
}

void AssetsManager::Initialize()
{
    // 1) Load manifests first (registry has metadata)
    mRegistry.LoadAudioManifest();
    mRegistry.LoadTextureManifest();
    mRegistry.LoadFontManifest();
    mRegistry.LoadSceneManifest();
    mRegistry.LoadAnimationManifest();

    //// 2) Bring up subsystems (pass registry where needed)
    //mAudio = std::make_unique<AudioManager>(&mRegistry);
    //mAudio->Initialize(); // already called in constructor

    //mGraphics = std::make_unique<GraphicsManager>(&mRegistry);
    //mGraphics->Initialize(); // already called in constructor

    std::cout << "[AssetsManager] Initialized complete.\n";
}

void AssetsManager::Shutdown()
{
    // Registry is POD metadata; no special teardown needed
    // Registry no external dependences, constructed
    // and destroyed automatically by compiler(RAII)
    // reset unique_ptr calls dtor for graphics and audio
    //if (mGraphics)
    //{ /*mGraphics->Shutdown();*/
    //    mGraphics.reset();
    //}
    //if (mAudio)
    //{ /*mAudio->Shutdown();  */
    //    mAudio.reset();
    //}

    std::cout << "[AssetsManager] Shutdown complete.\n";
}

AssetRegistry &AssetsManager::getRegistry() { return mRegistry; }
AudioManager &AssetsManager::getAudio() { return *mAudio; }
AudioManager const &AssetsManager::getAudio() const { return *mAudio; }

GraphicsManager &AssetsManager::getGraphics() { return *mGraphics; }
GraphicsManager const &AssetsManager::getGraphics() const { return *mGraphics; }

unsigned int AssetsManager::GetOrLoadTexture(const std::string &name)
{
    return mGraphics ? mGraphics->GetOrLoad(name) : 0u;
}

bool AssetsManager::EvictTexture(std::string const& idOrPath)
{
    return mGraphics ? mGraphics->EvictTexture(idOrPath) : false;
}


std::string AssetsManager::GetAudioPath(std::string const& id)  const {
    return mRegistry.GetAudioItemPath(id);
}
std::string AssetsManager::GetTexturePath(std::string const& id) const {
    return mRegistry.GetTextureItemPath(id);
}
std::string AssetsManager::GetFontPath(std::string const& id)   const {
    return mRegistry.GetFontItemPath(id);
}
std::string AssetsManager::GetScenePath(std::string const& id)   const {
    return mRegistry.GetSceneItemPath(id);
}
std::string AssetsManager::GetAnimationPath(std::string const& id)   const {
    return mRegistry.GetAnimationItemPath(id);
}


bool AssetsManager::PlaySFX(std::string const& id) {
    if (mAudio) {
        mAudio->PlaySound(id);  
        return true;                 
    }
    return false;                 
}

bool AssetsManager::PlayMusic(std::string const& id) {
    if (mAudio) {
        mAudio->PlayBGM(id);
        return true;
    }
    return false;
}

void AssetsManager::SetAudioManager(AudioManager* audio)
{
    mAudio = audio;
}

void AssetsManager::SetGraphicsManager(GraphicsManager* graphics)
{
    mGraphics = graphics;
}


