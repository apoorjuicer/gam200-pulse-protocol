/*********************************************************************************************
 \file      glapp.cpp
 \par		Bloop
 \author	Goh Pin Kai (pinkai.g@digipen.edu)


\copyright All content   2025 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/
#include "glapp.h"
#include "DebugDraw.h"
//#include "../Managers/AssetsManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/*                                                   objects with file scope
----------------------------------------------------------------------------- */
// define singleton containers
std::vector<GLSLShader> GLApp::shdrpgms;
std::vector<GLApp::GLModel> GLApp::models;
std::vector<GLApp::GLViewport> GLApp::vps;

//other containers
const float PI = 3.14159265358979323846f;
//int polygonmode = 0;
GLuint future_shd_ref = 0; // Initial shader index
GLuint textureID;
const std::string test_vs = {
	#include "shader/test_vs.vert" 
};

const std::string test_fs = {
	#include "shader/test_fs.frag" 
};
const std::string bg_vs = {
	#include "shader/bg_vs.vert" 
};

const std::string bg_fs = {
	#include "shader/bg_fs.frag" 
};

const std::string sprite_vs = {
	#include "shader/sprite_vs.vert" 
};

const std::string sprite_fs = {
	#include "shader/sprite_fs.frag" 
};

const std::string font_vs = {
	#include "shader/font_vs.vert" 
};

const std::string font_fs = {
	#include "shader/font_fs.frag" 
};


double random() {
	return static_cast<double>((rand() % 10) / 10.0f);
}


GLApp::GLModel GLApp::mdl;
GLApp::TranslationBuf GLApp::tb;

GLuint GLApp::LoadTexture(const std::string& filepath) {
	// Log working dir + file existence to catch path mistakes
	try {
		auto cwd = std::filesystem::current_path();
		std::cerr << "[stbi] CWD: " << cwd.string() << "\n";
		std::cerr << "[stbi] exists(" << filepath << "): "
			<< (std::filesystem::exists(filepath) ? "YES" : "NO") << "\n";
	}
	catch (...) {
		std::cerr << "[stbi] std::filesystem not available\n";
	}

	int width = 0, height = 0, nrChannels = 0;

	const int desired_channels = 4;

	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, desired_channels);

	if (!data) {
		std::cerr << "Failed to load texture: " << filepath
			<< " | reason: " << (stbi_failure_reason() ? stbi_failure_reason() : "unknown")
			<< "\n";
		return 0;
	}

	GLuint texID = 0;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	// We asked for RGBA, so upload as RGBA
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);
	return texID;
}

void GLApp::init(float inputEditorWidth, float inputEditorHeight) {
	textureID = LoadTexture("../../MyEngine/Assets/spritesheet 543px x 467px.png");
	// Use local inputEditorWidth and inputEditorHeight values for calculations.
	GLint w = static_cast<GLint>(WindowWidth - static_cast<GLint>(inputEditorWidth)),
		h = static_cast<GLint>(WindowHeight - static_cast<GLint>(inputEditorHeight));

	float a = (9.0f / 16.0f) * w;
	float b = (16.0f / 9.0f) * h;

	if (h > a) {
		h = static_cast<GLint>(a);
	}
	else if (w > b) {
		w = static_cast<GLint>(b);
	}

	// Set OpenGL viewport using the editor width and height provided in the arguments
	glViewport(static_cast<GLint>(inputEditorWidth / 2.f), static_cast<GLint>(inputEditorHeight), w, h);

	// Update GLHelper static members with the values calculated
	GLHelper::GameWidth = w;
	GLHelper::GameHeight = h;
	GLHelper::GameOffsetX = static_cast<GLint>(inputEditorWidth / 2.f);
	GLHelper::GameOffsetY = static_cast<GLint>(inputEditorHeight);
	GLHelper::EditorWidth = static_cast<GLint>(inputEditorWidth);
	GLHelper::EditorHeight = static_cast<GLint>(inputEditorHeight);

	GLApp::vps.push_back({ 300, 200, w, h });
	// Shader Strings Storage
	GLApp::VPSS shdr_strs{
		std::make_pair(test_vs, test_fs),
		std::make_pair(bg_vs, bg_fs),
		std::make_pair(sprite_vs, sprite_fs),
		std::make_pair(font_vs, font_fs)
	};
	GLApp::init_shdrpgms_cont(shdr_strs);
	GLApp::init_models_cont();

	// Initialize debug draw
	DebugDraw::Init();

	bool fixed_seed = true;
	std::random_device rd;
	std::mt19937 gen((fixed_seed ? 123 : rd())); // Fixed seed for taking screenshot and comparison
	std::uniform_real_distribution<float> dis_translate(-tb.boundaryRange, tb.boundaryRange);
	std::uniform_real_distribution<float> dis_delta(-0.01f, 0.01f);

	// Generate and print random values
	for (int i = 0; i < tb.instance_count; ++i) {
		tb.position_scale.push_back(glm::vec4(dis_translate(gen), dis_translate(gen), tb.quad_scale, tb.quad_scale));
		tb.velocity.push_back(glm::vec2(dis_delta(gen), dis_delta(gen)));
	}

	glCreateBuffers(1, &tb.translationBuffer);

	glNamedBufferStorage(tb.translationBuffer, tb.translationBufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);


	GLuint bindingIndex = 0; // Set binding index in shader program
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, tb.translationBuffer);


	glNamedBufferSubData(tb.translationBuffer, 0, tb.translationBufferSize, tb.position_scale.data());
}


