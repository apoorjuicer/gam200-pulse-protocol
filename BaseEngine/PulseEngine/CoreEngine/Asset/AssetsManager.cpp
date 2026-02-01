/******************************************************************************/
/**
 * @file        AssetsManager.cpp
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin (primary) - 90%
 * @author      Leu Jun Yong (secondary) - 10%
 * 
 * @brief       Handles loading, retrieving, and unloading of audio (FMOD)
 *              and texture (OpenGL) assets.
 *
 * @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "AssetsManager.h"
#include "Graphics/GraphicsManager.h"
#include "Audio/AudioManager.h"
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

    //mGraphics->Initialize(); // already called in constructor

    std::cout << "[AssetsManager] Initialized complete.\n";
}

void AssetsManager::Shutdown()
{
    // Registry is POD metadata; no special teardown needed
    // Registry no external dependences, constructed
    // and destroyed automatically by compiler(RAII)
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

std::unordered_map<std::string, AudioResource> const& AssetsManager::getAudioRegContainer() const {
    return mRegistry.getAudioContainer();
}

std::unordered_map<std::string, TextureResource> const& AssetsManager::getTexRegContainer() const {
    return mRegistry.getTexContainer();
}
std::unordered_map<std::string, FontResource>const& AssetsManager::getFontRegContainer() const {
    return mRegistry.getFontContainer();
}
std::unordered_map<std::string, SceneResource> const& AssetsManager::getSceneRegContainer() const {
    return mRegistry.getSceneContainer();
}
std::unordered_map<std::string, AnimationResource>const& AssetsManager::getAnimRegContainer() const {
    return mRegistry.getAnimContainer();
}

bool AssetsManager::HasAudio(std::string const& id)const {
    return getAudioRegContainer().contains(id);
}
bool AssetsManager::HasTexture(std::string const& id)const {
    return getTexRegContainer().contains(id);
}
bool AssetsManager::HasFont(std::string const& id)const {
    return getFontRegContainer().contains(id);
}
bool AssetsManager::HasScene(std::string const& id) const {
    return getSceneRegContainer().contains(id);
}
bool AssetsManager::HasAnimation(std::string const& id)const {
    return getAnimRegContainer().contains(id);
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


