/******************************************************************************/
/**
 * @file        GraphicsManager.cpp
 * @project     Pulse Protocol
 * @author      Goh Pin Kai (primary) - 60%
 * @author      Leu Jun Yong (secondary) - 20%
 * @author      Ban Kai Wei Benjamin (secondary) - 20%
 *
 * @brief       Implements GraphicsManager:
 *              - Initializes GL rendering systems
 *              - Draws world (via GLApp) and overlay text (via TextRenderer)
 *              - Updates FPS display text every frame (entity from scene JSON)
 *              - Handles window resize notifications
 *              - Manages font loading and lifecycle
 *              - Shutdown of local GL resources
 *
 *              DEBUG TEXT (FPS DISPLAY):
 *              The FPS text entity ("_FPS_Display") is created in the scene JSON
 *              with a TextComponent. Every frame, Update() finds this entity and
 *              updates its text field with the current FPS value. The entity is
 *              destroyed/managed by SceneManager when the scene unloads.
 *              Visibility toggle (F11 to show/hide) is handled by the input system
 *              by setting TextComponent::visible on the FPS entity.
 *
 *              FONTS:
 *              Fonts are loaded via FreeType and stored in m_fontMap.
 *              FontData contains glyph information rendered by TextRenderer.
 * 
 * @copyright  Copyright (C) 2026 DigiPen Institute of Technology.
 *             Reproduction or disclosure of this file or its contents without the
 *             prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "GraphicsManager.h"
#include "Resources/AssetRegistry.h"
#include "glapp.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Graphics/stb_image.h"

//ECS
#include "CoreEngine/ECS/Coordinator.h"
#include "CoreEngine/ECS/Types.h"

// These are the asset keys we expect to exist after AssetsManager::LoadAllFonts()
// namespace {
//    constexpr const char* UI_FONT_KEY = "patrickhand-regular_48";
//    constexpr const char* DEBUG_FONT_KEY = "liberation-mono_48";
//}
// TODO: Make these data-driven via Config class once config integration is stable.
namespace
{
    constexpr const char *UI_FONT_KEY = "PatrickHand_font";
    constexpr const char *DEBUG_FONT_KEY = "liberation_font";

    constexpr Entity NULL_ENTITY = static_cast<Entity>(-1);
}

// GraphicsManager::GraphicsManager()
//{
//     glapp = new GLApp;
// }

GraphicsManager::GraphicsManager(AssetRegistry *registry) : mRegistry{registry}
{
    // Initialize();
    // Construct GLApp only once
}

GraphicsManager::~GraphicsManager()
{
    // Cleanup happens in Shutdown()
}

void GraphicsManager::SetRegistry(AssetRegistry *registry)
{
    mRegistry = registry;
}

void GraphicsManager::Initialize()
{

    if (!mRegistry)
    {
        std::cerr << "[GraphicsManager] Initialize without registry.\n";
        return;
    }

    // Construct GLApp only once
    if (!glapp)
    {
        // same as glapp = new GLApp;
        // but more safe and better
        // glapp = std::make_unique<GLApp>();
        glapp = new GLApp;
    }
    // Print GL info and init the app renderer
    GLHelper::print_specs();
    glapp->init();
    InitFonts();
    if (!LoadFontsFromRegistry())
    {
        std::cerr << "[GraphicsManager] Failed to load some fonts from asset registry.\n";
    }
    else
    {
        //std::cout << "[GraphicsManager] Successfully loaded fonts from  asset registry.\n";
    }

    // Subscribe to combo messages - DONT REMOVE, TEST FOR MESSAGE SYSTEM
    {
        auto *msgMgr = MessageManager::GetInstance();
        msgMgr->Subscribe(
            Message::Type::INPUT_COMBO,
            "GraphicsManager",
            [this](const Message &msg)
            {
                const InputComboMessage &comboMsg =
                    static_cast<const InputComboMessage &>(msg);
                std::cout << "[GRAPHICS] Received combo: "
                          << comboMsg.comboName << std::endl;
            });
    }

    std::cout << "[GraphicsManager] Successfully initialized for graphics.\n";

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Build a top-left-origin orthographic projection matching our virtual res
    {
        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(Engine->GetWindow(), &fbW, &fbH);

        projection = glm::ortho(
            0.0f,
            static_cast<float>(GLApp::VIRTUAL_W),
            static_cast<float>(GLApp::VIRTUAL_H),
            0.0f);
    }

    // Grab fonts from AssetsManager (must have been loaded already in engine bootstrap)
    {
        // m_uiFont = AM.GetFont(UI_FONT_KEY);
        // m_debugFont = AM.GetFont(DEBUG_FONT_KEY);

        m_uiFont = GM_FindFont(UI_FONT_KEY);
        m_debugFont = GM_FindFont(DEBUG_FONT_KEY);
        if (!m_uiFont)
        {
            std::cerr << "[GraphicsManager] ERROR: missing font "
                      << UI_FONT_KEY << "\n";
        }

        if (!m_debugFont)
        {
            std::cerr << "[GraphicsManager] ERROR: missing font "
                      << DEBUG_FONT_KEY << "\n";
        }

        // PrintFontMap();
    }

    // Create VAO/VBO used for glyph quads
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // pos.xy + uv.xy for 6 verts
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

        // layout(location = 0) vec2 aPos;
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0, 2, GL_FLOAT, GL_FALSE,
            4 * sizeof(float),
            (void *)0);

        // layout(location = 1) vec2 aUV;
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE,
            4 * sizeof(float),
            (void *)(2 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // Initialize TextRenderer
    m_textRenderer.Initialize(VAO, VBO, projection);

    // Set text shader sampler uniform once
    {
        auto &textShader = GLApp::shdrpgms[3]; // index 3 assumed to be text shader
        textShader.Use();
        GLint loc = glGetUniformLocation(textShader.GetHandle(), "text");
        if (loc != -1)
        {
            glUniform1i(loc, 0); // sampler "text" uses texture unit 0
        }
    }

    if (!LoadTexturesFromRegistry())
    {
        std::cerr << "[GraphicsManager] Failed to load some textures from asset registry.\n";
    }
    else
    {
        std::cout << "[GraphicsManager] Successfully loaded textures from  asset registry.\n";
    }

    // create white texture for colored quads
    InitWhiteTexture();

    // Create fullscreen quad VAO/VBO for the screen-wide fade overlay
    // 2 triangles covering NDC [-1,1]x[-1,1], position only (vec2)
    {
        float overlayVerts[] = {
            -1.0f,  1.0f,   // top-left
            -1.0f, -1.0f,   // bottom-left
             1.0f, -1.0f,   // bottom-right

            -1.0f,  1.0f,   // top-left
             1.0f, -1.0f,   // bottom-right
             1.0f,  1.0f    // top-right
        };

        glGenVertexArrays(1, &m_overlayVAO);
        glGenBuffers(1, &m_overlayVBO);

        glBindVertexArray(m_overlayVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_overlayVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(overlayVerts), overlayVerts, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void GraphicsManager::Update()
{
    // Poll input / advance time
    glfwPollEvents();
    GLHelper::update_time(1.0);
    glapp->update(Engine->GetWindow(), EntityMember);
    glClearColor(0.3f, 0.36f, 0.32f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT);  // Don't clear depth
    glDisable(GL_DEPTH_TEST);      // Disable depth test for 2D

    // Update entity alpha fades before drawing (modifies Renderable.tintColor.a)
    UpdateEntityFades(static_cast<float>(GLHelper::delta_time));

    //--- Rendering passes ---
    glapp->draw(EntityMember, m_editorLayerFilter);

    // Render ALL TextComponent entities (including FPS text objects)
    Update2DTextComponent();

    // Render screen-wide fade overlay on top of everything
    if (m_screenFade.active)
    {
        UpdateScreenFade(static_cast<float>(GLHelper::delta_time));
        RenderScreenOverlay();
    }
}

void GraphicsManager::Shutdown()
{
    // Unsubscribe from messages - DONT REMOVE
    {
        auto *msgMgr = MessageManager::GetInstance();
        msgMgr->Unsubscribe(Message::Type::INPUT_COMBO, "GraphicsManager");
    }

    // Cleanup GLApp with new and delete
    if (glapp)
    {
        glapp->cleanup();
        delete glapp;
        glapp = nullptr;
    }

    // Cleanup GLApp with unique_ptr
    // if (glapp) {
    //    glapp->cleanup();  // your custom cleanup
    //    glapp.reset();     // calls delete automatically (safe if null)
    //}

    // Delete GPU buffers we own for text quads
    if (VAO)
        glDeleteVertexArrays(1, &VAO);
    if (VBO)
        glDeleteBuffers(1, &VBO);
    VAO = 0;
    VBO = 0;

    if (whiteTexture)
    {
        glDeleteTextures(1, &whiteTexture);
        whiteTexture = 0;
    }

    // Clean up screen-fade overlay
    if (m_overlayVAO) { glDeleteVertexArrays(1, &m_overlayVAO); m_overlayVAO = 0; }
    if (m_overlayVBO) { glDeleteBuffers(1, &m_overlayVBO);      m_overlayVBO = 0; }

    // Realease fonts
    ShutdownFonts();

    std::cout << "[GraphicsManager] Shutdown completed\n";
}

void GraphicsManager::OnResize(int width, int height)
{
    // Tell GLApp about new viewport etc.
    GLApp::OnResize(width, height);

    // If you want the projection to react to new size (dynamic UI scaling),
    // you can rebuild 'projection' here too.
}

void GraphicsManager::SetEditorLayerFilter(LayerMask filter)
{
    m_editorLayerFilter = filter;
    std::cout << "[GraphicsManager] Editor layer filter set to: " << filter << std::endl;
}

void GraphicsManager::InitWhiteTexture()
{
    glGenTextures(1, &whiteTexture);
    glBindTexture(GL_TEXTURE_2D, whiteTexture);

    // 1x1 white pixel
    unsigned char whitePixel[4] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    std::cout << "[GraphicsManager] White texture created for GUI buttons\n";
}

// Textures
static GLuint upload_rgba8(int w, int h, unsigned char *data)
{
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

GLuint GraphicsManager::LoadTextureFile(const std::string &path)
{
    // skip JSON files
    if (path.ends_with(".json"))
    {
        /*std::cerr << "[Graphics] Skipping JSON file (not a texture): " << path << "\n";*/
        return 0;
    }

    std::string resolvedPath = path;

    //std::cout << "[DEBUG LoadTextureFile] Original path: " << path << std::endl;

    if (path.find("../../") == 0)
    {
        // Get PulseEngine base path
        char exePath[MAX_PATH];
        GetModuleFileNameA(GetModuleHandleA("PulseEngine.dll"), exePath, MAX_PATH);
        std::filesystem::path currentDir = std::filesystem::path(exePath).parent_path();

        //std::cout << "[DEBUG LoadTextureFile] currentDir: " << currentDir << std::endl;

        std::filesystem::path basePath;
        if (std::filesystem::exists(currentDir / "PulseEngine"))
        {
            basePath = currentDir / "PulseEngine";  // Release
            //std::cout << "[DEBUG LoadTextureFile] Using release path" << std::endl;
        }
        else
        {
            auto baseEngineDir = currentDir;
            while (baseEngineDir.filename() != "BaseEngine" && baseEngineDir.has_parent_path())
            {
                baseEngineDir = baseEngineDir.parent_path();
            }
            basePath = baseEngineDir / "PulseEngine";  // Dev
            //std::cout << "[DEBUG LoadTextureFile] Using dev path" << std::endl;
        }

        // Strip ../../PulseEngine/ and use basePath
        std::string relativePath = path.substr(std::string("../../PulseEngine/").length());
        resolvedPath = (basePath / relativePath).make_preferred().string(); 

        //std::cout << "[DEBUG LoadTextureFile] Resolved to: " << resolvedPath << std::endl;
    }

    if (!std::filesystem::exists(resolvedPath))
    {
        std::cerr << "[DEBUG LoadTextureFile] FILE DOES NOT EXIST: " << resolvedPath << "\n";
        return 0;
    }
    else
    {
        //std::cout << "[DEBUG LoadTextureFile] FILE EXISTS! Size: "
          //  << std::filesystem::file_size(resolvedPath) << " bytes\n";
    }

    int w = 0, h = 0, n = 0;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(resolvedPath.c_str(), &w, &h, &n, 4);
    if (!data)
    {
        std::cerr << "Failed to load texture: " << path
                  << " reason: " << (stbi_failure_reason() ? stbi_failure_reason() : "unknown") << "\n";
        return 0;
    }
    GLuint tex_id = upload_rgba8(w, h, data);
    stbi_image_free(data);
    return tex_id;
}

