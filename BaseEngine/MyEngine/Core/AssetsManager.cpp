/*********************************************************************************************
 \file      AssetsManager.cpp
 \par		
 \author    Leu Jun Yong (l.junyong@digipen.edu) 
        


\copyright All content   2024 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/

#pragma once
#include "AssetsManager.h"

namespace fs = std::filesystem;

namespace Framework {

    void AssetsManager::LoadAllAssets() {
        LoadAllSound();
        LoadAllImage();
    }


    void AssetsManager::LoadAllSound()
    {
        const fs::path root = "../../MyEngine/Assets/Sound";

        if (!fs::exists(root)) {
            std::cerr << "[Audio] Sound folder not found: " << root << std::endl;
            return;
        }

        for (auto const& entry : fs::recursive_directory_iterator(root)) {
            if (!entry.is_regular_file()) continue;

            fs::path filePath = entry.path();
            std::string ext = filePath.extension().string();
            for (auto& c : ext) c = (char)tolower(c);

            if (ext != ".mp3" && ext != ".wav")
                continue;

            // Key: relative path without extension, lowercased
            std::string soundKey = filePath.stem().string();
            for (auto& c : soundKey) c = (char)tolower(c);

            bool loop = (ext == ".mp3"); // example rule: mp3 loops, wav doesn’t

            if (LoadSound(soundKey, filePath.string(), loop)) {
                std::cout << "[Audio] Loaded: " << soundKey
                    << " (" << filePath.string() << ")"
                    << " loop=" << (loop ? "true" : "false") << std::endl;
            }
            else {
                std::cout << "[Audio] FAILED: " << filePath.string() << std::endl;
            }
        }
    }

    bool AssetsManager::LoadSound(const std::string& soundName, const std::string& filePath, bool loop) {
        // Check if the sound is already loaded
        if (soundMap.find(soundName) != soundMap.end()) return true;

        // Load the sound using FMOD
        FMOD::Sound* newSound = nullptr;
        FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_DEFAULT;
        FMOD_RESULT result = mSystem->createSound(filePath.c_str(), mode, nullptr, &newSound);
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
        // Stop anything that may still be playing (prevents stale handles).
        if (mSystem) {
            FMOD::ChannelGroup* master = nullptr;
            if (mSystem->getMasterChannelGroup(&master) == FMOD_OK && master) {
                master->stop();
            }
        }

        size_t released = 0;
        for (auto& kv : soundMap) {
            FMOD::Sound* s = kv.second;
            if (!s) continue;

            FMOD_RESULT r = s->release();
            if (r != FMOD_OK) {
                std::cerr << "[Audio] release failed for '" << kv.first
                    << "': " << FMOD_ErrorString(r) << "\n";
            }
            else {
                ++released;
            }
        }
        soundMap.clear();
        std::cout << "[Audio] Unloaded " << released << " sound(s).\n";
    }


    void AssetsManager::LoadAllImage()
    {
        const fs::path root = "../../MyEngine/Assets/Image";

        if (!fs::exists(root)) {
            std::cerr << "[Image] Folder not found: " << root.string() << "\n";
            return;
        }

        size_t count = 0;

        for (const auto& entry : fs::recursive_directory_iterator(root)) {
            if (!entry.is_regular_file()) continue;

            const fs::path& p = entry.path();
            std::string ext = p.extension().string();
            for (auto& c : ext) c = (char)std::tolower((unsigned char)c);
            if (ext != ".png") continue;

            const std::string pathStr = p.generic_string(); // forward slashes
            loadImage(pathStr.c_str());
            ++count;
        }

        std::cout << "[Image] Scan complete. Attempted to load " << count
            << " .png file(s) from " << root.generic_string() << "\n";
    }

    void AssetsManager::loadImage(const char* filepath /*,int frames*/) {
        // Check if the texture is already loaded
        if (textureMap.find(filepath) != textureMap.end()) {
            std::cout << "Texture already loaded: " << filepath << std::endl;
            return;
        }

 /*       int col = 1;
        int row = 1;

        col = (frames == 6) ? 3 : ((frames == 4) ? 4 : 1);
        row = (frames == 6) ? 2 : ((frames == 4) ? 1 : 1); */


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
            textureMap[filepath] = { textureID, width, height, nrChannels, data, textureName, /*col, row,*/ filepath };

            // assignment of textures to current objects ( background only for now) 
        }
        else {
            std::cerr << "Failed to load texture from: " << filepath << std::endl;
        }
    }
    
    //void Framework::AssetsManager::TextureAssignment() {
    //    // For each transform, find a texture with the same name and assign its ID
    //    for (auto& x : transObj) {
    //        if (!x) continue;
    //        // If your spriteName/file names differ in case, consider lowercasing both strings
    //        const AssetsManager::TextureData* td = getTextureDataByName(x->spriteName);
    //        if (td) {
    //            x->textureID = td->textureID;
    //            // If you later store animation grids, set them here:
    //            // x->columns = td->col;
    //            // x->rows    = td->row;

    //            std::cout << "Assigned texture '" << td->textureName
    //                << "' (ID: " << td->textureID
    //                << ") to object with spriteName '" << x->spriteName << "'\n";
    //        }
    //        else {
    //            // Optional: set a fallback (e.g., "ScreenPng") if no matching texture name found
    //            // auto* fallback = getTextureDataByName("ScreenPng");
    //            // if (fallback) { x->textureID = fallback->textureID; x->spriteName = "ScreenPng"; }
    //            std::cerr << "No texture found for spriteName '" << x->spriteName << "'\n";
    //        }
    //    }
    //}

    //void AssetsManager::CreateTransformsFromTextures(bool overwriteExisting) {
    //    // Build a quick set of existing names/ids to avoid duplicates
    //    std::unordered_set<std::string> existingNames;
    //    std::unordered_set<GLuint> existingIDs;
    //    existingNames.reserve(transObj.size());
    //    existingIDs.reserve(transObj.size());

    //    for (auto* t : transObj) {
    //        if (!t) continue;
    //        existingNames.insert(t->spriteName);
    //        existingIDs.insert(t->textureID);
    //    }

    //    for (const auto& [key, td] : textureMap) {
    //        const std::string& name = td.textureName;
    //        const GLuint id = td.textureID;

    //        // If a transform with the same name already exists
    //        if (!overwriteExisting && (existingNames.count(name) || existingIDs.count(id))) {
    //            // If you want to *update* existing ones when overwriteExisting=true:
    //            if (overwriteExisting) {
    //                for (auto* t : transObj) {
    //                    if (!t) continue;
    //                    if (t->spriteName == name || t->textureID == id) {
    //                        t->spriteName = name;     // keep name consistent
    //                        t->textureID = id;       // update to current texture ID
    //                        // Optional if you store grids in TextureData:
    //                        // t->columns = td.col;
    //                        // t->rows    = td.row;
    //                    }
    //                }
    //            }
    //            continue;
    //        }

    //        // Create a new Transform for this texture
    //        auto* t = new Transform();        // make sure this matches your ctor
    //        t->spriteName = name;
    //        t->textureID = id;
    //        // Optional defaults (uncomment if you have these fields/methods):
    //        // t->SetActive(true);
    //        // t->columns = td.col;
    //        // t->rows    = td.row;
    //        // t->position = {0.0f, 0.0f};  // or any default you like

    //        transObj.push_back(t);
    //        existingNames.insert(name);
    //        existingIDs.insert(id);

    //        std::cout << "[Image] Created Transform for '"
    //            << name << "' (ID: " << id << ")\n";
    //    }
    //}

    //void AssetsManager::DestroyAllTransforms()
    //{
    //    for (auto* t : transObj)
    //    {
    //        delete t;   // free each Transform
    //    }
    //    transObj.clear();

    //    std::cout << "[Cleanup] All transforms destroyed\n";
    //}

    
    void AssetsManager::UnloadAllImage() {
        // Must be called while a valid OpenGL context is current!
        // If you shut the window/GL first, glDeleteTextures is undefined.

        // Collect texture IDs first (avoid iterator pitfalls)
        std::vector<GLuint> ids;
        ids.reserve(textureMap.size());
        for (const auto& kv : textureMap) {
            if (kv.second.textureID != 0) ids.push_back(kv.second.textureID);
        }

        if (!ids.empty()) {
            // Unbind to be safe
            glBindTexture(GL_TEXTURE_2D, 0);
            glDeleteTextures(static_cast<GLsizei>(ids.size()), ids.data());
        }

        textureMap.clear();
        std::cout << "[Image] Unloaded " << ids.size() << " texture(s).\n";
    }

    const AssetsManager::TextureData* AssetsManager::getTextureDataByName(const std::string& textureName) const {
        for (const auto& pair : textureMap) {
            if (pair.second.textureName == textureName) {
                return &pair.second; // Return a pointer to the TextureData
            }
        }
        std::cerr << "Texture not found by name: " << textureName << std::endl;
        return nullptr; // Return nullptr if the texture name is not found
    }

} // namespace Framework
