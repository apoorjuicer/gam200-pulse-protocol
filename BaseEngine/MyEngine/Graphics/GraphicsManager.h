/******************************************************************************/
/**
 * @file        GraphicsManager.cpp
 * @project     Pulse Protocol
 * @author		Goh Pin Kai (primary) - 70%
 * @author		Leu Jun Yong (secondary) - 30% each
 * @brief       Declares the GraphicsManager, which sets up and manages
 *              OpenGL rendering, FreeType-based text rendering, message
 *              subscriptions, and the interface for initialization,
 *              per-frame updates, and cleanup of graphics resources.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/ 

#pragma once
#include "pch/pch.h"
#include "glapp.h"
#include "CoreEngine/ECS/System.h"
#include "CoreEngine/ECS/SystemManager.h"
#include <iostream>
#include <filesystem>
#include "stb_image.h"
#include <chrono>
#include <vector>
#include <string.h>
#include "Transform.h"
#include"../Physics/collision.h"

#include <ft2build.h> 
#include FT_FREETYPE_H 


class GLApp;
namespace Framework { class Transform; }
namespace Framework { class Systems; }

struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
};


class GraphicsManager : public Systems
{
public:
	GraphicsManager();
	~GraphicsManager(); // destructor



	void Initialize();
	void Update();
	void Shutdown();

	bool LoadFont(const std::string& fontPath, FT_Face& fontFace, std::map<char, Character>& characterMap);
	void RenderText(GLSLShader& shader, std::string text, float x, float y, float scale, glm::vec3 color, std::map<char, Character>& fontCharacters);

private:
	GLApp* glapp;


	// FreeType variables
	FT_Library ft;
	FT_Face face, face2;
	glm::mat4 projection;
	// Map of characters for quick lookup
	std::map<char, Character> characters;

	// Shader and rendering details
	GLuint VAO, VBO;

};