bool GraphicsManager::LoadTexturesFromRegistry()
{
    if (!mRegistry)
    {
        std::cerr << "[Graphics] LoadTexturesFromRegistry called without registry.\n";
        return false;
    }

    auto const &texTable = mRegistry->getTexContainer();
    if (texTable.empty())
    {
        std::cout << "[Graphics] No textures listed in registry.\n";
        return true; // nothing to do, not an error
    }

    bool ok = true;

    for (auto const &[name, tr] : texTable)
    {
        if (tr.path.empty())
        {
            std::cerr << "[Texture] Missing path for id='" << name << "'\n";
            ok = false;
            continue;
        }

        // Load from disk to GL
        std::string resolvedPath = mRegistry->GetTextureItemPath(name);
        GLuint tex_id = LoadTextureFile(resolvedPath);
        if (tex_id == 0)
        {
            std::cerr << "[Texture] Failed: id='" << name << "' path='" << tr.path << "'\n";
            ok = false;
            continue;
        }

        // textureMap[name] = TextureData{ id, /*w*/0, /*h*/0, /*nr*/0, name, tr.path };
        texHandle_key[tr.tex_name] = tex_id;
        // s_idToPath[id] = tr.path;

        //std::cout << "[Texture] Loaded: id='" << name << "' path='" << tr.path
        //          << "' -> GL#" << tex_id << "\n";
    }

    return ok;
}

