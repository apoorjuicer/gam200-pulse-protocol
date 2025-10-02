///*********************************************************************************************
// \file      Font.h
// \par		Bloop
// \author	Goh Pin Kai (pinkai.g@digipen.edu)
//			Leu Jun Yong (l.junyong)
//
//\copyright All content   2025 DigiPen Institute of Technology Singapore. All rights reserved.
//**********************************************************************************************/
//
//#pragma once
//#include "../pch/pch.h"
//#include <ft2build.h>
//
//#include FT_FREETYPE_H
//
//
//    struct Character {
//        unsigned int TextureID;  // ID handle of the glyph texture
//        glm::ivec2   Size;       // Size of glyph
//        glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
//        unsigned int Advance;    // Offset to advance to next glyph
//    };
//
//    class FontSystem : public Systems {
//    public:
//        FontSystem();
//        ~FontSystem();
//
//        // Initialize the system
//        void Initialize();
//
//        // Update every game frame
//        void Update();
//
//        // Render text at a specified position, scale, and color
//        void RenderText(GLSLShader& shader, std::string text, float x, float y, float scale, glm::vec3 color, std::map<char, Character>& fontCharacters);
//
//        // Provide a string name for debugging
//        std::string GetName();
//
//
//        bool LoadFont(const std::string& fontPath, FT_Face& fontFace, std::map<char, Character>& characterMap);
//
//    private:
//        // FreeType variables
//        FT_Library ft;
//        FT_Face face, face2;
//        glm::mat4 projection;
//        // Map of characters for quick lookup
//        std::map<char, Character> characters;
//
//        // Shader and rendering details
//        GLuint VAO, VBO;
//    };
