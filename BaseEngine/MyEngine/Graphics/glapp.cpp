///*********************************************************************************************
// \file      glapp.cpp
// \par		Bloop
// \author	Goh Pin Kai (pinkai.g@digipen.edu)
//
//
//\copyright All content   2024 DigiPen Institute of Technology Singapore. All rights reserved.
//**********************************************************************************************/
////#include "../Engine/Precompiled.h"
//#include "glapp.h"
//#include "iostream"
////#include "../Managers/LevelManager.h"
////#include "../ComponentJson/Component_System.h"
////#include "../Managers/AssetsManager.h"
////#include "../Engine/Core.h"
//
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
//
///*                                                   objects with file scope
//----------------------------------------------------------------------------- */
////using namespace Framework;
//// define singleton containers
//std::vector<GLSLShader> GLApp::shdrpgms;
//std::vector<GLApp::GLModel> GLApp::models;
//std::vector<GLApp::GLViewport> GLApp::vps;
////std::list<Framework::Transform*> transObj;
////std::list<Framework::Transform*> fore;
////std::list<Framework::Transform*> Obj;
////std::list<Framework::Transform*> pause;
//
////other containers
//const float PI = 3.14159265358979323846f;
//int polygonmode = 0;
//GLuint future_shd_ref = 0; // Initial shader index
//GLuint textureID;
//
//const std::string bg_vs = {
//    #include "shader/bg_vs.vert" 
//};
//
//const std::string bg_fs = {
//    #include "shader/bg_fs.frag" 
//};
//
//const std::string sprite_vs = {
//    #include "shader/sprite_vs.vert" 
//};
//
//const std::string sprite_fs = {
//    #include "shader/sprite_fs.frag" 
//};
//
//const std::string font_vs = {
//    #include "shader/font_vs.vert" 
//};
//
//const std::string font_fs = {
//    #include "shader/font_fs.frag" 
//};
//
//
//double random() {
//    return static_cast<double>((rand() % 10) / 10.0f);
//}
//
//
//GLApp::GLModel GLApp::mdl;
//GLApp::TranslationBuf GLApp::tb;
//void GLApp::init(float inputEditorWidth, float inputEditorHeight) {
//
//    // Use local inputEditorWidth and inputEditorHeight values for calculations.
//    GLint w = static_cast<GLint>(GLHelper::width - static_cast<GLint>(inputEditorWidth)),
//        h = static_cast<GLint>(GLHelper::height - static_cast<GLint>(inputEditorHeight));
//
//    float a = (9.0f / 16.0f) * w;
//    float b = (16.0f / 9.0f) * h;
//
//    if (h > a) {
//        h = static_cast<GLint>(a);
//    }
//    else if (w > b) {
//        w = static_cast<GLint>(b);
//    }
//
//    // Set OpenGL viewport using the editor width and height provided in the arguments
//    glViewport(static_cast<GLint>(inputEditorWidth / 2.f), static_cast<GLint>(inputEditorHeight), w, h);
//
//    // Update GLHelper static members with the values calculated
//    GLHelper::GameWidth = w;
//    GLHelper::GameHeight = h;
//    GLHelper::GameOffsetX = static_cast<GLint>(inputEditorWidth / 2.f);
//    GLHelper::GameOffsetY = static_cast<GLint>(inputEditorHeight);
//    GLHelper::EditorWidth = static_cast<GLint>(inputEditorWidth);
//    GLHelper::EditorHeight = static_cast<GLint>(inputEditorHeight);
//
//    GLApp::vps.push_back({ 300, 200, w, h });
//
//    // Shader Strings Storage
//    GLApp::VPSS shdr_strs{
//        std::make_pair(bg_vs, bg_fs),
//        std::make_pair(sprite_vs, sprite_fs),
//        std::make_pair(font_vs, font_fs)
//    };
//    GLApp::init_shdrpgms_cont(shdr_strs);
//    GLApp::init_models_cont();
//
//    bool fixed_seed = true;
//    std::random_device rd;
//    std::mt19937 gen((fixed_seed ? 123 : rd())); // Fixed seed for taking screenshot and comparison
//    std::uniform_real_distribution<float> dis_translate(-tb.boundaryRange, tb.boundaryRange);
//    std::uniform_real_distribution<float> dis_delta(-0.01f, 0.01f);
//
//    // Generate and print random values
//    for (int i = 0; i < tb.instance_count; ++i) {
//        tb.position_scale.push_back(glm::vec4(dis_translate(gen), dis_translate(gen), tb.quad_scale, tb.quad_scale));
//        tb.velocity.push_back(glm::vec2(dis_delta(gen), dis_delta(gen)));
//    }
//
//    glCreateBuffers(1, &tb.translationBuffer);
//
//
//    GLbitfield flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT; // Adjust flags based on usage
//    glNamedBufferStorage(tb.translationBuffer, tb.translationBufferSize, nullptr, flags);
//
//
//    GLuint bindingIndex = 0; // Set binding index in shader program
//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, tb.translationBuffer);
//
//
//    glNamedBufferSubData(tb.translationBuffer, 0, tb.translationBufferSize, tb.position_scale.data());
//}
//
//
//void GLApp::update() {
//    if (GLHelper::keystateP == GL_TRUE) {
//        if (polygonmode == 0) {
//            ++polygonmode;
//
//            GLHelper::keystateP = GL_FALSE;
//
//        }
//        else if (polygonmode == 1) {
//            ++polygonmode;
//            GLHelper::keystateP = GL_FALSE;
//        }
//        else if (polygonmode == 2) {
//            polygonmode = 0;
//            GLHelper::keystateP = GL_FALSE;
//        }
//        GLHelper::keystateP = GL_FALSE;
//    }
//
//    for (auto& x : transObj) {
//        x->update(GLHelper::delta_time);
//    }
//    for (auto& x : Obj) {
//        x->update(GLHelper::delta_time);
//    }
//    for (auto& x : fore) {
//        x->update(GLHelper::delta_time);
//    }
//    for (auto& x : pause) {
//        x->update(GLHelper::delta_time);
//    }
//}
//
//void GLApp::updateAnimation(Framework::Transform& x) {
//    float frameDuration = 1.0f / x.animationSpeed; // Time per frame
//
//    // Accumulate elapsed time
//    x.elapsedTime += (float)GLHelper::delta_time;
//
//    // Update frame only if enough time has passed
//    if (x.elapsedTime >= frameDuration) {
//        x.currentFrame = (x.currentFrame + 1) % x.totalFrames; // Loop through frames
//        x.elapsedTime -= frameDuration; // Handle overshoot
//    }
//}
//
//void GLApp::drawTrans(Framework::Transform x) {
//    GLint uniform_var_loc1 = 0;
//    GLint uOffsetLoc = 0;
//    GLint uScaleLoc = 0;
//    //// Part 1
//    shdrpgms[static_cast<uint64_t>(x.shd_ref)].Use();
//
//    //// Part 2
//    glBindVertexArray(models[static_cast<uint64_t>(x.mdl_ref)].vaoid);
//
//    x.totalFrames = x.rows * x.columns;
//
//    float frameWidth = 1.0f / x.columns;
//    float frameHeight = 1.0f / x.rows;
//    // Calculate current frame offset
//    float uOffsetX = (x.currentFrame % x.columns) * frameWidth;
//    float uOffsetY = 1.0f - ((x.currentFrame / x.columns + 1) * frameHeight); // Vertical offset (top-to-bottom)
//
//    //// Part 3
//    if (x.shd_ref == 1) {
//        uOffsetLoc = glGetUniformLocation(shdrpgms[static_cast<uint64_t>(x.shd_ref)].GetHandle(), "uOffset");
//        uScaleLoc = glGetUniformLocation(shdrpgms[static_cast<uint64_t>(x.shd_ref)].GetHandle(), "uScale");
//        uniform_var_loc1 = glGetUniformLocation(shdrpgms[static_cast<uint64_t>(x.shd_ref)].GetHandle(), "uModel_to_NDC");
//    }
//    if (uOffsetLoc >= 0)
//        glUniform2f(uOffsetLoc, uOffsetX, uOffsetY);
//
//    if (uScaleLoc >= 0)
//        glUniform2f(uScaleLoc, frameWidth, frameHeight);
//    if (uniform_var_loc1 >= 0) {
//        glm::mat3x3 mdl_to_ndc_xform = static_cast<glm::mat3x3>(x.mdl_to_ndc_xform);
//        glUniformMatrix3fv(uniform_var_loc1, 1, GL_FALSE, glm::value_ptr(mdl_to_ndc_xform));
//    }
//    else
//    {
//        std::cout << "Uniform variable doesn't exist!!!\n";
//        std::exit(EXIT_FAILURE);
//    }
//    glDrawElements(models[static_cast<uint64_t>(x.mdl_ref)].primitive_type, models[static_cast<uint64_t>(x.mdl_ref)].draw_cnt, GL_UNSIGNED_SHORT, NULL);
//    //// Part 5
//    glBindVertexArray(0);
//    shdrpgms[static_cast<uint64_t>(x.shd_ref)].UnUse();
//}
//
//void GLApp::draw() {
//    bool pm = false;
//    bool fg = false;
//    // clear back buffer as before
//    glClear(GL_COLOR_BUFFER_BIT);
//    glClear(GL_DEPTH_BUFFER_BIT);
//    shdrpgms[0].Use();
//    transObj.clear();
//    Obj.clear();
//    pause.clear();
//    fore.clear();
//
//    for (auto it = Framework::levelManager->GameObjects.GOCM.begin(); it != Framework::levelManager->GameObjects.GOCM.end(); it++) {
//        Transform* t = dynamic_cast<Transform*>(it->second->GetCom(Com_Trans));
//
//        if (t) {
//            t->update(0.f);
//            Player* p = dynamic_cast<Player*>(t->GetOwner()->GetCom(Com_Player));
//            Cat* c = dynamic_cast<Cat*>(t->GetOwner()->GetCom(Com_Cat));
//            pm = t->GetOwner()->Name == "Pausepopup menu " || t->GetOwner()->Name == "Pausepopup Resume button"
//                || t->GetOwner()->Name == "Pausepopup Exit button";
//            fg = t->GetOwner()->Name == "Level1_Foreground";
//            if (p || c) {
//                Obj.push_back(t); // Add the current Transform* to the vector
//            }
//            else if (pm) {
//                pause.push_back(t);
//            }
//            else if (fg) {
//                fore.push_back(t);
//            }
//            else {
//                transObj.push_back(t); // Add the current Transform* to the vector
//            }
//
//            std::vector<Framework::Transform*> vec(Obj.begin(), Obj.end());
//            std::sort(vec.begin(), vec.end(), [](Framework::Transform* a, Framework::Transform* b) {
//                return a->Pos.getY() > b->Pos.getY();
//                });
//            Obj.assign(vec.begin(), vec.end()); // Convert back to list if needed
//        }
//    }
//
//    // Render the transforms after collecting them
//    for (auto x : transObj) {
//        updateAnimation(*x);
//        glBindTexture(GL_TEXTURE_2D, x->textureID);
//        drawTrans(*x);
//    }
//    for (auto x : Obj) {
//        updateAnimation(*x);
//        glBindTexture(GL_TEXTURE_2D, x->textureID);
//        drawTrans(*x);
//    }
//    for (auto x : fore) {
//        updateAnimation(*x);
//        glBindTexture(GL_TEXTURE_2D, x->textureID);
//        drawTrans(*x);
//    }
//    for (auto x : pause) {
//        updateAnimation(*x);
//        glBindTexture(GL_TEXTURE_2D, x->textureID);
//        drawTrans(*x);
//    }
//
//
//    if (polygonmode == 0)
//    {
//        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//    }
//    else if (polygonmode == 1)
//    {
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//        glLineWidth(3.f);
//    }
//    else if (polygonmode == 2)
//    {
//        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
//        glPointSize(10.f); // Reset back to 1
//    }
//
//
//
//
//}
//
//void GLApp::cleanup() {
//    // Delete VAOs, VBOs
//    for (auto& model : GLApp::models) {
//        if (model.vaoid) {
//            glDeleteVertexArrays(1, &model.vaoid);
//            model.vaoid = 0;
//        }
//    }
//    GLApp::models.clear();
//    GLApp::shdrpgms.clear();
//
//    // Delete translation buffer
//    if (tb.translationBuffer) {
//        glDeleteBuffers(1, &tb.translationBuffer);
//        tb.translationBuffer = 0;
//    }
//
//    // Delete textures
//    if (textureID) {
//        glDeleteTextures(1, &textureID);
//        textureID = 0;
//    }
//
//    // Clear viewport containers
//    GLApp::vps.clear();
//
//    std::cout << "Graphics Resources successfully cleaned up." << std::endl;
//}
//
//void GLApp::init_models_cont() {
//    GLApp::models.emplace_back(GLApp::bg_model());
//    GLApp::models.emplace_back(GLApp::box_model());
//    GLApp::models.emplace_back(GLApp::mystery_model());
//}
//
////Shader Program Container
//void GLApp::init_shdrpgms_cont(GLApp::VPSS const& vpss) {
//    for (auto const& x : vpss) {
//        GLSLShader shdr_pgm;
//        if (!shdr_pgm.CompileShaderFromString(GL_VERTEX_SHADER, x.first)) {
//            std::cout << "Vertex shader failed to compile: ";
//            std::cout << shdr_pgm.GetLog() << std::endl;
//            std::exit(EXIT_FAILURE);
//        }
//        if (!shdr_pgm.CompileShaderFromString(GL_FRAGMENT_SHADER, x.second)) {
//            std::cout << "Fragment shader failed to compile: ";
//            std::cout << shdr_pgm.GetLog() << std::endl;
//            std::exit(EXIT_FAILURE);
//        }
//        if (!shdr_pgm.Link()) {
//            std::cout << "Shader program failed to link!" << std::endl;
//            std::exit(EXIT_FAILURE);
//        }
//        if (!shdr_pgm.Validate()) {
//            std::cout << "Shader program failed to validate!" << std::endl;
//            std::exit(EXIT_FAILURE);
//        }
//        // insert shader program into container
//        GLApp::shdrpgms.emplace_back(shdr_pgm);
//    }
//}
//
////Background VAO
//GLApp::GLModel GLApp::bg_model() {
//    std::vector<GLushort> idx_vtx = { 0, 1, 2, 2, 3, 0 };
//
//    struct Vertex {
//        glm::vec2 position;
//        glm::vec3 color;
//        glm::vec2 texCoord;
//    };
//
//    std::array<Vertex, 4> vertices = {
//        Vertex{ glm::vec2(1.f, -1.f), glm::vec3(0.f, 1.f, 0.f), glm::vec2(1.0f, 0.0f) },
//        Vertex{ glm::vec2(1.f, 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec2(1.0f, 1.0f) },
//        Vertex{ glm::vec2(-1.f, 1.f), glm::vec3(1.f, 0.f, 1.f), glm::vec2(0.0f, 1.0f) },
//        Vertex{ glm::vec2(-1.f, -1.f), glm::vec3(1.f, 0.f, 0.f), glm::vec2(0.0f, 0.0f) }
//    };
//
//    GLuint vbo_hdl;
//    glCreateBuffers(1, &vbo_hdl);
//    glNamedBufferStorage(vbo_hdl, sizeof(vertices), vertices.data(), GL_DYNAMIC_STORAGE_BIT);
//
//    GLuint ebo_hdl;
//    glCreateBuffers(1, &ebo_hdl);
//    glNamedBufferStorage(ebo_hdl, sizeof(idx_vtx), idx_vtx.data(), GL_DYNAMIC_STORAGE_BIT);
//
//    glCreateVertexArrays(1, &vaoid);
//    glVertexArrayVertexBuffer(vaoid, 0, vbo_hdl, 0, sizeof(Vertex));
//    glVertexArrayElementBuffer(vaoid, ebo_hdl);
//
//    glEnableVertexArrayAttrib(vaoid, 0);
//    glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
//    glVertexArrayAttribBinding(vaoid, 0, 0);
//
//    glEnableVertexArrayAttrib(vaoid, 1);
//    glVertexArrayAttribFormat(vaoid, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
//    glVertexArrayAttribBinding(vaoid, 1, 0);
//
//    glEnableVertexArrayAttrib(vaoid, 2);
//    glVertexArrayAttribFormat(vaoid, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoord));
//    glVertexArrayAttribBinding(vaoid, 2, 0);
//
//    GLApp::GLModel quad{};
//    quad.vaoid = vaoid;
//    quad.primitive_type = GL_TRIANGLES;
//    quad.draw_cnt = static_cast<GLuint>(idx_vtx.size());
//    quad.primitive_cnt = 4;
//    return quad;
//}
//GLApp::GLModel GLApp::box_model() {
//    std::vector<GLushort> idx_vtx = { 0, 1, 2, 2, 3, 0 };
//
//    struct Vertex {
//        glm::vec2 position;
//        glm::vec3 color;
//        glm::vec2 texCoord;
//    };
//
//    std::array<Vertex, 4> vertices = {
//        Vertex{ glm::vec2(0.5f, -0.5f), glm::vec3(0.f, 1.f, 0.f), glm::vec2(1.0f, 0.0f) },
//        Vertex{ glm::vec2(0.5f, 0.5f), glm::vec3(0.f, 0.f, 1.f), glm::vec2(1.0f, 1.0f) },
//        Vertex{ glm::vec2(-0.5f, 0.5f), glm::vec3(1.f, 0.f, 1.f), glm::vec2(0.0f, 1.0f) },
//        Vertex{ glm::vec2(-0.5f, -0.5f), glm::vec3(1.f, 0.f, 0.f), glm::vec2(0.0f, 0.0f) }
//    };
//
//    GLuint vbo_hdl;
//    glCreateBuffers(1, &vbo_hdl);
//    glNamedBufferStorage(vbo_hdl, sizeof(vertices), vertices.data(), GL_DYNAMIC_STORAGE_BIT);
//
//    GLuint ebo_hdl;
//    glCreateBuffers(1, &ebo_hdl);
//    glNamedBufferStorage(ebo_hdl, sizeof(idx_vtx), idx_vtx.data(), GL_DYNAMIC_STORAGE_BIT);
//
//    glCreateVertexArrays(1, &vaoid);
//    glVertexArrayVertexBuffer(vaoid, 0, vbo_hdl, 0, sizeof(Vertex));
//    glVertexArrayElementBuffer(vaoid, ebo_hdl);
//
//    glEnableVertexArrayAttrib(vaoid, 0);
//    glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
//    glVertexArrayAttribBinding(vaoid, 0, 0);
//
//    glEnableVertexArrayAttrib(vaoid, 1);
//    glVertexArrayAttribFormat(vaoid, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
//    glVertexArrayAttribBinding(vaoid, 1, 0);
//
//    glEnableVertexArrayAttrib(vaoid, 2);
//    glVertexArrayAttribFormat(vaoid, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoord));
//    glVertexArrayAttribBinding(vaoid, 2, 0);
//
//    GLApp::GLModel quad{};
//    quad.vaoid = vaoid;
//    quad.primitive_type = GL_TRIANGLE_STRIP;
//    quad.draw_cnt = static_cast<GLuint>(idx_vtx.size());
//    quad.primitive_cnt = 2;
//    return quad;
//}
//
//GLApp::GLModel GLApp::mystery_model()
//{
//    // Define the vertices to create a hexagon
//    std::array<glm::vec2, 7> vertPos{
//        glm::vec2(-0.5f, -0.86f),  // Bottom left
//        glm::vec2(0.5f, -0.86f),   // Bottom right
//        glm::vec2(1.0f, 0.0f),     // Right
//        glm::vec2(0.5f, 0.86f),    // Top right
//        glm::vec2(-0.5f, 0.86f),   // Top left
//        glm::vec2(-1.0f, 0.0f),    // Left
//        glm::vec2(0.0f, 0.0f)      // Center
//    };
//
//    // Define the indices to draw the hexagon using triangles
//    std::array<GLushort, 18> indices{
//        0, 1, 6,   // Bottom triangle
//        1, 2, 6,   // Bottom right triangle
//        2, 3, 6,   // Top right triangle
//        3, 4, 6,   // Top triangle
//        4, 5, 6,   // Top left triangle
//        5, 0, 6    // Bottom left triangle
//    };
//
//    // Define colors for each vertex
//    std::array<glm::vec3, 7> clr_vtx{
//        glm::vec3(random(), random(), random()),
//        glm::vec3(random(), random(), random()),
//        glm::vec3(random(), random(), random()),
//        glm::vec3(random(), random(), random()),
//        glm::vec3(random(), random(), random()),
//        glm::vec3(random(), random(), random()),
//        glm::vec3(random(), random(), random())
//    };
//    // create GL Unsigned Int
//    GLuint vbo1;
//
//    // Create 1 Buffer using vbo1
//    glCreateBuffers(1, &vbo1);
//
//    // assign memory
//    glNamedBufferStorage(vbo1,
//        sizeof(glm::vec2) * vertPos.size() + sizeof(glm::vec3) * clr_vtx.size(),
//        nullptr, GL_DYNAMIC_STORAGE_BIT);
//
//    glNamedBufferSubData(vbo1, 0,
//        sizeof(glm::vec2) * vertPos.size(), vertPos.data());
//
//    glNamedBufferSubData(vbo1, sizeof(glm::vec2) * vertPos.size(),
//        sizeof(glm::vec3) * clr_vtx.size(), clr_vtx.data());
//
//    // create GL Unsigned Int
//    GLuint qaoid;
//    // set instructions for position 0
//    glCreateVertexArrays(1, &qaoid);
//    glEnableVertexArrayAttrib(qaoid, 0);
//    glVertexArrayVertexBuffer(qaoid, 0, vbo1, 0, sizeof(glm::vec2));
//
//    glVertexArrayAttribFormat(qaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
//    glVertexArrayAttribBinding(qaoid, 0, 0);
//
//    // set instructions for colour 1
//    glEnableVertexArrayAttrib(qaoid, 1);
//    glVertexArrayVertexBuffer(qaoid, 1, vbo1, 0, sizeof(glm::vec3));
//    // set value to 3 because RGB
//    glVertexArrayAttribFormat(qaoid, 1, 3, GL_FLOAT, GL_FALSE, 0);
//    glVertexArrayAttribBinding(qaoid, 1, 1);
//
//    GLuint ebo;
//    glCreateBuffers(1, &ebo);
//    glNamedBufferStorage(ebo, sizeof(GLushort) * indices.size(),
//        reinterpret_cast<GLvoid*>(indices.data()),
//        GL_DYNAMIC_STORAGE_BIT);
//    glVertexArrayElementBuffer(qaoid, ebo);
//    glBindVertexArray(0);
//
//    GLApp::GLModel quad{};
//    quad.vaoid = vaoid;
//    quad.primitive_type = GL_TRIANGLES;
//    quad.draw_cnt = static_cast<GLuint>(indices.size());
//    quad.primitive_cnt = 4;
//    return quad;
//}