GLuint GraphicsManager::GetOrLoad(const std::string &tex_name)
{
    if (auto it = texHandle_key.find(tex_name); it != texHandle_key.end())
        return it->second;
    GLuint tex_id = LoadTextureFile(tex_name);
    if (tex_id)
    {
        texHandle_key[tex_name] = tex_id;
        texHandle_id[tex_id] = tex_name;
    }
    return tex_id;
}

std::string const &GraphicsManager::GetTexName(GLuint texID)
{
    return texHandle_id.at(texID); // throws if not found (use .find if you prefer no-throw)
}

void GraphicsManager::Clear()
{
    for (auto &kv : texHandle_key)
        glDeleteTextures(1, &kv.second);
    texHandle_key.clear();
    texHandle_id.clear();
}

GLuint const *GraphicsManager::GM_FindTextureHandle(std::string const &id) const
{
    if (auto it = texHandle_key.find(id); it != texHandle_key.end())
    {
        return &it->second;
    }
    return nullptr;
}

// Fonts
bool GraphicsManager::InitFonts()
{
    if (FT_Init_FreeType(&m_ft))
    {
        std::cerr << "[Font] ERROR: Could not init FreeType library\n";
        m_ft = nullptr;
        return false;
    }
    return true;
}

bool GraphicsManager::LoadFontFile(const std::string &fontFilePath,
                                   const std::string &fontKey,
                                   unsigned int pixelHeight /* e.g. 48 */)
{
    // Don't reload if we already have it
    if (m_fontMap.find(fontKey) != m_fontMap.end())
        return true;

    if (!m_ft)
    {
        std::cerr << "[Font] ERROR: FreeType not initialized\n";
        return false;
    }

    FT_Face face = nullptr;
    if (FT_New_Face(m_ft, fontFilePath.c_str(), 0, &face))
    {
        std::cerr << "[Font] ERROR: Failed to load face " << fontFilePath << "\n";
        return false;
    }

    FT_Set_Pixel_Sizes(face, 0, pixelHeight);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    FontData data;
    data.pixelHeight = pixelHeight;


    for (unsigned char c = 0; c < 128; ++c)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
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
            face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character ch{
            tex,
            {(int)face->glyph->bitmap.width, (int)face->glyph->bitmap.rows},
            {(int)face->glyph->bitmap_left, (int)face->glyph->bitmap_top},
            (unsigned int)face->glyph->advance.x};

        data.glyphs.emplace((char)c, ch);
    }

    FT_Done_Face(face);
    m_fontMap.emplace(fontKey, std::move(data));
    /*std::cout << "[Font] Loaded font '" << fontKey
        << "' from " << fontFilePath
        << " @ " << pixelHeight << "px\n";*/
    return true;
}

