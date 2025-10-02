///*********************************************************************************************
// \file      Font.cpp
// \par		Bloop
// \author    Leu Jun Yong (l.junyong@digipen.edu) 
//            Goh Pin Kai (pinkai.g@digipen.edu) 
//
//
//\copyright All content   2025 DigiPen Institute of Technology Singapore. All rights reserved.
//**********************************************************************************************/
//
//
//#include "Graphics/glapp.h"
//#include "Font.h"
//#include "FontObj.h"
//
//    FontSystem::FontSystem() : ft(nullptr), face(nullptr), face2(nullptr), VAO(0), VBO(0) {}
//    std::map<char, Character> Characters;
//    std::map<char, Character> Characters2; // Map for the second font
//    //FT_Library ft;
//
//    void FontSystem::Initialize() {
//        // Initialize FreeType
//        if (FT_Init_FreeType(&ft)) {
//            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
//            return;
//        }
//
//        // Enable blending for text rendering with transparency
//        glEnable(GL_BLEND);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//        // Set up the orthographic projection matrix
//        projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
//
//        // Load the first font
//        std::string fontPath1 = "../../MyEngine/Assets/PatrickHand-Regular.ttf";
//        if (!LoadFont(fontPath1, face, Characters)) {
//            std::cout << "ERROR::FREETYPE: Failed to load first font" << std::endl;
//            return;
//        }
//
//        // Load the second font
//        std::string fontPath2 = "../../MyEngine/Assets/liberation-mono.ttf";
//        if (!LoadFont(fontPath2, face2, Characters2)) {
//            std::cout << "ERROR::FREETYPE: Failed to load second font" << std::endl;
//            return;
//        }
//
//        // Generate and configure VAO/VBO for text rendering
//        glGenVertexArrays(1, &VAO);
//        glGenBuffers(1, &VBO);
//        glBindVertexArray(VAO);
//        glBindBuffer(GL_ARRAY_BUFFER, VBO);
//        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
//        glEnableVertexAttribArray(0);
//        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);
//        glBindVertexArray(0);
//    }
//
//    void FontSystem::RenderText(GLSLShader& shader, std::string text, float x, float y, float scale, glm::vec3 color, std::map<char, Character>& fontCharacters) {
//        shader.Use();
//        glUniformMatrix4fv(glGetUniformLocation(shader.GetHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
//        glUniform3f(glGetUniformLocation(shader.GetHandle(), "textColor"), color.x, color.y, color.z);
//        glActiveTexture(GL_TEXTURE0);
//        glBindVertexArray(VAO);
//
//        for (auto c : text) {
//            Character ch = fontCharacters[c]; // Use the fontCharacters parameter instead of Characters
//            float xpos = x + ch.Bearing.x * scale;
//            float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
//            float w = ch.Size.x * scale;
//            float h = ch.Size.y * scale;
//            float vertices[6][4] = {
//                { xpos,     ypos + h,   0.0f, 0.0f },
//                { xpos,     ypos,       0.0f, 1.0f },
//                { xpos + w, ypos,       1.0f, 1.0f },
//
//                { xpos,     ypos + h,   0.0f, 0.0f },
//                { xpos + w, ypos,       1.0f, 1.0f },
//                { xpos + w, ypos + h,   1.0f, 0.0f }
//            };
//            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
//            glBindBuffer(GL_ARRAY_BUFFER, VBO);
//            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
//            glBindBuffer(GL_ARRAY_BUFFER, 0);
//            glDrawArrays(GL_TRIANGLES, 0, 6);
//            x += (ch.Advance >> 6) * scale;
//        }
//        glBindVertexArray(0);
//        glBindTexture(GL_TEXTURE_2D, 0);
//    }
//
//    void FontSystem::Update() {
//        RenderText(GLApp::shdrpgms[2], "Playing", 250.0f, 25.0f, 1.0f, glm::vec3(0.25f, 0.88f, 0.82f), Characters);
//        //RenderText(GLApp::shdrpgms[2], "RightClickMe", 230.0f, 280.0f, 1.0f, glm::vec3(0.5f, 0.0f, 0.0f), Characters2); // Use second font
//    }
//
//    FontSystem::~FontSystem() {
//        if (face)  FT_Done_Face(face);
//        if (face2) FT_Done_Face(face2);
//        if (ft)    FT_Done_FreeType(ft);
//        for (auto& [key, ch] : Characters) glDeleteTextures(1, &ch.TextureID);
//        for (auto& [key, ch] : Characters2) glDeleteTextures(1, &ch.TextureID);
//        glDeleteVertexArrays(1, &VAO);
//        glDeleteBuffers(1, &VBO);
//    }
//
//    std::string FontSystem::GetName() {
//        return "FontSystem";
//    }
//
//    bool FontSystem::LoadFont(const std::string& fontPath, FT_Face& fontFace, std::map<char, Character>& characterMap) {
//        if (FT_New_Face(this->ft, fontPath.c_str(), 0, &fontFace)) {
//            return false; // Font loading failed
//        }
//        FT_Set_Pixel_Sizes(fontFace, 0, 48);
//        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//
//        for (unsigned char c = 0; c < 128; c++) {
//            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;
//            unsigned int texture;
//            glGenTextures(1, &texture);
//            glBindTexture(GL_TEXTURE_2D, texture);
//            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//            Character character = {
//                texture,
//                glm::ivec2(static_cast<unsigned>(face->glyph->bitmap.width), static_cast<unsigned>(face->glyph->bitmap.rows)),
//                glm::ivec2(static_cast<unsigned>(face->glyph->bitmap_left), static_cast<unsigned>(face->glyph->bitmap_top)),
//                static_cast<unsigned>(face->glyph->advance.x)
//            };
//            characterMap.insert(std::pair<char, Character>(c, character));
//        }
//        return true; // Font loading succeeded
//    }