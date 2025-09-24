/*********************************************************************************************
 \file      AssetsManager.cpp
 \par		Cat Feeders
 \author    Leu Jun Yong (l.junyong@digipen.edu) 70%
            Ong Zi Ming (o.ziming@digipen.edu)   30%


\copyright All content   2024 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/

 #pragma once
 #include "AssetsManager.h"  

     void AssetsManager::LoadAllSound() {

        LoaderBot yy;
        std::vector<fs::path> check =  yy.LoadGeneralType(for_Audio);
        for (auto it : check)
        {
            LoadSound(it.stem().string(), it.string());
        }
     }

     bool AssetsManager::LoadSound(const std::string& soundName, const std::string& filePath, bool loop) {
         // Check if the sound is already loaded
         if (soundMap.find(soundName) != soundMap.end()) return true;

         // Load the sound using FMOD
         FMOD::Sound* newSound = nullptr;
         FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_DEFAULT;
         FMOD_RESULT result = pSystem->createSound(filePath.c_str(), mode, nullptr, &newSound);
         if (result != FMOD_OK) return false;

         // Store the sound in the map
         soundMap[soundName] = newSound;
         std::cout << "Sound '" << soundName << "' loaded successfully from: " << filePath << std::endl;
         return true;
     }

     FMOD::Sound* AssetsManager::GetSound(const std::string& soundName) {
         auto it = soundMap.find(soundName);
         return (it != soundMap.end()) ? it->second : nullptr;
     }

     void AssetsManager::UnloadAllSound() { 
         for (auto it = soundMap.begin(); it != soundMap.end(); ++it) {
             // Release the FMOD sound object
             it->second->release();
         }
         soundMap.clear();
         std::cout << "All sounds have been unloaded." << std::endl;
     }



     void AssetsManager::LoadAllImage() { 
         LoaderBot yy;
         std::vector<fs::path> check = yy.LoadGeneralType(for_Art);
         for (auto it : check)
         {
             int frames = 1;
             if (it.string().find("SpriteSheet")!= std::string::npos)
             {
                 int size_check = (int)fs::file_size(it) / 1024;
                 frames = (size_check > 1200) ? 6 : 4;
                 frames = (size_check > 400) ? frames : 1;
             }
             loadImage(it.string().c_str(), frames);
         }
     }

     void AssetsManager::loadImage(const char* filepath, int frames) {
         // Check if the texture is already loaded
         if (textureMap.find(filepath) != textureMap.end()) {
             std::cout << "Texture already loaded: " << filepath << std::endl;
             return;
         }

         int col = 1;
         int row = 1;

         col = (frames == 6) ? 3 : ((frames == 4) ? 4 : 1);
         row = (frames == 6) ? 2 : ((frames == 4) ? 1 : 1); 


         // Extract the texture name from the file path
         std::string fullPath(filepath);
         std::string textureName = fullPath.substr(fullPath.find_last_of("\\/") + 1); // Get the file name
         textureName = textureName.substr(0, textureName.find_last_of('.'));         // Remove the extension

         // Load image using stb_image
         stbi_set_flip_vertically_on_load(true);
         int width, height, nrChannels;
         unsigned char* data = stbi_load(filepath, &width, &height, &nrChannels, 4);

         if (data) {
             GLuint textureID;
             glGenTextures(1, &textureID);  
             glBindTexture(GL_TEXTURE_2D, textureID);   
                
             // Set texture parameters
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

             // Upload the texture data based on number of channels
             if (nrChannels == 3)
                 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
             else if (nrChannels == 4)
                 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); 

             glGenerateMipmap(GL_TEXTURE_2D);
             stbi_image_free(data); 

             // Store texture data in the map, including file path
             textureMap[filepath] = { textureID, width, height, nrChannels, data, textureName, col, row, filepath };

             // assignment of textures to current objects ( background only for now) 
         }
         else {
             std::cerr << "Failed to load texture from: " << filepath << std::endl;
         }
     }
    
     void Framework::AssetsManager::TextureAssignment() {
         // Loop through all textures in the texture map
         for (const auto& [key, value] : textureMap) {
             const std::string& textureName = value.textureName; // Extract texture name
             GLuint textureID = value.textureID;                 // Extract texture ID
             int col = value.col;
             int row = value.row;

             // Loop through all transform objects and assign the correct texture
             for (auto x : transObj) {
                 if (x->spriteName == textureName && x->active) {
                     // Assign the texture ID if the sprite name matches the texture name
                     x->textureID = textureID;

                     // Assign the Col and Row for moving sprites (default will be 1 : 1 if is not moving sprite) 
                     x->columns = col;
                     x->rows = row;

                     std::cout << "Assigned texture '" << textureName
                         << "' (ID: " << textureID
                         << ") to object with spriteName '"
                         << x->spriteName << "'" << std::endl;
                 }
                 else if (!x->active)
                 {
                     x->textureID = textureMap["ScreenPng"].textureID;
                     x->spriteName = "ScreenPng";
                     x->SetActive(true);
                 }
             }
         }
     }



//     void AssetsManager::UnloadAllImage() {
//         for (auto it = textureMap.begin(); it != textureMap.end(); it++) {
//             // Store the file path for logging before erasing
//             std::string filePath = it->second.filePath;
//             // Delete the OpenGL texture
//             glDeleteTextures(1, &it->second.textureID);
//         }
//         textureMap.clear();
//         std::cout << "All textures have been unloaded." << std::endl;
//     }

//     const AssetsManager::TextureData* AssetsManager::getTextureData(const char* filepath) const {
//         auto it = textureMap.find(filepath);
//         if (it != textureMap.end()) {
//             return &it->second; // Return a pointer to the TextureData
//         }
//         else {
//             std::cerr << "Texture not found: " << filepath << std::endl;
//             return nullptr; // Return nullptr if the texture is not found
//         }
//     }

//     const AssetsManager::TextureData* AssetsManager::getTextureDataByName(const std::string& textureName) const {
//         for (const auto& pair : textureMap) {
//             if (pair.second.textureName == textureName) {
//                 return &pair.second; // Return a pointer to the TextureData
//             }
//         }
//         std::cerr << "Texture not found by name: " << textureName << std::endl;
//         return nullptr; // Return nullptr if the texture name is not found
//     }

