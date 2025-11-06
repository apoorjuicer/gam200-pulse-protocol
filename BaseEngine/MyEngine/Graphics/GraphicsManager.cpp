/******************************************************************************/
/**
 * @file        GraphicsManager.cpp
 * @project     Pulse Protocol
 * @author      Goh Pin Kai (primary)
 * @author      Leu Jun Yong (secondary)
 * @author      Ban Kai Wei Benjamin (secondary)
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
#include "../Resources/AssetRegistry.h"
#include "glapp.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../Graphics/stb_image.h"

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
}

void GraphicsManager::Update()
{
    // Poll input / advance time
    glfwPollEvents();
    GLHelper::update_time(1.0);
    glapp->update(Engine->GetWindow(), EntityMember);
    glClearColor(0.0f, 0.5f, 0.8f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //--- Rendering passes ---
    // Background pass
    glapp->draw(EntityMember, LAYER_BACKGROUND);

    // Sprites pass
    glapp->draw(EntityMember, LAYER_WORLD);
    //------------------------

    // Draw in-game gui system & UI overlay
    RenderUIOverlayElements();

    // Draw overlay text last
    auto &textShader = GLApp::shdrpgms[3];

    // Build FPS string and heaviest system string and render it
    if (!MyEditor::IsEnabled())
    {

        if (m_debugFont)
        {
            std::ostringstream fpsText;
            fpsText << "FPS: " << std::fixed << std::setprecision(1) << GLHelper::fps;

            RenderText(GLApp::shdrpgms[3],
                       fpsText.str(),
                       100.0f, 850.0f,
                       1.0f,
                       glm::vec3(0.5f, 0, 0),
                       m_debugFont);

            // Add top system below FPS
            auto *profiler = ProfilingManager::GetInstance();
            const auto &profiles = profiler->GetDisplayProfiles();

            std::string topSystem = "None";
            double topPercentage = 0.0;
            for (const auto &[systemName, profile] : profiles)
            {
                if (profile.percentage > topPercentage)
                {
                    topPercentage = profile.percentage;
                    topSystem = systemName;
                }
            }

            std::ostringstream topText;
            topText << topSystem << ": " << std::fixed
                    << std::setprecision(2) << topPercentage << "%";

            // RenderText(GLApp::shdrpgms[3], topText.str(), 100.0f, 880.0f,
            //            0.8f, glm::vec3(0.5f, 0.5f, 0), m_debugFont);
        }
        else
        {
            std::cerr << "[GraphicsManager] WARNING: m_debugFont not found!\n";
        }
    }

    if (m_uiFont)
    {
        // Team label, error probably from IMGUI
        RenderText(
            GLApp::shdrpgms[3],
            "Team BLOOP",
            600.0f, 850.0f,
            1.0f,
            glm::vec3(1, 1, 1),
            m_uiFont);
    }
    else
    {
        std::cerr << "[GraphicsManager] WARNING: m_uiFont not found!\n";
    }

    // FPS counter (while editor hidden)

    // // Controls help, anchored at right side
    // {
    //     int fbW = 0, fbH = 0;
    //     glfwGetFramebufferSize(Engine->GetWindow(), &fbW, &fbH);

    // team label using font patrickhand-regular_48
    // RenderText(textShader, TEAM_LABEL, teamLabelX, teamLabelY, TEAM_LABEL_SCALE, TEAM_COLOR, m_uiFont);

    //// Controls help, anchored at right side
    //{
    //    int fbW = 0, fbH = 0;
    //    glfwGetFramebufferSize(Engine->GetWindow(), &fbW, &fbH);

    //    float startX = fbW - 350.0f;
    //    float startY = fbH - 200.0f;
    //    float lineHeight = 25.0f;

    //    RenderText(textShader, "Controls:",
    //        startX, startY,
    //        0.5f,
    //        glm::vec3(1, 1, 0),
    //        m_debugFont);

    //    RenderText(textShader, "M - Start monsters",
    //        startX, startY + lineHeight * 1,
    //        0.4f,
    //        glm::vec3(1, 1, 1),
    //        m_debugFont);

    //    RenderText(textShader, "W/A/S/D - Input",
    //        startX, startY + lineHeight * 2,
    //        0.4f,
    //        glm::vec3(1, 1, 1),
    //        m_debugFont);

    //    RenderText(textShader, "K/L - Scale hero",
    //        startX, startY + lineHeight * 3,
    //        0.4f,
    //        glm::vec3(1, 1, 1),
    //        m_debugFont);

    //    RenderText(textShader, "H/J - Rotate monsters",
    //        startX, startY + lineHeight * 4,
    //        0.4f,
    //        glm::vec3(1, 1, 1),
    //        m_debugFont);

    //    RenderText(textShader, "O - Toggle editor",
    //        startX, startY + lineHeight * 5,
    //        0.4f,
    //        glm::vec3(1, 1, 1),
    //        m_debugFont);

    //    RenderText(textShader, "F1-F6 - Debug draw",
    //        startX, startY + lineHeight * 6,
    //        0.4f,
    //        glm::vec3(1, 1, 1),
    //        m_debugFont);
    //}
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

void GraphicsManager::RenderUIOverlayElements()
{
    auto *coordinator = Coordinator::GetInstance();
    auto inputMgr = coordinator->GetSystem<InputManager>();

    if (!inputMgr)
    {
        std::cerr << "[GraphicsManager] : inputMgr not found!!! in RenderUIOverlayElements\n";
        return;
    }

    auto &shader = GLApp::shdrpgms[2];
    shader.Use();

    for (auto entity : inputMgr->GUIButtonEntities)
    {
        if (!coordinator->HasComponent<GUIButton>(entity))
            continue;
        if (!coordinator->HasComponent<Framework::Transform>(entity))
            continue;

        auto &button = coordinator->GetComponent<GUIButton>(entity);
        auto &transform = coordinator->GetComponent<Framework::Transform>(entity);

        // Skip rendering if entity is hidden
        if (!transform.isVisible)
        {
            continue;
        }

        // Skip rendering if alpha is 0 (transparent)
        if (button.normalColor.a <= 0.0f && button.hoverColor.a <= 0.0f)
        {
            continue;
        }

        // Choose color based on button state
        glm::vec4 color = (button.currentState == ButtonState::Hover)
                              ? button.hoverColor
                              : button.normalColor;

        // Convert world coords to screen coords
        float screenX = transform.Pos.x + 800.0f;
        float screenY = 450.0f - transform.Pos.y;

        // Draw colored rectangle
        RenderColoredQuad(screenX, screenY, transform.Scale.x, transform.Scale.y, color);

        // ADD TEXT LABEL ON THE BUTTON
        if (m_uiFont)
        {
            // Calculate text size to center it properly
            float textScale = 0.8f;

            float estimatedTextWidth = button.label.length() * 10.0f * textScale;
            float textX = screenX - (estimatedTextWidth * 0.5f) - 30.0f;
            float textY = screenY + 5.0f;

            // Different adjustments for each button, prevents both text from moving together
            if (button.buttonID == "btn_create")
            {
                textX -= 10.0f; // Move "Create Hero" left
                textY += 6.0f;  // Move down
            }
            else if (button.buttonID == "btn_destroy")
            {
                textX -= 10.0f; // Move "Destroy Hero" left
                textY += 6.0f;  // Move down
            }

            RenderText(
                GLApp::shdrpgms[3],
                button.label,
                textX,
                textY,
                textScale,
                glm::vec3(1.0f, 1.0f, 1.0f), // White text
                m_uiFont);

            // std::cerr << "[GraphicsManager] m_uiFont found!!!!!!!!!\n";
        }
        else
        {
            std::cerr << "[GraphicsManager] m_uiFont not found\n";
        }
    }
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

void GraphicsManager::RenderColoredQuad(float x, float y, float width, float height, glm::vec4 color)
{
    // DEBUG - only print first few frames
    // static int count = 0;
    // if (count++ < 10) {
    //    std::cout << "[RenderQuad] Rendering at screen: (" << x << ", " << y << ")" << std::endl;
    //}

    auto &shader = GLApp::shdrpgms[3]; // Text shader
    shader.Use();

    glUniformMatrix4fv(glGetUniformLocation(shader.GetHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Pass vec4 color including alpha
    glUniform3f(glGetUniformLocation(shader.GetHandle(), "textColor"), color.r, color.g, color.b);

    // if shader supports vec4:
    // glUniform4f(glGetUniformLocation(shader.GetHandle(), "color"), color.r, color.g, color.b, color.a);

    // Bind white texture instead of unbinding
    glBindTexture(GL_TEXTURE_2D, whiteTexture);
    glBindVertexArray(VAO);

    // Center the quad around the position
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;

    float vertices[6][4] = {
        {x - halfWidth, y - halfHeight, 0.0f, 0.0f},
        {x - halfWidth, y + halfHeight, 0.0f, 1.0f},
        {x + halfWidth, y + halfHeight, 1.0f, 1.0f},

        {x - halfWidth, y - halfHeight, 0.0f, 0.0f},
        {x + halfWidth, y + halfHeight, 1.0f, 1.0f},
        {x + halfWidth, y - halfHeight, 1.0f, 0.0f}};

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
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
        std::cout << "Found Font '" << id << "' in font map" << "\n";
        return &it->second;
    }

    std::cout << "Not Found Font '" << id << "' in font map" << "\n";
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