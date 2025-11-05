/******************************************************************************/
/**
 * @file        AssetsManager.h
 * @project     Pulse Protocol
 * @author      Leu Jun Yong (primary) - 60%
 * @author      Goh Pin Kai (secondary) - 40%
 * @brief       Central manager for loading, accessing, and releasing audio (FMOD)
                and texture (OpenGL) assets.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once
#include "pch/pch_temp.h"
#include "../Resources/AssetRegistry.h"
// #include "../Graphics/GraphicsManager.h"
// #include "../Audio/AudioManager.h"

class GraphicsManager; // fwd
class AudioManager;    // fwd
// class AssetRegistry; //cant fwd because take by value
class AssetsManager
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

    std::string GetAudioPath(std::string const& id)  const;
    std::string GetTexturePath(std::string const& id) const;
    std::string GetFontPath(std::string const& id)   const;
    std::string GetScenePath(std::string const& id)   const;
    std::string GetAnimationPath(std::string const& id)   const;


    void SetAudioManager(AudioManager* audio);
    void SetGraphicsManager(GraphicsManager* graphics);
    bool PlaySFX(std::string const& id);
    bool PlayMusic(std::string const& id);

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
