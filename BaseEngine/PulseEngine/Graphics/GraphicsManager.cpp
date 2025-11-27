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
 *              - Draws world (via GLApp) then overlay text (FPS, controls)
 *              - Handles message subscriptions for debug combo inputs
 *              - Handles shutdown of local GL resources
 *
 *              Fonts:
 *              - We DO NOT create glyphs here.
 *              - We pull them from AssetsManager::GetFont() which returns
 *                const Framework::FontData*.
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
        std::cout << "[GraphicsManager] Successfully loaded fonts from  asset registry.\n";
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

    // create FPS/debug text entities (proper text objects - rubric compliant)
    CreateDebugTextEntities();
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

    //--- Rendering passes ---
    glapp->draw(EntityMember, m_editorLayerFilter);

    // Update FPS text content (syncs visibility with m_showDebugOverlay)
    UpdateDebugTextEntities();

    // Render ALL TextComponent entities (including FPS text objects)
    Update2DTextComponent();
    //------------------------
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

    // Realease fonts
    ShutdownFonts();

    std::cout << "[GraphicsManager] Shutdown completed\n";
}

void GraphicsManager::ToggleDebugOverlay()
{
    m_showDebugOverlay = !m_showDebugOverlay;

    // Immediately update visibility on text entities
    auto* g = Coordinator::GetInstance();
    if (g)
    {
        if (m_fpsTextEntity != NULL_ENTITY && g->HasComponent<TextComponent>(m_fpsTextEntity))
            g->GetComponent<TextComponent>(m_fpsTextEntity).visible = m_showDebugOverlay;

        if (m_topSystemTextEntity != NULL_ENTITY && g->HasComponent<TextComponent>(m_topSystemTextEntity))
            g->GetComponent<TextComponent>(m_topSystemTextEntity).visible = m_showDebugOverlay;

        if (m_teamLabelEntity != NULL_ENTITY && g->HasComponent<TextComponent>(m_teamLabelEntity))
            g->GetComponent<TextComponent>(m_teamLabelEntity).visible = m_showDebugOverlay;
    }

    std::cout << "[DEBUG OVERLAY] " << (m_showDebugOverlay ? "SHOWN" : "HIDDEN") << " (F8 to toggle)\n";
}

