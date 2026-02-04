/******************************************************************************/
/**
 * @file        GraphicsManager.h
 * @project     Pulse Protocol
 * @author      Goh Pin Kai (primary) - 50%
 * @author      Leu Jun Yong (secondary) - 40%
 * @author      Ban Kai Wei Benjamin (secondary) - 10%
 *
 * @brief       Handles high-level rendering:
 *              - World draw via GLApp (sprites, backgrounds, entities)
 *              - Text rendering via TextRenderer (overlay text, UI)
 *              - Window resize notifications
 *              - Font loading and management via FreeType
 *              - FPS display update every frame (text entity from scene JSON)
 *
 *              NOTE ON DEBUG TEXT (FPS DISPLAY):
 *              The FPS text entity is created in the scene JSON with TextComponent.
 *              GraphicsManager updates its text content every frame by finding the
 *              entity by name ("_FPS_Display") and updating the text field.
 *              The entity lifecycle is managed by SceneManager, not GraphicsManager.
 *              Visibility toggle (F11) is handled by the input system via
 *              TextComponent::visible flag.
 *
 *              NOTE ON FONTS:
 *              Fonts are loaded and owned by GraphicsManager. FontData* is stored
 *              in m_fontMap. Glyphs are rendered via TextRenderer.
 *
 *              IMPORTANT:
 *              Transform still lives in namespace Framework, unchanged.
 * 
 * @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

// #include "pch/pch.h"
// #include "glapp.h"
// #include "CoreEngine/ECS/System.h"
// #include "CoreEngine/ECS/SystemManager.h"
// #include "Transform.h"
// #include "../Physics/collision.h"
// #include "../CoreEngine/Asset/AssetsManager.h"
// #include "../Resources/AssetRegistry.h"

#pragma once
#include "pch/pch_temp.h"

#include "Layer.h"
#include "Transform.h"
#include "Text.h"
#include "CoreEngine/ECS/System.h"
#include "CoreEngine/Core/ImportExport.h"
#include "TextRenderer.h"
// #include "CoreEngine/ECS/SystemManager.h"
//#include "UI/GUIManager.h"
//#include "UI/UIElement.h"

#include "glapp.h"
#include <ft2build.h>
#include <memory> // redundant, alr in pch_temp
#include FT_FREETYPE_H

class AssetRegistry; // fwd
class GLApp;         // fwd

/**************************************************************************
 * Font rendering data types
 **************************************************************************/
// cache of all glyphs(characters) loaded in a font
// for rendering FreeType + OpenGL, represents 1 letter(a single glyph)
struct Character
{
    GLuint TextureID;     // OpenGL texture for this glyph
    glm::ivec2 Size;      // pixel width/height of glyph bitmap
    glm::ivec2 Bearing;   // offset from baseline to left/top
    unsigned int Advance; // horizontal advance (1/64 px from FreeType)
};
// Represents an entire font face
// contain glyph(s) container and pixelHeight
struct FontData
{
    std::map<char, Character> glyphs; // ASCII glyph table
    unsigned int pixelHeight = 0;     // baked pixel height (e.g. 48)
};

// struct TextureData {
//     GLuint textureID = 0;
//     int width = 0, height = 0, nrChannels = 0;
//     // (Optional) debug info only:
//     std::string textureName;
//     std::string filePath;
// };

class DLL_API GraphicsManager : public Systems
{
public:
    GraphicsManager() = default;
    explicit GraphicsManager(AssetRegistry *registry);
    ~GraphicsManager();

    void Initialize();
    void Update();
    void Shutdown();
    void OnResize(int width, int height);
    void SetEditorLayerFilter(LayerMask filter);

    GraphicsManager(GraphicsManager const &) = delete; // no copy
    GraphicsManager &operator=(GraphicsManager const &) = delete;
    GraphicsManager(GraphicsManager &&) noexcept = default; // move ok
    GraphicsManager &operator=(GraphicsManager &&) noexcept = default;

    // Debug overlay toggle (hidden by default in Release, auto shown in Debug)
    bool IsDebugOverlayEnabled() const { return m_showDebugOverlay; }

    // Texture
    GLuint LoadTextureFile(std::string const &path);
    GLuint GetOrLoad(std::string const &id);
    std::string const &GetTexName(GLuint texID);
    void Clear();

    FontData const *GM_FindFont(std::string const &id) const;
    GLuint const *GM_FindTextureHandle(std::string const &id) const;

    bool   EvictTexture(std::string const& idOrPath);

    void SetRegistry(AssetRegistry* registry);

    //text component
    void DrawText2D(
        std::string const& text,
        std::string const& fontId,
        float x, float y,
        float pixelHeight,
        glm::vec3 const& color,
        TextAlignment alignment = TextAlignment::Left // default left aligned
    );

    void Update2DTextComponent();
private:
    // Draw text (world overlay / debug text etc.)
    TextRenderer m_textRenderer;

    bool LoadTexturesFromRegistry();

    // Fonts
    bool InitFonts();
    void ShutdownFonts();
    bool LoadFontFile(const std::string &fontFilePath,
                      const std::string &fontKey,
                      unsigned int pixelHeight);

    // bool LoadFontsFromRegistry(AssetRegistry const& reg);

    bool LoadFontsFromRegistry();

    void InitWhiteTexture();

private:
    // std::unique_ptr<GLApp> glapp;
    GLApp *glapp = nullptr; // owning raw pointer

    // Shared quad VAO/VBO for glyph rendering
    GLuint VAO = 0;
    GLuint VBO = 0;

    // Shader and rendering details
    GLuint whiteTexture;

    // Screen-space ortho for text drawing
    glm::mat4 projection{1.0f};

    // Cached font refs //no longer from AssetsManager
    FontData const *m_uiFont = nullptr;
    FontData const *m_debugFont = nullptr;
    // Textures
    // std::unordered_map<std::string, TextureData> textureMap;
    // texture ID mapping, <name,GLuint>
    // Logical ID (engine/editor name) -> GPU handle
    std::unordered_map<std::string, GLuint> texHandle_key;
    std::unordered_map<GLuint, std::string> texHandle_id;
    // GPU handle -> Logical ID (for reverse lookup, debugging, reloading)
    // std::unordered_map<GLuint, std::string> handleToID;

    // static GLuint LoadTextureFile(const std::string& path);

    // path <-> GL texture ID mapping, <path,GLuint>
    // std::unordered_map<std::string, GLuint> s_pathToID{};
    // std::unordered_map<GLuint, std::string> s_idToPath{};

    AssetRegistry *mRegistry = nullptr; // link to asset registry

    FT_Library m_ft = nullptr; // FreeType main library handle
    std::unordered_map<std::string, FontData> m_fontMap;

    LayerMask m_editorLayerFilter = LAYER_ALL;

    // Debug overlay toggle - shown in Debug builds, hidden in Release builds
#ifdef _DEBUG
    bool m_showDebugOverlay = true;   // Show by default in Debug builds
#else
    bool m_showDebugOverlay = false;  // Hidden by default in Release builds (rubric compliance)
#endif

    //debug
    void PrintFontMap();
};


//std::ostream& operator<<(std::ostream& os, const FontData& fd);
