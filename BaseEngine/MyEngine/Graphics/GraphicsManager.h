/******************************************************************************/
/**
 * @file        GraphicsManager.h
 * @project     Pulse Protocol
 * @author      Goh Pin Kai (primary) - 70%
 * @author      Leu Jun Yong (secondary) - 30%
 * @brief       Handles high-level rendering:
 *              - World draw via GLApp
 *              - Text rendering overlay (FPS, UI, debug info)
 *              - Window resize notifications
 *              - Message subscription for debug combos
 *
 *              NOTE:
 *              Fonts are owned by Framework::AssetsManager. We just grab
 *              const FontData* and render with it. We do NOT own or free
 *              the glyph textures here.
 *
 *              IMPORTANT:
 *              Transform still lives in namespace Framework, unchanged.
 */
 /******************************************************************************/

#pragma once

#include "pch/pch.h"
#include "glapp.h"
#include "CoreEngine/ECS/System.h"
#include "CoreEngine/ECS/SystemManager.h"
#include "Transform.h"          
#include "../Physics/collision.h"
#include "../CoreEngine/Asset/AssetsManager.h" 


#include <map>
#include <string>
#include <sstream>

class GLApp;

namespace Framework {
    struct FontData; // already defined in AssetsManager.h, but forward-declare is fine in header
}

class GraphicsManager : public Systems
{
public:
    GraphicsManager();
    ~GraphicsManager();

    void Initialize();
    void Update();
    void Shutdown();
    void OnResize(int width, int height);

private:
    // Draw text (world overlay / debug text etc.)
    void RenderText(
        GLSLShader& shader,
        const std::string& text,
        float x, float y,
        float scale,
        const glm::vec3& color,
        const Framework::FontData* fontData
    );

	// For In-Game GUI System
	void RenderGUIButtons();
	void RenderColoredQuad(float x, float y, float width, float height, glm::vec4 color);
private:
    GLApp* glapp = nullptr;

    // Shared quad VAO/VBO for glyph rendering
    GLuint VAO = 0;
    GLuint VBO = 0;

	// Shader and rendering details
	GLuint whiteTexture;

    // Screen-space ortho for text drawing
    glm::mat4 projection{ 1.0f };

    // Cached font refs from AssetsManager
    const Framework::FontData* m_uiFont = nullptr;
    const Framework::FontData* m_debugFont = nullptr;
};
