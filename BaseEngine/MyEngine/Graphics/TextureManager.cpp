/*********************************************************************************************
 \file      TextureManager .cpp
 \par       Bloop
 \author    Goh Pin Kai (pinkai.g@digipen.edu) 


\copyright All content   2025 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/
#include "TextureManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


static GLuint upload_rgba8(int w,int h,unsigned char* data) {
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

GLuint TextureManager::LoadTextureFile(const std::string& path) {
    int w=0,h=0,n=0;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &n, 4);
    if (!data) {
        std::cerr << "Failed to load texture: " << path
                  << " reason: " << (stbi_failure_reason()? stbi_failure_reason() : "unknown") << "\n";
        return 0;
    }
    GLuint id = upload_rgba8(w,h,data);
    stbi_image_free(data);
    return id;
}

GLuint TextureManager::GetOrLoad(const std::string& path) {
    if (auto it = s_pathToID.find(path); it != s_pathToID.end()) return it->second;
    GLuint id = LoadTextureFile(path);
    if (id) {
        s_pathToID[path] = id;
        s_idToPath[id]   = path;
    }
    return id;
}

const std::string& TextureManager::PathFromID(GLuint texID) {
    return s_idToPath.at(texID); // throws if not found (use .find if you prefer no-throw)
}

void TextureManager::Clear() {
    for (auto& kv : s_pathToID) glDeleteTextures(1, &kv.second);
    s_pathToID.clear();
    s_idToPath.clear();
}


