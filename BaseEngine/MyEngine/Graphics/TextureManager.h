/*********************************************************************************************
 \file      TextureManager.h
 \par       Bloop
 \author    Goh Pin Kai (pinkai.g@digipen.edu) 


\copyright All content   2025 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/
#pragma once
#include "pch/pch.h"

class TextureManager {
public:
    // Returns existing ID if already loaded
    static GLuint GetOrLoad(const std::string& path);

    static const std::string& PathFromID(GLuint texID);   // optional reverse lookup
    static void Clear();                                   // delete all GL textures on shutdown

private:
    static GLuint LoadTextureFile(const std::string& path);

    static inline std::unordered_map<std::string, GLuint> s_pathToID{};
    static inline std::unordered_map<GLuint, std::string> s_idToPath{};
};