void GraphicsManager::RenderText(
    GLSLShader &shader,
    const std::string &text,
    float x, float y,
    float scale,
    const glm::vec3 &color,
    const FontData *fontData)
{
    if (!fontData)
    {
        std::cerr << "[GraphicsManager] : fontData not found!!! in RenderText\n";
        return;
    }

    shader.Use();

    glUniformMatrix4fv(
        glGetUniformLocation(shader.GetHandle(), "projection"),
        1, GL_FALSE,
        glm::value_ptr(projection));

    glUniform3f(
        glGetUniformLocation(shader.GetHandle(), "textColor"),
        color.x, color.y, color.z);

    glUniform1i(
        glGetUniformLocation(shader.GetHandle(), "text"),
        0);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    for (unsigned char c : text)
    {
        auto it = fontData->glyphs.find(c);
        if (it == fontData->glyphs.end())
            continue;

        const auto &ch = it->second;

        // screen quad placement
        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - ch.Bearing.y * scale + ch.Size.y * scale;
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        // 6 verts, each vert = {pos.x, pos.y, uv.x, uv.y}
        float verts[6][4] = {
            {xpos, ypos, 0.0f, 1.0f},
            {xpos, ypos - h, 0.0f, 0.0f},
            {xpos + w, ypos - h, 1.0f, 0.0f},

            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos - h, 1.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f}};

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        // advance cursor (FreeType gives advance in 1/64 pixels)
        x += (ch.Advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // std::cout << "[GraphicsManager] RenderText successful for " << text << "\n";
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

    int w = 0, h = 0, n = 0;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path.c_str(), &w, &h, &n, 4);
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
        GLuint tex_id = LoadTextureFile(tr.path);
        if (tex_id == 0)
        {
            std::cerr << "[Texture] Failed: id='" << name << "' path='" << tr.path << "'\n";
            ok = false;
            continue;
        }

        // textureMap[name] = TextureData{ id, /*w*/0, /*h*/0, /*nr*/0, name, tr.path };
        texHandle_key[tr.tex_name] = tex_id;
        // s_idToPath[id] = tr.path;

        std::cout << "[Texture] Loaded: id='" << name << "' path='" << tr.path
                  << "' -> GL#" << tex_id << "\n";
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

        if (!LoadFontFile(fr.path, fr.font_id, fr.pixelHeight))
        {
            std::cerr << "[Font] Failed: id='" << fr.font_id
                      << "' path='" << fr.path
                      << "' px=" << fr.pixelHeight << "\n";
            ok = false;
        }
        else
        {
            std::cout << "[Font] Loaded: id='" << fr.font_id
                      << "' path='" << fr.path
                      << "' px=" << fr.pixelHeight << "\n";
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

        // Print each glyph info (optional � can be large)
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

bool GraphicsManager::EvictTexture(std::string const &idOrPath)
{
    // Find by key (the same string used in GetOrLoad). Today you store by tex_name.
    auto it = texHandle_key.find(idOrPath);
    if (it == texHandle_key.end())
        return false;

    GLuint oldId = it->second;
    // Delete GPU object
    if (oldId)
        glDeleteTextures(1, &oldId);

    // Remove both directions
    texHandle_key.erase(it);
    if (auto it2 = texHandle_id.find(oldId); it2 != texHandle_id.end())
        texHandle_id.erase(it2);

    std::cout << "[Graphics] Evicted texture: " << idOrPath << "\n";
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
     glm::vec3 const& color
) {
    if (text.empty()) { //empty text dont need draw
        return;
    }

    FontData const* fontData = GM_FindFont(fontId);
    if (!fontData)
    {
        std::cerr << "[GraphicsManager] DrawText2D: font '" << fontId << "' not found\n";
        return;
    }

    float scale = 1.0f;
    //Font height is baked at 48pixels need to scale accordingly
    if (fontData->pixelHeight > 0u) {
        scale = pixelHeight / static_cast<float>(fontData->pixelHeight);
    }

    //text shader
    GLSLShader& textShader = GLApp::shdrpgms[3];
    RenderText(textShader, text, x, y, scale, color, fontData);
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
            txt.color
        );
    }
}

// =============================================================================
// Debug Text Entities (FPS, Top System, Team Label)
// These are proper text objects that can be moved/edited in the editor
// =============================================================================