bool GraphicsManager::LoadFontsFromRegistry()
{
    if (!mRegistry)
    {
        std::cerr << "[Graphics] LoadFontsFromRegistry called without registry.\n";
        return false;
    }
    if (mRegistry->getFontContainer().empty())
    {
        std::cout << "[Graphics] No fonts listed in registry.\n";
        return true; // nothing to do, but not an error
    }

    bool ok = true;
    for (auto const &[key, fr] : mRegistry->getFontContainer())
    {

        if (fr.pixelHeight == 0)
        {
            std::cerr << "[Font] Missing pixelHeight for id='" << fr.font_id
                      << "' (" << fr.path << ")\n";
            ok = false;
            continue;
        }

        std::string resolvedPath = mRegistry->GetFontItemPath(key);
        if (!LoadFontFile(resolvedPath, fr.font_id, fr.pixelHeight))
        {
            std::cerr << "[Font] Failed: id='" << fr.font_id
                      << "' path='" << fr.path
                      << "' px=" << fr.pixelHeight << "\n";
            ok = false;
        }
        else
        {
           // std::cout << "[Font] Loaded: id='" << fr.font_id
           //           << "' path='" << fr.path
           //           << "' px=" << fr.pixelHeight << "\n";
        }
    }
    return ok;
}

void GraphicsManager::ShutdownFonts()
{
    // delete glyph textures
    for (auto &[key, font] : m_fontMap)
    {
        for (auto &[chr, glyph] : font.glyphs)
        {
            if (glyph.TextureID)
            {
            }
            glDeleteTextures(1, &glyph.TextureID);
        }
    }
    m_fontMap.clear();

    if (m_ft)
    {
        FT_Done_FreeType(m_ft);
        m_ft = nullptr;
    }

    std::cout << "[GraphicsManager] Fonts Shutdown completed\n";
}