void GLApp::update(GLFWwindow* window,std::set<Entity> entity) {
    //if (GLHelper::keystateP == GL_TRUE) {
    //    if (polygonmode == 0) {
    //        ++polygonmode;
    //        GLHelper::keystateP = GL_FALSE;
    //    }
    //    else if (polygonmode == 1) {
    //        ++polygonmode;
    //        GLHelper::keystateP = GL_FALSE;
    //    }
    //    else if (polygonmode == 2) {
    //        polygonmode = 0;
    //        GLHelper::keystateP = GL_FALSE;
    //    }
    //    GLHelper::keystateP = GL_FALSE;
    //}
    GLHelper::update_time(1.0); // updates delta_time and fps
    if (window)
    {
        glfwSetWindowTitle(window, "Pulse Protocol");
    }
     //update(GLHelper::delta_time, entity);
    Framework::Transform transformSystem;
    transformSystem.update(GLHelper::delta_time, entity);
    
}
void GLApp::updateAnimation(Framework::Transform& x, float deltaTime) {
	float frameDuration = 1.0f / x.animationSpeed;
	x.elapsedTime += deltaTime;

	if (x.elapsedTime >= frameDuration) {
		x.currentFrame = (x.currentFrame + 1) % x.totalFrames;
		x.elapsedTime -= frameDuration;
	}
}


void GLApp::drawTrans(Framework::Transform& x) {

	auto& shader = shdrpgms[static_cast<uint64_t>(x.shd_ref)];
	shader.Use();
	GLuint prog = shader.GetHandle();

	GLint uModelLoc = glGetUniformLocation(prog, "uModel_to_NDC");
	GLint uOffsetLoc = glGetUniformLocation(prog, "uOffset");
	GLint uScaleLoc = glGetUniformLocation(prog, "uScale");

	float frameWidth = 1.0f / x.columns;
	float frameHeight = 1.0f / x.rows;
	float uOffsetX = (x.currentFrame % x.columns) * frameWidth;
	float uOffsetY = 1.0f - ((x.currentFrame / x.columns + 1) * frameHeight);

	if (uOffsetLoc >= 0) glUniform2f(uOffsetLoc, uOffsetX, uOffsetY);
	if (uScaleLoc >= 0) glUniform2f(uScaleLoc, frameWidth, frameHeight);
	if (uModelLoc >= 0) glUniformMatrix3fv(uModelLoc, 1, GL_FALSE, x.mdl_to_ndc_xform.Begin());

	glBindVertexArray(models[static_cast<uint64_t>(x.mdl_ref)].vaoid);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, x.textureID);
	glDrawElements(models[static_cast<uint64_t>(x.mdl_ref)].primitive_type,
		models[static_cast<uint64_t>(x.mdl_ref)].draw_cnt,
		GL_UNSIGNED_SHORT, nullptr);

	glBindVertexArray(0);
	shader.UnUse();
}

