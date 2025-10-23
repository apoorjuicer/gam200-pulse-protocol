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


#include "pch/pch.h"

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


        // ---------- Sound management ----------
        void LoadAllSound(const std::string& folder = "../../MyEngine/Assets/Sound");
        bool LoadSound(const std::string& soundName, const std::string& filePath, bool loop);
        void UnloadAllSound();
        FMOD::Sound* GetSound(const std::string& soundName);

        // ---------- Texture management ----------
        static GLuint GetOrLoad(const std::string& path);

        static const std::string& PathFromID(GLuint texID);   // optional reverse lookup
        static void Clear();

        /*void CreateTransformsFromTextures(bool overwriteExisting = false);*/

        //void TextureAssignment();
        /*const TextureData* getTextureDataByName(const std::string& textureName) const;*/
        /*void DestroyAllTransforms();*/

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

        static GLuint LoadTextureFile(const std::string& path);

        static inline std::unordered_map<std::string, GLuint> s_pathToID{};
        static inline std::unordered_map<GLuint, std::string> s_idToPath{};
    };

} // namespace Framework