FontData const *GraphicsManager::GM_FindFont(std::string const &id) const
{
    if (auto it = m_fontMap.find(id); it != m_fontMap.end())
    {
        //std::cout << "Found Font '" << id << "' in font map" << "\n";
        return &it->second;
    }

    //std::cout << "Not Found Font '" << id << "' in font map" << "\n";
    return nullptr;
}

// debug

void GraphicsManager::PrintFontMap()
{
    std::cout << "===== FONT MAP DUMP =====\n";

    for (const auto &[fontKey, fontData] : m_fontMap)
    {
        std::cout << "Font ID: " << fontKey << "\n";
        std::cout << "  Pixel Height: " << fontData.pixelHeight << "\n";
        std::cout << "  Glyph Count:  " << fontData.glyphs.size() << "\n";

        // Print each glyph info (optional   can be large)
        for (const auto &[ch, glyph] : fontData.glyphs)
        {
            std::cout << "    '" << ch << "'"
                      << " (ASCII " << static_cast<int>(ch) << "): "
                      << "TexID=" << glyph.TextureID
                      << ", Size=(" << glyph.Size.x << "," << glyph.Size.y << ")"
                      << ", Bearing=(" << glyph.Bearing.x << "," << glyph.Bearing.y << ")"
                      << ", Advance=" << glyph.Advance
                      << "\n";
        }

        std::cout << "-------------------------------\n";
    }

    std::cout << "===== END OF FONT MAP =====\n";
}