void GLApp::draw(std::set<Entity> entities) {
	bool pm = false;
	bool fg = false;
	// clear back buffer as before
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.5f, 0.8f, 1.0f);
	glClearDepth(1.0f);
	//shdrpgms[0].Use();
	//transObj.clear();
	auto* g_coordinator = Coordinator::GetInstance();
	//if (auto* tr = g_coordinator->TryGetComponent<Framework::Transform>(it))
	//{
	//    transObj.push_back(tr);
	//    tr->textureID = textureID;
	//}
	static auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
	lastTime = currentTime;

	for (auto it : entities) {
		auto& tr = g_coordinator->GetComponent<Framework::Transform>(it);
		if (tr.textureID == 0) tr.textureID = textureID;
		if (tr.isVisible) {
			updateAnimation(tr, deltaTime);
			glBindTexture(GL_TEXTURE_2D, tr.textureID);
			drawTrans(tr);
		}
	}
}

void GLApp::cleanup() {
	// Delete VAOs, VBOs
	for (auto& model : GLApp::models) {
		if (model.vaoid) {
			glDeleteVertexArrays(1, &model.vaoid);
			model.vaoid = 0;
		}
	}
	GLApp::models.clear();
	GLApp::shdrpgms.clear();

	// Delete translation buffer
	if (tb.translationBuffer) {
		glDeleteBuffers(1, &tb.translationBuffer);
		tb.translationBuffer = 0;
	}

	// Delete textures
	if (textureID) {
		glDeleteTextures(1, &textureID);
		textureID = 0;
	}

	// Clear viewport containers
	GLApp::vps.clear();

	// Debug draw shutdown
	DebugDraw::Shutdown();




	std::cout << "Graphics Resources successfully cleaned up." << std::endl;
}

void GLApp::init_models_cont() {
	GLApp::models.emplace_back(GLApp::bg_model());
	GLApp::models.emplace_back(GLApp::box_model());
	//GLApp::models.emplace_back(GLApp::mystery_model());
}

//Shader Program Container
void GLApp::init_shdrpgms_cont(GLApp::VPSS const& vpss) {
	shdrpgms.clear();
	shdrpgms.reserve(vpss.size());
	for (auto const& x : vpss) {
		GLSLShader shdr_pgm;
		if (!shdr_pgm.CompileShaderFromString(GL_VERTEX_SHADER, x.first)) {
			std::cout << "Vertex shader failed to compile: ";
			std::cout << shdr_pgm.GetLog() << std::endl;
			std::exit(EXIT_FAILURE);
		}
		if (!shdr_pgm.CompileShaderFromString(GL_FRAGMENT_SHADER, x.second)) {
			std::cout << "Fragment shader failed to compile: ";
			std::cout << shdr_pgm.GetLog() << std::endl;
			std::exit(EXIT_FAILURE);
		}
		if (!shdr_pgm.Link()) {
			std::cout << "Shader program failed to link!" << std::endl;
			std::exit(EXIT_FAILURE);
		}
		if (!shdr_pgm.Validate()) {
			std::cout << "Shader program failed to validate!" << std::endl;
			std::exit(EXIT_FAILURE);
		}
		// insert shader program into container
		GLApp::shdrpgms.emplace_back(std::move(shdr_pgm));
	}
}

