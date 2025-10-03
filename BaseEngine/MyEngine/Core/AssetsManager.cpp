/*********************************************************************************************
 \file      AssetsManager.cpp
 \par
 \author    Leu Jun Yong (l.junyong@digipen.edu) 60%
            Goh Pin Kai (pinkai.g@digipen.edu) 40%


\copyright All content   2024 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/

#pragma once
#include "AssetsManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../Graphics/stb_image.h"

namespace fs = std::filesystem;

namespace Framework {


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

            bool loop = (ext == ".mp3"); // example rule: mp3 loops, wav doesn�t

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


    static GLuint upload_rgba8(int w, int h, unsigned char* data) {
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        return tex;
    }

    GLuint AssetsManager::LoadTextureFile(const std::string& path) {
        int w = 0, h = 0, n = 0;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &n, 4);
        if (!data) {
            std::cerr << "Failed to load texture: " << path
                << " reason: " << (stbi_failure_reason() ? stbi_failure_reason() : "unknown") << "\n";
            return 0;
        }
        GLuint id = upload_rgba8(w, h, data);
        stbi_image_free(data);
        return id;
    }

    GLuint AssetsManager::GetOrLoad(const std::string& path) {
        if (auto it = s_pathToID.find(path); it != s_pathToID.end()) return it->second;
        GLuint id = LoadTextureFile(path);
        if (id) {
            s_pathToID[path] = id;
            s_idToPath[id] = path;
        }
        return id;
    }

    const std::string& AssetsManager::PathFromID(GLuint texID) {
        return s_idToPath.at(texID); // throws if not found (use .find if you prefer no-throw)
    }

    void AssetsManager::Clear() {
        for (auto& kv : s_pathToID) glDeleteTextures(1, &kv.second);
        s_pathToID.clear();
        s_idToPath.clear();
    }




} // namespace Framework
