/*********************************************************************************************
 \file      AssetsManager.h
 \par		Cat Feeders
 \author    Leu Jun Yong (l.junyong@digipen.edu) 70%
            Ong Zi Ming (o.ziming@digipen.edu)   30%


\copyright All content   2024 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/


// #pragma once

// #include "fmod.hpp"
// #include <GL/glew.h> // Ensure OpenGL types like GLuint are available
// #include "../Engine/Sound.h"
// #include "../Graphics/glappSystem.h"
// #include "../Components/Transform.h"
// #include "../Graphics/glapp.h"
// #include "Pipeline.h"
// #include "../Component_System.h"
// #include "../../Engine/Precompiled.h"

// namespace Framework {

//     class AssetsManager {
//     public:
//         // Data structure for texture metadata
//         struct TextureData {
//             GLuint textureID;          // OpenGL texture ID
//             int width, height;         // Texture dimensions
//             int nrChannels;            // Number of color channels
//             unsigned char* data;       // Pointer to raw image data
//             std::string textureName;   // Name of the texture (e.g., "cat", "background")
//             int col;
//             int row;                // row and collum for the frames for moving sprites
//             std::string filePath;      // Full file path of the texture
//         };

//         // Singleton instance
//         static AssetsManager& Instance() {
//             static AssetsManager instance; // Guaranteed to be destroyed and instantiated on first use
//             return instance;
//         }

//         // Sound management
//         void LoadAllSound();
//         bool LoadSound(const std::string& soundName, const std::string& filePath, bool loop = false);
//         void UnloadAllSound();
//         FMOD::Sound* GetSound(const std::string& soundName);

//         // Texture management
//         void LoadAllImage();
//         void loadImage(const char* filepath, int frames);
//         void UnloadAllImage();  
//         void TextureAssignment();
//         const TextureData* getTextureData(const char* filepath) const;
//         const TextureData* getTextureDataByName(const std::string& textureName) const;

//         // Getter methods for external access
//         const std::unordered_map<std::string, FMOD::Sound*>& GetSoundMap() const {
//             return soundMap;
//         }
//         const std::unordered_map<std::string, TextureData>& GetTextureMap() const {
//             return textureMap;
//         }

//     private:
//         // Prevent external instantiation and copying
//         AssetsManager() = default; // Default constructor
//         ~AssetsManager() = default; // Default destructor
//         AssetsManager(const AssetsManager&) = delete; // Delete copy constructor
//         AssetsManager& operator=(const AssetsManager&) = delete; // Delete copy assignment

//         // Data structures for managing sounds and textures
//         std::unordered_map<std::string, FMOD::Sound*> soundMap; // Map for managing sounds
//         std::unordered_map<std::string, TextureData> textureMap; // Map for managing textures
//     };

// } // namespace Framework

// // Extern declarations for global access (if needed)
// extern std::unordered_map<std::string, FMOD::Sound*> soundMap;
// extern std::unordered_map<std::string, Framework::AssetsManager::TextureData> textureMap;