//Background VAO
GLApp::GLModel GLApp::bg_model() {
	std::vector<GLushort> idx_vtx = { 0, 1, 2, 2, 3, 0 };

	struct Vertex {
		glm::vec2 position;
		glm::vec3 color;
		glm::vec2 texCoord;
	};

	std::array<Vertex, 4> vertices = {
		Vertex{ glm::vec2(1.f, -1.f), glm::vec3(0.f, 1.f, 0.f), glm::vec2(1.0f, 0.0f) },
		Vertex{ glm::vec2(1.f, 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec2(1.0f, 1.0f) },
		Vertex{ glm::vec2(-1.f, 1.f), glm::vec3(1.f, 0.f, 1.f), glm::vec2(0.0f, 1.0f) },
		Vertex{ glm::vec2(-1.f, -1.f), glm::vec3(1.f, 0.f, 0.f), glm::vec2(0.0f, 0.0f) }
	};

	GLuint vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);
	glNamedBufferStorage(vbo_hdl, sizeof(vertices), vertices.data(), GL_DYNAMIC_STORAGE_BIT);

	GLuint ebo_hdl;
	glCreateBuffers(1, &ebo_hdl);
	glNamedBufferStorage(ebo_hdl, sizeof(idx_vtx), idx_vtx.data(), GL_DYNAMIC_STORAGE_BIT);

	glCreateVertexArrays(1, &vaoid);
	glVertexArrayVertexBuffer(vaoid, 0, vbo_hdl, 0, sizeof(Vertex));
	glVertexArrayElementBuffer(vaoid, ebo_hdl);

	glEnableVertexArrayAttrib(vaoid, 0);
	glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
	glVertexArrayAttribBinding(vaoid, 0, 0);

	glEnableVertexArrayAttrib(vaoid, 1);
	glVertexArrayAttribFormat(vaoid, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
	glVertexArrayAttribBinding(vaoid, 1, 0);

	glEnableVertexArrayAttrib(vaoid, 2);
	glVertexArrayAttribFormat(vaoid, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoord));
	glVertexArrayAttribBinding(vaoid, 2, 0);

	GLApp::GLModel quad{};
	quad.vaoid = vaoid;
	quad.primitive_type = GL_TRIANGLES;
	quad.draw_cnt = static_cast<GLuint>(idx_vtx.size());
	quad.primitive_cnt = 4;
	return quad;
}
GLApp::GLModel GLApp::box_model() {
	std::vector<GLushort> idx_vtx = { 0, 1, 2, 2, 3, 0 };

	struct Vertex {
		glm::vec2 position;
		glm::vec3 color;
		glm::vec2 texCoord;
	};

	std::array<Vertex, 4> vertices = {
		Vertex{ glm::vec2(0.5f, -0.5f), glm::vec3(0.f, 1.f, 0.f), glm::vec2(1.0f, 0.0f) },
		Vertex{ glm::vec2(0.5f, 0.5f), glm::vec3(0.f, 0.f, 1.f), glm::vec2(1.0f, 1.0f) },
		Vertex{ glm::vec2(-0.5f, 0.5f), glm::vec3(1.f, 0.f, 1.f), glm::vec2(0.0f, 1.0f) },
		Vertex{ glm::vec2(-0.5f, -0.5f), glm::vec3(1.f, 0.f, 0.f), glm::vec2(0.0f, 0.0f) }
	};

	GLuint vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);
	glNamedBufferStorage(vbo_hdl, sizeof(vertices), vertices.data(), GL_DYNAMIC_STORAGE_BIT);

	GLuint ebo_hdl;
	glCreateBuffers(1, &ebo_hdl);
	glNamedBufferStorage(ebo_hdl, sizeof(idx_vtx), idx_vtx.data(), GL_DYNAMIC_STORAGE_BIT);

	glCreateVertexArrays(1, &vaoid);
	glVertexArrayVertexBuffer(vaoid, 0, vbo_hdl, 0, sizeof(Vertex));
	glVertexArrayElementBuffer(vaoid, ebo_hdl);

	glEnableVertexArrayAttrib(vaoid, 0);
	glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
	glVertexArrayAttribBinding(vaoid, 0, 0);

	glEnableVertexArrayAttrib(vaoid, 1);
	glVertexArrayAttribFormat(vaoid, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
	glVertexArrayAttribBinding(vaoid, 1, 0);

	glEnableVertexArrayAttrib(vaoid, 2);
	glVertexArrayAttribFormat(vaoid, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoord));
	glVertexArrayAttribBinding(vaoid, 2, 0);

	GLApp::GLModel quad{};
	quad.vaoid = vaoid;
	//quad.primitive_type = GL_TRIANGLE_STRIP;
	quad.primitive_type = GL_TRIANGLES;
	quad.draw_cnt = static_cast<GLuint>(idx_vtx.size());

	quad.primitive_cnt = 2;
	return quad;
}

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
//        reinterpret_cast<GLvoid*>(indice_images.data()),
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