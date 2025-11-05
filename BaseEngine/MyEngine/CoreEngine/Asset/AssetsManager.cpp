    /******************************************************************************/
    /**
     * @file        AssetsManager.cpp
     * @project     Pulse Protocol
     * @author      Leu Jun Yong (primary) - 60%
     * @author      Goh Pin Kai (secondary) - 40%
     * @brief       Handles loading, retrieving, and unloading of audio (FMOD) 
                    and texture (OpenGL) assets.
     * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
     *              Reproduction or disclosure of this file or its contents without the
     *              prior written consent of DigiPen Institute of Technology is prohibited.
     */
     /******************************************************************************/

    #pragma once
    #include "AssetsManager.h"
    #define STB_IMAGE_IMPLEMENTATION
    #include "../Graphics/stb_image.h"

    namespace fs = std::filesystem;

    namespace Framework {


        void AssetsManager::LoadAllSound(const std::string& folder)
        {
            const fs::path root = folder;

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
            if (!mSystem) {
                soundMap.clear();  // Clear the map even if system is gone
                std::cout << "[Audio] System already shut down.\n";
                return;
            }

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

        GLuint AssetsManager::GetOrLoad(const std::string& path)
        {
            namespace fs = std::filesystem;
            std::error_code ec;

            // 1) Normalize what we got
            fs::path p_in = fs::path(path);
            std::string key = p_in.lexically_normal().string();

            // 2) If already cached, return it
            if (auto it = s_pathToID.find(key); it != s_pathToID.end())
                return it->second;

            // 3) If the file doesn't exist OR the user passed only a filename (no parent),
            //    resolve it by scanning the default Textures root.
            const std::string kTexturesRoot = "../../MyEngine/Assets/Textures";
            bool needsResolve = !fs::exists(p_in, ec) || !p_in.has_parent_path();

            if (needsResolve)
            {
                // Reduce to base filename and search the whole Textures tree.
                std::string base = p_in.filename().string();
                if (base.empty()) base = key;

                std::string resolved = FindTextureByFilename(base, kTexturesRoot);
                if (!resolved.empty())
                    key = fs::path(resolved).lexically_normal().string();
            }

            // 4) Re-check cache under the (possibly) resolved key
            if (auto it2 = s_pathToID.find(key); it2 != s_pathToID.end())
                return it2->second;

            // 5) Load and cache
            GLuint id = LoadTextureFile(key);
            if (id)
            {
                s_pathToID[key] = id;
                s_idToPath[id] = key;
            }
            else
            {
                std::cerr << "Failed to load texture: " << key
                    << " reason: " << (stbi_failure_reason() ? stbi_failure_reason() : "unknown") << "\n";
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

        static std::string normalize_full(const std::filesystem::path& p) {
            return p.lexically_normal().string();
        }

        std::string AssetsManager::FindTextureByFilename(std::string_view filename,
            const std::string& root)
        {
            namespace fs = std::filesystem;
            if (filename.empty()) return {};

            // Reduce any path to just the base filename ("../Enemies/Hero.png" -> "Hero.png")
            fs::path nameOnly = fs::path(std::string(filename)).filename();

            std::error_code ec;

            // 1) If the caller actually passed a full path and it exists, accept it.
            fs::path maybePath = std::string(filename);
            if ((maybePath.has_parent_path() || maybePath.is_absolute()) && fs::exists(maybePath, ec)) {
                return normalize_full(maybePath);
            }

            // 2) Try to find an already-loaded path in our cache that matches the filename.
            for (const auto& kv : s_idToPath) {                // uses your existing map
                fs::path p = kv.second;
                if (p.filename() == nameOnly) {
                    return normalize_full(p);
                }
            }

            // 3) Search the Textures folder recursively.
            fs::path rootDir = root;
            if (!fs::exists(rootDir, ec)) return {};

            for (const auto& entry : fs::recursive_directory_iterator(rootDir, ec)) {
                if (entry.is_regular_file(ec) && entry.path().filename() == nameOnly) {
                    return normalize_full(entry.path());
                }
            }

            return {};
        }

        GLuint AssetsManager::GetOrLoadByFilename(std::string_view filename,
            const std::string& root)
        {
            const std::string full = FindTextureByFilename(filename, root);
            if (full.empty()) return 0;
            return GetOrLoad(full);    
        }

        bool AssetsManager::InitFonts()
        {
            if (FT_Init_FreeType(&m_ft)) {
                std::cerr << "[Font] ERROR: Could not init FreeType library\n";
                m_ft = nullptr;
                return false;
            }
            return true;
        }

        void AssetsManager::ShutdownFonts()
        {
            // delete glyph textures
            for (auto& kv : m_fontMap) {
                for (auto& g : kv.second.glyphs) {
                    glDeleteTextures(1, &g.second.TextureID);
                }
            }
            m_fontMap.clear();

            if (m_ft) {
                FT_Done_FreeType(m_ft);
                m_ft = nullptr;
            }
        }


        bool AssetsManager::LoadFontFile(const std::string& fontFilePath,
            const std::string& fontKey,
            unsigned int pixelHeight /* e.g. 48 */)
        {
            // Don't reload if we already have it
            if (m_fontMap.find(fontKey) != m_fontMap.end())
                return true;

            if (!m_ft) {
                std::cerr << "[Font] ERROR: FreeType not initialized\n";
                return false;
            }

            FT_Face face = nullptr;
            if (FT_New_Face(m_ft, fontFilePath.c_str(), 0, &face)) {
                std::cerr << "[Font] ERROR: Failed to load face " << fontFilePath << "\n";
                return false;
            }

            FT_Set_Pixel_Sizes(face, 0, pixelHeight);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            FontData data;
            data.pixelHeight = pixelHeight;

            for (unsigned char c = 0; c < 128; ++c) {
                if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                    // couldn't load this glyph, skip it
                    continue;
                }

                GLuint tex;
                glGenTextures(1, &tex);
                glBindTexture(GL_TEXTURE_2D, tex);

                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_R8, // single channel
                    face->glyph->bitmap.width,
                    face->glyph->bitmap.rows,
                    0,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    face->glyph->bitmap.buffer
                );

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                Character ch{
                    tex,
                    { (int)face->glyph->bitmap.width,  (int)face->glyph->bitmap.rows },
                    { (int)face->glyph->bitmap_left,   (int)face->glyph->bitmap_top  },
                    (unsigned int)face->glyph->advance.x
                };

                data.glyphs.emplace((char)c, ch);
            }

            FT_Done_Face(face);
            m_fontMap.emplace(fontKey, std::move(data));
            std::cout << "[Font] Loaded font '" << fontKey
                << "' from " << fontFilePath
                << " @ " << pixelHeight << "px\n";
            return true;
        }

        void AssetsManager::LoadAllFonts(const std::string& folder, unsigned int pixelHeight)
        {
            fs::path root = folder;
            if (!fs::exists(root)) {
                std::cerr << "[Font] Folder not found: " << root << "\n";
                return;
            }

            for (auto const& entry : fs::recursive_directory_iterator(root)) {
                if (!entry.is_regular_file()) continue;

                fs::path filePath = entry.path();
                std::string ext = filePath.extension().string();
                for (auto& c : ext) c = (char)tolower(c);

                // accept .ttf .otf
                if (ext != ".ttf" && ext != ".otf")
                    continue;

                // make a key from filename only (lowercase, no spaces if you want)
                std::string fontKey = filePath.stem().string();
                for (auto& c : fontKey) {
                    c = (c == ' ') ? '_' : (char)tolower(c);
                }

                // also encode size into key so we can support multiple sizes later
                fontKey += "_" + std::to_string(pixelHeight);

                LoadFontFile(filePath.string(), fontKey, pixelHeight);
            }
        }

        const FontData* AssetsManager::GetFont(const std::string& fontKey) const
        {
            auto it = m_fontMap.find(fontKey);
            if (it == m_fontMap.end()) return nullptr;
            return &it->second;
        }



    } // namespace Framework
