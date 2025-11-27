/******************************************************************************/
/**
 * @file        AssetsManager.h
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin (primary) - 90%
 * @author      Leu Jun Yong (secondary) - 10%
 * @brief       Central manager for loading, accessing, and releasing audio (FMOD)
 *              and texture (OpenGL) assets.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once
#include "pch/pch_temp.h"
#include "../MyEngine/Resources/AssetRegistry.h"

// for .net
#include "CoreEngine/Core/ImportExport.h"
#pragma warning(push)
#pragma warning(disable: 4251) // stupid warning just saying to make sure we are using same compilier type and runtime MDd. WHICH WE ARE SO WHY WARNING STILL??????????


class GraphicsManager; // fwd
class AudioManager;    // fwd
// class AssetRegistry; //cant fwd because take by value
class DLL_API AssetsManager
{
public:
    AssetsManager() = default;
    ~AssetsManager();

    void Initialize();
    void Shutdown();

    // Accessors (refs avoid copies)
    AssetRegistry &getRegistry();

    AudioManager &getAudio();
    AudioManager const &getAudio() const;

    GraphicsManager &getGraphics();
    GraphicsManager const &getGraphics() const;

    unsigned int GetOrLoadTexture(const std::string &name);

    bool   EvictTexture(std::string const& idOrPath);

    std::string GetAudioPath(std::string const& id)  const;
    std::string GetTexturePath(std::string const& id) const;
    std::string GetFontPath(std::string const& id)   const;
    std::string GetScenePath(std::string const& id)   const;
    std::string GetAnimationPath(std::string const& id)   const;


    void SetAudioManager(AudioManager* audio);
    void SetGraphicsManager(GraphicsManager* graphics);
    bool PlaySFX(std::string const& id);
    bool PlayMusic(std::string const& id);

    //get container
    std::unordered_map<std::string, AudioResource> const& getAudioRegContainer()const;
    std::unordered_map<std::string, TextureResource>const& getTexRegContainer()const;
    std::unordered_map<std::string, FontResource> const& getFontRegContainer()const;
    std::unordered_map<std::string, SceneResource> const& getSceneRegContainer()const;
    std::unordered_map<std::string, AnimationResource> const& getAnimRegContainer()const;
    //boolean check has XXX id in container
    bool HasAudio(std::string const& id)const;
    bool HasTexture(std::string const& id)const;
    bool HasFont(std::string const& id)const;
    bool HasScene(std::string const& id)const;
    bool HasAnimation(std::string const& id)const;


private:
    AssetRegistry mRegistry; // owned
    // AudioManager mAudio;
    // GraphicsManager mGraphics;
    // Asset Manager owns AudioManager and GraphicsManager
    // use unique_ptr for automatic clean up
    // single-ownership cannot be copied only moved
    // more safety
   //std::unique_ptr<AudioManager> mAudio;       // owned
   // std::unique_ptr<GraphicsManager> mGraphics; // owned
    AudioManager* mAudio = nullptr;    // non-owning
    GraphicsManager* mGraphics = nullptr;   // non-owning
};
#pragma warning(pop)