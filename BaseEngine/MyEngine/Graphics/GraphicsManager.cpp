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
    auto& AM = Framework::AssetsManager::Instance();

    AM.InitFonts();
    AM.LoadAllFonts("../../MyEngine/Assets/Font", 48);

    // Print GL info and init the app renderer
    GLHelper::print_specs();
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
        auto& AM = Framework::AssetsManager::Instance();
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
}

void GraphicsManager::Update()
{
    // Poll input / advance time
    glfwPollEvents();
    GLHelper::update_time(1.0);

    // 1) Update and draw world (sprites etc.)
    glapp->update(Engine->GetWindow(), EntityMember);
    glapp->draw(EntityMember);

    // 2) Draw overlay text last
    auto& textShader = GLApp::shdrpgms[3];
	// Build FPS string and heaviest system string and render it
    if (!MyEditor::IsEnabled()) {
        auto& AM = Framework::AssetsManager::Instance();
        const Framework::FontData* debugFont = AM.GetFont("liberation-mono_48");

        if (debugFont) {
            std::ostringstream fpsText;
            fpsText << "FPS: " << std::fixed << std::setprecision(1) << GLHelper::fps;

            RenderText(GLApp::shdrpgms[3], fpsText.str(), 100.0f, 850.0f,
                1.0f, glm::vec3(0.5f, 0, 0), debugFont);

            // Add top system below FPS
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

            std::ostringstream topText;
            topText << topSystem << ": " << std::fixed
                << std::setprecision(2) << topPercentage << "%";

            RenderText(GLApp::shdrpgms[3], topText.str(), 100.0f, 880.0f,
                0.8f, glm::vec3(0.5f, 0.5f, 0), debugFont);
        }
        else {
            std::cerr << "[GraphicsManager] WARNING: debugFont not found!\n";
        }
    }


    // Team label
    RenderText(
        textShader,
        "Team BLOOP",
        600.0f, 850.0f,
        1.0f,
        glm::vec3(1, 1, 1),
        m_uiFont
    );

    // FPS counter (while editor hidden)
    if (!MyEditor::IsEnabled()) {
        std::ostringstream fpsText;
        fpsText << "FPS: " << std::fixed
            << std::setprecision(1)
            << GLHelper::fps;

        RenderText(
            textShader,
            fpsText.str(),
            100.0f, 850.0f,
            1.0f,
            glm::vec3(0.5f, 0.0f, 0.0f),
            m_debugFont
        );
    }

    // Controls help, anchored at right side
    {
        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(Engine->GetWindow(), &fbW, &fbH);

        float startX = fbW - 350.0f;
        float startY = fbH - 200.0f;
        float lineHeight = 25.0f;

        RenderText(textShader, "Controls:",
            startX, startY,
            0.5f,
            glm::vec3(1, 1, 0),
            m_debugFont);

        RenderText(textShader, "M - Start monsters",
            startX, startY + lineHeight * 1,
            0.4f,
            glm::vec3(1, 1, 1),
            m_debugFont);

        RenderText(textShader, "W/A/S/D - Input",
            startX, startY + lineHeight * 2,
            0.4f,
            glm::vec3(1, 1, 1),
            m_debugFont);

        RenderText(textShader, "K/L - Scale hero",
            startX, startY + lineHeight * 3,
            0.4f,
            glm::vec3(1, 1, 1),
            m_debugFont);

        RenderText(textShader, "H/J - Rotate monsters",
            startX, startY + lineHeight * 4,
            0.4f,
            glm::vec3(1, 1, 1),
            m_debugFont);

        RenderText(textShader, "O - Toggle editor",
            startX, startY + lineHeight * 5,
            0.4f,
            glm::vec3(1, 1, 1),
            m_debugFont);

        RenderText(textShader, "F1-F6 - Debug draw",
            startX, startY + lineHeight * 6,
            0.4f,
            glm::vec3(1, 1, 1),
            m_debugFont);
    }
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