bool GraphicsManager::EvictTexture(std::string const& idOrPath)
{
    namespace fs = std::filesystem;

    // 1. Try exact key match
    auto it = texHandle_key.find(idOrPath);

    // 2. If not found, try matching by filename stem
    if (it == texHandle_key.end())
    {
        std::string targetFile = fs::path(idOrPath).filename().string();
        for (auto candidate = texHandle_key.begin();
            candidate != texHandle_key.end(); ++candidate)
        {
            std::string candidateFile = fs::path(candidate->first).filename().string();
            if (candidateFile == targetFile)
            {
                it = candidate;
                break;
            }
        }
    }

    if (it == texHandle_key.end())
        return false;

    GLuint oldId = it->second;

    // Delete GPU object
    if (oldId)
        glDeleteTextures(1, &oldId);

    // Remove forward mapping
    texHandle_key.erase(it);

    // Remove reverse mapping
    texHandle_id.erase(oldId);

    // 3. Also remove any OTHER entries pointing to the same GL ID
    // (handles the duplicate-key problem)
    for (auto jt = texHandle_key.begin(); jt != texHandle_key.end(); )
    {
        if (jt->second == oldId)
            jt = texHandle_key.erase(jt);
        else
            ++jt;
    }

    // 4. Reset ALL live entities that were using this deleted texture
    //    Without this, entities keep binding the deleted GL ID every frame
    //    causing GL_INVALID_OPERATION in BindTexture
    if (auto* coord = Coordinator::GetInstance())
    {
        for (Entity e : coord->GetAllEntities())
        {
            if (!coord->HasComponent<Framework::Renderable>(e))
                continue;
            auto& r = coord->GetComponent<Framework::Renderable>(e);
            if (r.textureID == oldId)
            {
                r.textureID = 0;              // forces re-load next frame
                r.needsTextureReload = true;
            }
        }
    }

    return true;
}

// std::ostream& operator<<(std::ostream& os, const FontData& fd)
//{
//     os << "FontData { pixelHeight=" << fd.pixelHeight
//         << ", glyphs=" << fd.glyphs.size() << " }";
//
//     // Optional: print first few glyphs for debug
//     int count = 0;
//     for (auto const& [ch, glyph] : fd.glyphs)
//     {
//         if (count++ >= 5) { os << ", ..."; break; }
//         os << "\n  '" << ch << "': (size=" << glyph.Size.x << "x" << glyph.Size.y
//             << ", bearing=" << glyph.Bearing.x << "," << glyph.Bearing.y
//             << ", advance=" << glyph.Advance << ")";
//     }
//
//     return os;
// }


//draw 2d text Component
void GraphicsManager::DrawText2D(
    std::string const& text,
    std::string const& fontId,
    float x, float y,
    float pixelHeight,
    glm::vec3 const& color,
    TextAlignment alignment
) {
    if (text.empty()) {
        return;
    }

    FontData const* fontData = GM_FindFont(fontId);
    if (!fontData)
    {
        std::cerr << "[GraphicsManager] DrawText2D: font '" << fontId << "' not found\n";
        return;
    }

    float scale = 1.0f;
    if (fontData->pixelHeight > 0u) {
        scale = pixelHeight / static_cast<float>(fontData->pixelHeight);
    }

    GLSLShader& textShader = GLApp::shdrpgms[3];

    // Use TextRenderer instead of old RenderText
    m_textRenderer.RenderText(textShader, text, x, y, scale, color, fontData, alignment);
}


void GraphicsManager::Update2DTextComponent() {
    // --- Draw Text components (screen-space UI text) ---
    auto* g_coordinator = Coordinator::GetInstance();

    for (Entity entity = 0; entity < MaxEntity; ++entity)
    {
        if (!g_coordinator->HasComponent<TextComponent>(entity) ||
            !g_coordinator->HasComponent<Framework::Transform>(entity)) {
            continue; //skip game object need transform(offset) and text
        }
           
        auto& txt = g_coordinator->GetComponent<TextComponent>(entity);
        auto& tr = g_coordinator->GetComponent<Framework::Transform>(entity);

        //not visible or empty text dont render
        if (!txt.visible || txt.text.empty()) { 
            continue;
        }
           

        //  pick font (fallback if empty)
        std::string fontId = txt.font_id.empty()
            ? UI_FONT_KEY // "PatrickHand_font"
            : txt.font_id;

        float screenX, screenY;

        // Check if debug entity (name starts with "_")
        bool isDebugEntity = false;
        if (g_coordinator->HasComponent<Name>(entity))
        {
            auto& name = g_coordinator->GetComponent<Name>(entity);
            if (!name.name.empty() && name.name[0] == '_')
            {
                isDebugEntity = true;
            }
        }

        if (isDebugEntity)
        {
            // Debug entities use screen coords directly
            screenX = tr.Pos.x + txt.offset.x;
            screenY = tr.Pos.y + txt.offset.y;
        }
        else
        {
            // Normal entities convert world coords to screen coords
            screenX = tr.Pos.x + (GLApp::VIRTUAL_W / 2.0f) + txt.offset.x;
            screenY = (GLApp::VIRTUAL_H / 2.0f) - tr.Pos.y + txt.offset.y;
        }

        DrawText2D(
            txt.text,
            fontId,
            screenX,
            screenY,
            txt.font_size,
            txt.color,
            txt.alignment
        );
    }
}

