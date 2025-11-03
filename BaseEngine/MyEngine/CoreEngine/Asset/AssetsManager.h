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
#include <unordered_map>
#include <string>
#include <map>
#include <filesystem>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Framework {

    /**************************************************************************
     * Font rendering data types
     * These are at namespace scope so systems like GraphicsManager can use
     * them without including the whole AssetsManager implementation details.
     **************************************************************************/

    struct Character {
        GLuint      TextureID;   // OpenGL texture for this glyph
        glm::ivec2  Size;        // pixel width/height of glyph bitmap
        glm::ivec2  Bearing;     // offset from baseline to left/top
        unsigned int Advance;    // horizontal advance (1/64 px from FreeType)
    };

    struct FontData {
        std::map<char, Character> glyphs;   // ASCII glyph table
        unsigned int pixelHeight = 0;       // baked pixel height (e.g. 48)
    };


    /**************************************************************************
     * Texture metadata (for debugging / inspection; not required to render
     * during normal draw path because we also cache the GL texture ID map).
     **************************************************************************/
    struct TextureData {
        GLuint textureID = 0;
        int width = 0;
        int height = 0;
        int nrChannels = 0;
        unsigned char* data = nullptr; // raw loaded data (if kept)
        std::string textureName;
        std::string filePath;
    };


    /**************************************************************************
     * AssetsManager
     **************************************************************************/
    class AssetsManager {
    public:
        // --------------------------------------------------------------------
        // Singleton access
        // --------------------------------------------------------------------
        static AssetsManager& Instance() {
            static AssetsManager instance;
            return instance;
        }

        // --------------------------------------------------------------------
        // SOUND (FMOD)
        // --------------------------------------------------------------------
        // Load all .mp3/.wav in the given folder (recursive). Uses the filename
        // (lowercased, no extension) as the key.
        void LoadAllSound(const std::string& folder = "../../MyEngine/Assets/Sound");

        // Load a single sound by path and store in map under soundName.
        // 'loop' controls FMOD loop mode.
        bool LoadSound(const std::string& soundName,
            const std::string& filePath,
            bool loop);

        // Get a loaded FMOD::Sound* by key (returns nullptr if missing).
        FMOD::Sound* GetSound(const std::string& soundName);

        // Stop & release all sounds, clear map.
        void UnloadAllSound();

        // Inject FMOD system from engine init.
        void SetFMODSystem(FMOD::System* system) { mSystem = system; }


        // --------------------------------------------------------------------
        // TEXTURE (OpenGL)
        // --------------------------------------------------------------------
        // Returns existing texture ID if already loaded, otherwise loads from path.
        static GLuint GetOrLoad(const std::string& path);

        // Reverse lookup: texture ID -> original path (will throw if not found).
        static const std::string& PathFromID(GLuint texID);

        // Delete all GL textures tracked by this manager and clear records.
        static void Clear();

        // (Optional) allow engine/editor to inspect what we've loaded.
        const std::unordered_map<std::string, TextureData>& GetTextureMap() const {
            return textureMap;
        }

        // (Optional) allow engine/editor to inspect loaded sounds.
        const std::unordered_map<std::string, FMOD::Sound*>& GetSoundMap() const {
            return soundMap;
        }

        static std::string FindTextureByFilename(std::string_view filename,
            const std::string& root = "../../MyEngine/Assets/Textures");
        
        static GLuint GetOrLoadByFilename(std::string_view filename,
            const std::string& root = "../../MyEngine/Assets/Textures");


        // --------------------------------------------------------------------
        // FONT (FreeType)
        // --------------------------------------------------------------------
        // Must be called once before loading any fonts.
        bool InitFonts();

        // Frees glyph textures + shuts down FreeType.
        void ShutdownFonts();

        // Bake one font file (e.g. "PatrickHand-Regular.ttf") at a given pixel
        // height into a glyph atlas map stored under fontKey.
        // fontKey is something like "patrickhand-regular_48".
        bool LoadFontFile(const std::string& fontFilePath,
            const std::string& fontKey,
            unsigned int pixelHeight);

        // Walks a folder (recursive), finds .ttf/.otf, and calls LoadFontFile
        // on each one using "<filename>_<pixelHeight>" as the key.
        void LoadAllFonts(const std::string& folder,
            unsigned int pixelHeight);

        // Get baked font data for rendering text. Returns nullptr if missing.
        const FontData* GetFont(const std::string& fontKey) const;


    private:
        AssetsManager() = default;
        ~AssetsManager() = default;
        AssetsManager(const AssetsManager&) = delete;
        AssetsManager& operator=(const AssetsManager&) = delete;

        // --------------------------------------------------------------------
        // INTERNAL DATA
        // --------------------------------------------------------------------

        // ---------- Sound ----------
        std::unordered_map<std::string, FMOD::Sound*> soundMap;
        FMOD::System* mSystem = nullptr; // not owned; provided by engine init

        // ---------- Textures ----------
        std::unordered_map<std::string, TextureData> textureMap;

        static GLuint LoadTextureFile(const std::string& path);

        // path <-> GL texture ID mapping
        static inline std::unordered_map<std::string, GLuint> s_pathToID{};
        static inline std::unordered_map<GLuint, std::string> s_idToPath{};

        // ---------- Fonts ----------
        FT_Library m_ft = nullptr; // FreeType main library handle
        std::unordered_map<std::string, FontData> m_fontMap;
    };

} // namespace Framework
