/*********************************************************************************************
 \file      AssetsManager.h
 \par
 \author    Leu Jun Yong (l.junyong@digipen.edu)

\copyright All content   2024 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/

#pragma once

#include "fmod.hpp"
#include <GL/glew.h>
#include "../Audio/Audio.h"
#include "../Graphics/GraphicsManager.h"
#include "../Graphics/glapp.h"
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <string>
#include <fmod_errors.h>
#include <filesystem>


namespace Framework {

    class AssetsManager {
    public:
        // Data structure for texture metadata
        struct TextureData {
            GLuint textureID;
            int width, height;
            int nrChannels;
            unsigned char* data;
            std::string textureName;
 /*           int col;
            int row;*/
            std::string filePath;
        };

        // Singleton instance
        static AssetsManager& Instance() {
            static AssetsManager instance;
            return instance;
        }

        // ---------- Assets management ----------
        void LoadAllAssets();
        void UnloadAllAssets();

        // ---------- Sound management ----------
        void LoadAllSound();
        bool LoadSound(const std::string& soundName, const std::string& filePath, bool loop);
        void UnloadAllSound();
        FMOD::Sound* GetSound(const std::string& soundName);

        // ---------- Texture management ----------
        void LoadAllImage();
        void loadImage(const char* filepath /*,int frames*/);
        void UnloadAllImage();
        //void TextureAssignment();
        const TextureData* getTextureData(const char* filepath) const;
        const TextureData* getTextureDataByName(const std::string& textureName) const;

        // ---------- Getters ----------
        const std::unordered_map<std::string, FMOD::Sound*>& GetSoundMap() const {
            return soundMap;
        }
        const std::unordered_map<std::string, TextureData>& GetTextureMap() const {
            return textureMap;
        }

        // Allow FMOD system to be injected after initialization
        void SetFMODSystem(FMOD::System* system) { mSystem = system; }

    private:
        AssetsManager() = default;
        ~AssetsManager() = default;
        AssetsManager(const AssetsManager&) = delete;
        AssetsManager& operator=(const AssetsManager&) = delete;

        // Data structures
        std::unordered_map<std::string, FMOD::Sound*> soundMap;
        std::unordered_map<std::string, TextureData> textureMap;

        // FMOD system instance pointer
        FMOD::System* mSystem = nullptr;
    };

} // namespace Framework