void GraphicsManager::CreateDebugTextEntities()
{
    auto* g = Coordinator::GetInstance();
    if (!g) return;

    // ===== FPS TEXT ENTITY =====
    m_fpsTextEntity = g->CreateEntity();

    {
        Framework::Transform t;
        t.Pos = { 100.0f, 80.0f };
        t.Scale = { 1.0f, 1.0f };
        t.Rotation = { 0.0f, 0.0f };
        t.isVisible = true;
        t.autoHide = false;
        g->AddComponent(m_fpsTextEntity, t);
    }

    {
        TextComponent txt;
        txt.text = "FPS: --";
        txt.font_id = "liberation_font";
        txt.font_size = 40.0f;
        txt.color = { 0.0f, 1.0f, 1.0f };  // Cyan
        txt.offset = { 0.0f, 0.0f };
        txt.visible = m_showDebugOverlay;
        g->AddComponent(m_fpsTextEntity, txt);
    }

    {
        LayerTag layer;
        layer.mask = LAYER_UI;
        layer.orderInLayer = 999;
        layer.z = 100.0f;
        g->AddComponent(m_fpsTextEntity, layer);
    }

    {
        Name name;
        name.name = "_FPS_Display";
        g->AddComponent(m_fpsTextEntity, name);
    }

    // ===== TOP SYSTEM TEXT ENTITY =====
    m_topSystemTextEntity = g->CreateEntity();

    {
        Framework::Transform t;
        t.Pos = { 100.0f, 130.0f };
        t.Scale = { 1.0f, 1.0f };
        t.Rotation = { 0.0f, 0.0f };
        t.isVisible = true;
        t.autoHide = false;
        g->AddComponent(m_topSystemTextEntity, t);
    }

    {
        TextComponent txt;
        txt.text = "Top: --";
        txt.font_id = "liberation_font";
        txt.font_size = 40.0f;
        txt.color = { 1.0f, 0.0f, 1.0f };  // Magenta
        txt.offset = { 0.0f, 0.0f };
        txt.visible = m_showDebugOverlay;
        g->AddComponent(m_topSystemTextEntity, txt);
    }

    {
        LayerTag layer;
        layer.mask = LAYER_UI;
        layer.orderInLayer = 998;
        layer.z = 100.0f;
        g->AddComponent(m_topSystemTextEntity, layer);
    }

    {
        Name name;
        name.name = "_TopSystem_Display";
        g->AddComponent(m_topSystemTextEntity, name);
    }

    // ===== TEAM LABEL ENTITY =====
    m_teamLabelEntity = g->CreateEntity();

    {
        Framework::Transform t;
        t.Pos = { 1500.0f, 130.0f };  // Top-right area
        t.Scale = { 1.0f, 1.0f };
        t.Rotation = { 0.0f, 0.0f };
        t.isVisible = true;
        t.autoHide = false;
        g->AddComponent(m_teamLabelEntity, t);
    }

    {
        TextComponent txt;
        txt.text = "Team BLOOP";
        txt.font_id = "PatrickHand_font";
        txt.font_size = 60.0f;
        txt.color = { 1.0f, 1.0f, 1.0f };  // White
        txt.offset = { 0.0f, 0.0f };
        txt.visible = m_showDebugOverlay;
        g->AddComponent(m_teamLabelEntity, txt);
    }

    {
        LayerTag layer;
        layer.mask = LAYER_UI;
        layer.orderInLayer = 997;
        layer.z = 100.0f;
        g->AddComponent(m_teamLabelEntity, layer);
    }

    {
        Name name;
        name.name = "_Team_Label";
        g->AddComponent(m_teamLabelEntity, name);
    }

    std::cout << "[GraphicsManager] Created debug text entities (F8 to toggle)\n";
    std::cout << "                  FPS: Entity " << m_fpsTextEntity << "\n";
    std::cout << "                  TopSystem: Entity " << m_topSystemTextEntity << "\n";
    std::cout << "                  TeamLabel: Entity " << m_teamLabelEntity << "\n";
}

void GraphicsManager::UpdateDebugTextEntities()
{
    auto* g = Coordinator::GetInstance();
    if (!g) return;

    // Update FPS text
    if (m_fpsTextEntity != NULL_ENTITY && g->HasComponent<TextComponent>(m_fpsTextEntity))
    {
        auto& txt = g->GetComponent<TextComponent>(m_fpsTextEntity);

        std::ostringstream ss;
        ss << "FPS: " << std::fixed << std::setprecision(1) << GLHelper::fps;
        txt.text = ss.str();
        txt.visible = m_showDebugOverlay;
    }

    // Update top system text
    if (m_topSystemTextEntity != NULL_ENTITY && g->HasComponent<TextComponent>(m_topSystemTextEntity))
    {
        auto& txt = g->GetComponent<TextComponent>(m_topSystemTextEntity);

        auto* profiler = ProfilingManager::GetInstance();
        const auto& profiles = profiler->GetDisplayProfiles();

        std::string topSystem = "None";
        double topPercentage = 0.0;

        for (const auto& [name, profile] : profiles)
        {
            if (profile.percentage > topPercentage)
            {
                topPercentage = profile.percentage;
                topSystem = name;
            }
        }

        std::ostringstream ss;
        ss << topSystem << ": " << std::fixed << std::setprecision(2) << topPercentage << "%";
        txt.text = ss.str();
        txt.visible = m_showDebugOverlay;
    }

    // Update team label visibility (text is static)
    if (m_teamLabelEntity != NULL_ENTITY && g->HasComponent<TextComponent>(m_teamLabelEntity))
    {
        auto& txt = g->GetComponent<TextComponent>(m_teamLabelEntity);
        txt.visible = m_showDebugOverlay;
    }
}