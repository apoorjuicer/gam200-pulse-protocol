/******************************************************************************/
/**
 * @file        GraphicsManager.cpp
 * @project     Pulse Protocol
 * @author      Goh Pin Kai (primary) - 70%
 * @author      Leu Jun Yong (secondary) - 30%
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
#include <iomanip>   // for std::setprecision
#include <iostream>
#include <chrono>

float editorWidth;
float editorHeight;

// These are the asset keys we expect to exist after AssetsManager::LoadAllFonts()
namespace {
    constexpr const char* UI_FONT_KEY = "patrickhand-regular_48";
    constexpr const char* DEBUG_FONT_KEY = "liberation-mono_48";
}

GraphicsManager::GraphicsManager()
{
    glapp = new GLApp;
}

GraphicsManager::~GraphicsManager()
{
    // Cleanup happens in Shutdown()
}

void GraphicsManager::Initialize()
{
    GLHelper::print_specs();
    auto& AM = Framework::AssetsManager::Instance();

    AM.InitFonts();
    AM.LoadAllFonts("../../MyEngine/Assets/Font", 48);

    // Print GL info and init the app renderer
    glapp->init();

    // Subscribe to combo messages - DONT REMOVE, TEST FOR MESSAGE SYSTEM
    {
        auto* msgMgr = MessageManager::GetInstance();
        msgMgr->Subscribe(
            Message::Type::INPUT_COMBO,
            "GraphicsManager",
            [this](const Message& msg) {
                const InputComboMessage& comboMsg =
                    static_cast<const InputComboMessage&>(msg);
                std::cout << "[GRAPHICS] Received combo: "
                    << comboMsg.comboName << std::endl;
            }
        );
    }

    std::cout << "[GraphicsManager] Successfully initialized for graphics.\n";

    // We want text to alpha-blend on top of world
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    // Build a top-left-origin orthographic projection matching our virtual res
    {
        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(Engine->GetWindow(), &fbW, &fbH);

        projection = glm::ortho(
            0.0f,
            static_cast<float>(GLApp::VIRTUAL_W),
            static_cast<float>(GLApp::VIRTUAL_H),
            0.0f
        );
    }

    // Grab fonts from AssetsManager (must have been loaded already in engine bootstrap)
    {
        m_uiFont = AM.GetFont(UI_FONT_KEY);
        m_debugFont = AM.GetFont(DEBUG_FONT_KEY);

        if (!m_uiFont) {
            std::cerr << "[GraphicsManager] ERROR: missing font "
                << UI_FONT_KEY << "\n";
        }
        if (!m_debugFont) {
            std::cerr << "[GraphicsManager] ERROR: missing font "
                << DEBUG_FONT_KEY << "\n";
        }
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
            (void*)0
        );

        // layout(location = 1) vec2 aUV;
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE,
            4 * sizeof(float),
            (void*)(2 * sizeof(float))
        );

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // Set text shader sampler uniform once
    {
        auto& textShader = GLApp::shdrpgms[3]; // index 3 assumed to be text shader
        textShader.Use();
        GLint loc = glGetUniformLocation(textShader.GetHandle(), "text");
        if (loc != -1) {
            glUniform1i(loc, 0); // sampler "text" uses texture unit 0
        }
    }

    // create white texture for colored quads
    glGenTextures(1, &whiteTexture);
    glBindTexture(GL_TEXTURE_2D, whiteTexture);

    // 1x1 white pixel
    unsigned char whitePixel[4] = { 255, 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    std::cout << "[GraphicsManager] White texture created for GUI buttons\n";
}

void GraphicsManager::Update()
{
    // Poll input / advance time
    glfwPollEvents();
    GLHelper::update_time(1.0);

    // 1) Update and draw world (sprites etc.)
    glapp->update(Engine->GetWindow(), EntityMember);
    glapp->draw(EntityMember);
    // 2) then draw text on top
    //RenderText(GLApp::shdrpgms[3], "Team BLOOP", 600.0f, 850.0f, 1.0f, glm::vec3(1, 1, 1), Characters);

    // 3) Draw in-game gui system & UI overlay
    RenderUIOverlayElements();

    // 4) Draw overlay text last
    auto& textShader = GLApp::shdrpgms[3];

    // get the game viewport dimensions from the editor itself
    MyEditor::GameViewport viewport = MyEditor::ComputeMiddleGameViewportPixels(Engine->GetWindow(), false);

    // only renders if viewport is valid
    if (viewport.w > 0 && viewport.h > 0) {

        // if the 2 fonts are loaded
        if (m_debugFont && m_uiFont) {
            // get framebuffer dimensions
            int fbW = 0, fbH = 0;
            glfwGetFramebufferSize(Engine->GetWindow(), &fbW, &fbH);

            // calculate scaling factors between framebuffer and virtual coordinates
            float scaleX = static_cast<float>(GLApp::VIRTUAL_W) / static_cast<float>(fbW);
            float scaleY = static_cast<float>(GLApp::VIRTUAL_H) / static_cast<float>(fbH);

            // convert viewport position to virtual coordinates
            float virtualX = static_cast<float>(viewport.x) * scaleX;
            float virtualY = static_cast<float>(viewport.y) * scaleY;
            float virtualW = static_cast<float>(viewport.w) * scaleX;
            float virtualH = static_cast<float>(viewport.h) * scaleY;

            // text positioning const
            const float TEXT_PADDING = -325.0f;
            const float LINE_SPACING = 60.0f;
            const float BOTTOM_OFFSET = 60.0f;
            const float FPS_SCALE = 1.3f;
            const float SYSTEM_SCALE = 1.0f;

            const float TEAM_PADDING_X = -150.0f;
            const float TEAM_PADDING_Y = -225.0f;
            const float TEAM_LABEL_SCALE = 1.3f;

            // text color const
            const glm::vec3 FPS_COLOR = glm::vec3(0.0f, 1.0f, 1.0f); // yellow
            const glm::vec3 SYSTEM_COLOR = glm::vec3(1.0f, 0.0f, 1.0f); // green
            const glm::vec3 TEAM_COLOR = glm::vec3(1.0f, 1.0f, 1.0f); // white

            const std::string TEAM_LABEL = "Team BLOOP";

            // bottom-left of game viewport
            float textX = virtualX + TEXT_PADDING;
            float textY = virtualY + virtualH - BOTTOM_OFFSET;

            // top right of game viewport
            float estimatedWidth = TEAM_LABEL.length() * 15.0f * TEAM_LABEL_SCALE;
            float teamLabelX = virtualX + virtualW - estimatedWidth - TEAM_PADDING_X;
            float teamLabelY = virtualY + TEAM_PADDING_Y;

            // render FPS text; uses font liberation-mono_48
            std::ostringstream fpsText;
            fpsText << "FPS: " << std::fixed << std::setprecision(1) << GLHelper::fps;
            
            // uses font: 
            RenderText(GLApp::shdrpgms[3], fpsText.str(), textX, textY, FPS_SCALE, FPS_COLOR, m_debugFont);

            // get top system from profiler
            auto* profiler = ProfilingManager::GetInstance();
            const auto& profiles = profiler->GetDisplayProfiles();

            std::string topSystem = "None";
            double topPercentage = 0.0;
            for (const auto& [systemName, profile] : profiles) {
                if (profile.percentage > topPercentage) {
                    topPercentage = profile.percentage;
                    topSystem = systemName;
                }
            }

            // render top system below FPS text
            std::ostringstream topText;
            topText << topSystem << ": " << std::fixed
                << std::setprecision(2) << topPercentage << "%";

            // uses font liberation-mono_48
            RenderText(textShader, topText.str(), textX, textY + LINE_SPACING, SYSTEM_SCALE, SYSTEM_COLOR, m_debugFont);

            // team label using font patrickhand-regular_48
            RenderText(textShader, TEAM_LABEL, teamLabelX, teamLabelY, TEAM_LABEL_SCALE, TEAM_COLOR, m_uiFont);
        }
        else {
            std::cerr << "[GraphicsManager] WARNING: fonts not found!\n";
        }
    }
    // // Controls help, anchored at right side
    // {
    //     int fbW = 0, fbH = 0;
    //     glfwGetFramebufferSize(Engine->GetWindow(), &fbW, &fbH);

    //     float startX = fbW - 350.0f;
    //     float startY = fbH - 200.0f;
    //     float lineHeight = 25.0f;

    //     RenderText(textShader, "Controls:",
    //         startX, startY,
    //         0.5f,
    //         glm::vec3(1, 1, 0),
    //         m_debugFont);

    //     RenderText(textShader, "M - Start monsters",
    //         startX, startY + lineHeight * 1,
    //         0.4f,
    //         glm::vec3(1, 1, 1),
    //         m_debugFont);

    //     RenderText(textShader, "W/A/S/D - Input",
    //         startX, startY + lineHeight * 2,
    //         0.4f,
    //         glm::vec3(1, 1, 1),
    //         m_debugFont);

    //     RenderText(textShader, "K/L - Scale hero",
    //         startX, startY + lineHeight * 3,
    //         0.4f,
    //         glm::vec3(1, 1, 1),
    //         m_debugFont);

    //     RenderText(textShader, "H/J - Rotate monsters",
    //         startX, startY + lineHeight * 4,
    //         0.4f,
    //         glm::vec3(1, 1, 1),
    //         m_debugFont);

    //     RenderText(textShader, "O - Toggle editor",
    //         startX, startY + lineHeight * 5,
    //         0.4f,
    //         glm::vec3(1, 1, 1),
    //         m_debugFont);

    //     RenderText(textShader, "F1-F6 - Debug draw",
    //         startX, startY + lineHeight * 6,
    //         0.4f,
    //         glm::vec3(1, 1, 1),
    //         m_debugFont);
    // }
}


void GraphicsManager::Shutdown()
{
    // Unsubscribe from messages - DONT REMOVE
    {
        auto* msgMgr = MessageManager::GetInstance();
        msgMgr->Unsubscribe(Message::Type::INPUT_COMBO, "GraphicsManager");
    }

    // Cleanup GLApp
    if (glapp) {
        glapp->cleanup();
        delete glapp;
        glapp = nullptr;
    }

    // Delete GPU buffers we own for text quads
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    VAO = 0;
    VBO = 0;

    if (whiteTexture) {
        glDeleteTextures(1, &whiteTexture);
        whiteTexture = 0;
    }

    std::cout << "[GraphicsManager] Shutdown completed\n";
}

void GraphicsManager::RenderText(
    GLSLShader& shader,
    const std::string& text,
    float x, float y,
    float scale,
    const glm::vec3& color,
    const Framework::FontData* fontData
)
{
    if (!fontData)
        return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader.Use();

    glUniformMatrix4fv(
        glGetUniformLocation(shader.GetHandle(), "projection"),
        1, GL_FALSE,
        glm::value_ptr(projection)
    );

    glUniform3f(
        glGetUniformLocation(shader.GetHandle(), "textColor"),
        color.x, color.y, color.z
    );

    glUniform1i(
        glGetUniformLocation(shader.GetHandle(), "text"),
        0
    );

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    for (unsigned char c : text) {
        auto it = fontData->glyphs.find(c);
        if (it == fontData->glyphs.end())
            continue;

        const auto& ch = it->second;

        // screen quad placement
        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - ch.Bearing.y * scale + ch.Size.y * scale;
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        // 6 verts, each vert = {pos.x, pos.y, uv.x, uv.y}
        float verts[6][4] = {
            { xpos,     ypos,     0.0f, 1.0f },
            { xpos,     ypos - h, 0.0f, 0.0f },
            { xpos + w, ypos - h, 1.0f, 0.0f },

            { xpos,     ypos,     0.0f, 1.0f },
            { xpos + w, ypos - h, 1.0f, 0.0f },
            { xpos + w, ypos,     1.0f, 1.0f }
        };

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
    auto* coordinator = Coordinator::GetInstance();
    auto inputMgr = coordinator->GetSystem<InputManager>();

    if (!inputMgr) return;

    auto& shader = GLApp::shdrpgms[2];
    shader.Use();

    for (auto entity : inputMgr->GUIButtonEntities) {
        if (!coordinator->HasComponent<GUIButton>(entity)) continue;
        if (!coordinator->HasComponent<Framework::Transform>(entity)) continue;

        auto& button = coordinator->GetComponent<GUIButton>(entity);
        auto& transform = coordinator->GetComponent<Framework::Transform>(entity);

        // Skip rendering if entity is hidden
        if (!transform.isVisible) {
            continue;
        }

        // Skip rendering if alpha is 0 (transparent)
        if (button.normalColor.a <= 0.0f && button.hoverColor.a <= 0.0f) {
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
        if (m_uiFont) {
            // Calculate text size to center it properly
            float textScale = 0.8f;

            float estimatedTextWidth = button.label.length() * 10.0f * textScale;
            float textX = screenX - (estimatedTextWidth * 0.5f) - 30.0f;
            float textY = screenY + 5.0f;

            // Different adjustments for each button, prevents both text from moving together
            if (button.buttonID == "btn_create") {
                textX -= 10.0f;  // Move "Create Hero" left
                textY += 6.0f;   // Move down
            }
            else if (button.buttonID == "btn_destroy") {
                textX -= 10.0f;  // Move "Destroy Hero" left
                textY += 6.0f;   // Move down
            }

            RenderText(
                GLApp::shdrpgms[3],
                button.label,
                textX,
                textY,
                textScale,
                glm::vec3(1.0f, 1.0f, 1.0f),  // White text
                m_uiFont
            );
        }
    }
}

void GraphicsManager::RenderColoredQuad(float x, float y, float width, float height, glm::vec4 color)
{
    // DEBUG - only print first few frames
    //static int count = 0;
    //if (count++ < 10) {
    //    std::cout << "[RenderQuad] Rendering at screen: (" << x << ", " << y << ")" << std::endl;
    //}

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto& shader = GLApp::shdrpgms[3]; // Text shader
    shader.Use();

    glUniformMatrix4fv(glGetUniformLocation(shader.GetHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Pass vec4 color including alpha
    glUniform3f(glGetUniformLocation(shader.GetHandle(), "textColor"), color.r, color.g, color.b);

    // if shader supports vec4:
    // glUniform4f(glGetUniformLocation(shader.GetHandle(), "color"), color.r, color.g, color.b, color.a);

    glBindTexture(GL_TEXTURE_2D, whiteTexture);
    glBindVertexArray(VAO);

    // Center the quad around the position
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;

    float vertices[6][4] = {
        { x - halfWidth, y - halfHeight, 0.0f, 0.0f },
        { x - halfWidth, y + halfHeight, 0.0f, 1.0f },
        { x + halfWidth, y + halfHeight, 1.0f, 1.0f },

        { x - halfWidth, y - halfHeight, 0.0f, 0.0f },
        { x + halfWidth, y + halfHeight, 1.0f, 1.0f },
        { x + halfWidth, y - halfHeight, 1.0f, 0.0f }
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}