// ============================================================
//  Per-entity fade
// ============================================================

void GraphicsManager::StartEntityFade(Entity e, float toAlpha, float duration)
{
    auto* coord = Coordinator::GetInstance();
    if (!coord || !coord->HasComponent<Framework::Renderable>(e)) return;

    float fromAlpha = coord->GetComponent<Framework::Renderable>(e).tintColor.a;
    m_entityFades[e] = EntityFadeState{ fromAlpha, toAlpha,
                                        std::max(duration, 0.0001f), 0.0f };
}

bool GraphicsManager::IsEntityFadeDone(Entity e) const
{
    return m_entityFades.find(e) == m_entityFades.end();
}

void GraphicsManager::UpdateEntityFades(float dt)
{
    auto* coord = Coordinator::GetInstance();
    if (!coord) return;

    for (auto it = m_entityFades.begin(); it != m_entityFades.end(); )
    {
        Entity e = it->first;
        EntityFadeState& s = it->second;

        s.elapsed += dt;
        float t = std::min(s.elapsed / s.duration, 1.0f);
        float alpha = s.startAlpha + t * (s.targetAlpha - s.startAlpha);

        if (coord->HasComponent<Framework::Renderable>(e))
        {
            coord->GetComponent<Framework::Renderable>(e).tintColor.a = alpha;
        }

        if (t >= 1.0f)
            it = m_entityFades.erase(it);
        else
            ++it;
    }
}

// ============================================================
//  Screen-wide fade overlay
// ============================================================

void GraphicsManager::StartScreenFade(float fromAlpha, float toAlpha,
                                      float r, float g, float b, float duration)
{
    m_screenFade.r            = r;
    m_screenFade.g            = g;
    m_screenFade.b            = b;
    m_screenFade.startAlpha   = fromAlpha;
    m_screenFade.currentAlpha = fromAlpha;
    m_screenFade.targetAlpha  = toAlpha;
    m_screenFade.duration     = std::max(duration, 0.0001f);
    m_screenFade.elapsed      = 0.0f;
    m_screenFade.active       = true;
}

void GraphicsManager::StopScreenFade()
{
    m_screenFade.active = false;
}

bool GraphicsManager::IsScreenFadeDone() const
{
    return !m_screenFade.active;
}

float GraphicsManager::GetScreenAlpha() const
{
    return m_screenFade.currentAlpha;
}

void GraphicsManager::UpdateScreenFade(float dt)
{
    if (!m_screenFade.active) return;

    m_screenFade.elapsed += dt;
    float t = std::min(m_screenFade.elapsed / m_screenFade.duration, 1.0f);
    // Linear interpolation from startAlpha to targetAlpha
    m_screenFade.currentAlpha = m_screenFade.startAlpha +
        t * (m_screenFade.targetAlpha - m_screenFade.startAlpha);

    if (t >= 1.0f)
    {
        m_screenFade.currentAlpha = m_screenFade.targetAlpha;
        m_screenFade.active       = false;
    }
}

void GraphicsManager::RenderScreenOverlay()
{
    if (m_screenFade.currentAlpha <= 0.0f) return;
    if (!m_overlayVAO || GLApp::shdrpgms.size() < 5) return;

    GLSLShader& overlayShader = GLApp::shdrpgms[4];
    overlayShader.Use();

    GLint loc = glGetUniformLocation(overlayShader.GetHandle(), "uOverlayColor");
    if (loc >= 0)
    {
        glUniform4f(loc,
            m_screenFade.r,
            m_screenFade.g,
            m_screenFade.b,
            m_screenFade.currentAlpha);
    }

    glBindVertexArray(m_overlayVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    overlayShader.UnUse();
}