/******************************************************************************/
/**
 * @file        glapp.cpp
 * @project     Pulse Protocol
 * @author      Goh Pin Kai
 * @brief		Implements OpenGL application utilities for initializing
				shaders, models, viewports, and rendering textured entities.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#include "glapp.h"
#include "DebugDraw.h"
//#include "../Managers/AssetsManager.h"

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

//static void ApplyEditorViewportPixels(int fbW, int fbH)
//{
//    // derive middle-blank area where the game renders
//    const int assets_h = static_cast<int>(fbH * kBottomRatio);
//    const int top_h    = fbH - assets_h;
//
//    const int left_w   = static_cast<int>(fbW * kLeftRatio);
//    const int right_w  = static_cast<int>(fbW * kRightRatio);
//    const int center_w = fbW - left_w - right_w;
//
//    // OpenGL Y=0 is bottom ? shift up by bottom bar height
//    glViewport(left_w, assets_h, center_w, top_h);
//
//    // Update your globals for any code that needs them
//    GLHelper::GameWidth  = center_w;
//    GLHelper::GameHeight = top_h;
//    GLHelper::GameOffsetX = left_w;
//    GLHelper::GameOffsetY = assets_h;
//
//}



GLApp::GLModel GLApp::mdl;
GLApp::TranslationBuf GLApp::tb;
GLApp::Camera2D GLApp::gCamera{};

void GLApp::init(/*float inputEditorWidth, float inputEditorHeight*/) {
	// --- Compute middle rectangle from window size using fixed ratios ---
	const float kLeftRatio = 0.20f; // 20% width
	const float kRightRatio = 0.25f; // 25% width
	const float kBottomRatio = 0.25f; // 25% height

	//GLint w = static_cast<GLint>(WindowWidth - static_cast<GLint>(inputEditorWidth));
	//GLint h = static_cast<GLint>(WindowHeight - static_cast<GLint>(inputEditorHeight));

	// Use your existing globals for window size
	const int fbW = static_cast<int>(WindowWidth);
	const int fbH = static_cast<int>(WindowHeight);

	// Panels in framebuffer pixels
	const int assets_h = static_cast<int>(fbH * kBottomRatio);
	const int top_h = fbH - assets_h;
	const int left_w = static_cast<int>(fbW * kLeftRatio);
	const int right_w = static_cast<int>(fbW * kRightRatio);
	const int center_w = fbW - left_w - right_w;

	// --- Fit a 16:9 viewport INSIDE the middle area and center it ---
	// Desired aspect = 16:9 ? width/height = 16/9
	const float targetAspect = 16.0f / 9.0f;

	// Start with full middle area
	int vpW = center_w;
	int vpH = top_h;

	// Adjust to preserve 16:9
	if (static_cast<float>(vpH) > static_cast<float>(vpW) / targetAspect) {
		// Too tall ? clamp height
		vpH = static_cast<int>(std::round(vpW / targetAspect));
	}
	else {
		// Too wide ? clamp width
		vpW = static_cast<int>(std::round(vpH * targetAspect));
	}

	// Center the viewport within the middle rect
	const int vpX = left_w + (center_w - vpW) / 2;
	const int vpY = assets_h + (top_h - vpH) / 2;

	// Apply to GL
	glViewport(vpX, vpY, vpW, vpH);

	// Update GLHelper for anyone who needs these numbers
	GLHelper::GameWidth = vpW;
	GLHelper::GameHeight = vpH;
	GLHelper::GameOffsetX = vpX;
	GLHelper::GameOffsetY = vpY;

	// (Optional) keep editor panel sizes if other code references them
	GLHelper::EditorWidth = left_w + right_w;   // total UI width if you need it
	GLHelper::EditorHeight = assets_h;           // bottom bar height if you need it

	// Keep your existing init below
	GLApp::vps.push_back({ 300, 200, vpW, vpH });

	GLApp::VPSS shdr_strs{
		std::make_pair(test_vs, test_fs),
		std::make_pair(bg_vs, bg_fs),
		std::make_pair(sprite_vs, sprite_fs),
		std::make_pair(font_vs, font_fs)
	};
	GLApp::init_shdrpgms_cont(shdr_strs);
	GLApp::init_models_cont();



	bool fixed_seed = true;
	std::random_device rd;
	std::mt19937 gen((fixed_seed ? 123 : rd())); // Fixed seed for reproducibility
	std::uniform_real_distribution<float> dis_translate(-tb.boundaryRange, tb.boundaryRange);
	std::uniform_real_distribution<float> dis_delta(-0.01f, 0.01f);

	// Generate instance data
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


void GLApp::update(GLFWwindow* window, std::set<Entity> entity) {
	GLHelper::update_time(1.0); // updates delta_time and fps
	if (window)
	{
		glfwSetWindowTitle(window, "Pulse Protocol");
	}
	//update(GLHelper::delta_time, entity);
	Framework::Transform transformSystem;
	transformSystem.update(GLHelper::delta_time, entity);

}
void GLApp::updateAnimation(Framework::Animation& x, float deltaTime) {
	float frameDuration = 1.0f / x.animationSpeed;
	x.elapsedTime += deltaTime;

	if (x.elapsedTime >= frameDuration) {
		x.currentFrame = (x.currentFrame + 1) % x.totalFrames;
		x.elapsedTime -= frameDuration;
	}
}


void GLApp::drawTrans(Framework::Renderable& x, Framework::Animation& y,
	Framework::Transform& z, const glm::mat3& ZoomNDC) {

	auto& shader = shdrpgms[static_cast<uint64_t>(x.shd_ref)];
	shader.Use();
	GLuint prog = shader.GetHandle();

	GLint uModelLoc = glGetUniformLocation(prog, "uModel_to_NDC");
	GLint uOffsetLoc = glGetUniformLocation(prog, "uOffset");
	GLint uScaleLoc = glGetUniformLocation(prog, "uScale");

	float frameWidth = 1.0f / y.columns;
	float frameHeight = 1.0f / y.rows;
	float uOffsetX = (y.currentFrame % y.columns) * frameWidth;
	float uOffsetY = 1.0f - ((y.currentFrame / y.columns + 1) * frameHeight);

	if (uOffsetLoc >= 0) glUniform2f(uOffsetLoc, uOffsetX, uOffsetY);
	if (uScaleLoc >= 0) glUniform2f(uScaleLoc, frameWidth, frameHeight);
	//if (uModelLoc >= 0) glUniformMatrix3fv(uModelLoc, 1, GL_FALSE, z.mdl_to_ndc_xform.Begin());

	glm::mat3 M = glm::make_mat3(z.mdl_to_ndc_xform.Begin());
	glm::mat3 mdlNdc = ZoomNDC * M;
	if (uModelLoc >= 0) {
		glUniformMatrix3fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(mdlNdc));
	}
	glBindVertexArray(models[static_cast<uint64_t>(x.mdl_ref)].vaoid);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, x.textureID);
	glDrawElements(models[static_cast<uint64_t>(x.mdl_ref)].primitive_type,
		models[static_cast<uint64_t>(x.mdl_ref)].draw_cnt,
		GL_UNSIGNED_SHORT, nullptr);

	glBindVertexArray(0);
	shader.UnUse();
}

static inline glm::mat3 BuildPanZoomNDC(const GLApp::Camera2D& cam) {
	// column-major: glm::mat3(col0, col1, col2)
	glm::mat3 S(cam.zoom, 0, 0,
		0, cam.zoom, 0,
		0, 0, 1);
	glm::mat3 T(1, 0, 0,
		0, 1, 0,
		cam.center.x, cam.center.y, 1);
	return T * S;   // translate after scaling
}

// Convert a framebuffer pixel to NDC inside the current game viewport
static inline glm::vec2 ScreenToNDC(double x, double y) {
	// x,y are OS pixels from glfwGetCursorPos
	const double rx = (x - GLHelper::GameOffsetX) / (double)GLHelper::GameWidth;
	const double ry = (y - GLHelper::GameOffsetY) / (double)GLHelper::GameHeight;
	// NDC X: 0..1 -> -1..1 ;  NDC Y: flip and map
	return glm::vec2(float(rx * 2.0 - 1.0), float(1.0 - ry * 2.0));
}

void GLApp::draw(std::set<Entity> entities) {
	GLFWwindow* window = glfwGetCurrentContext();

	if (MyEditor::IsEnabled()) {
		// Editor ON - render in ImGui panel
		auto vp = MyEditor::ComputeMiddleGameViewportPixels(window, true);
		glViewport(vp.x, vp.y, vp.w, vp.h);

		GLHelper::GameOffsetX = vp.x;
		GLHelper::GameOffsetY = vp.y;
		GLHelper::GameWidth = vp.w;
		GLHelper::GameHeight = vp.h;
		static double s_scrollY = 0.0;
		static bool   s_scrollInit = false;
		if (!s_scrollInit) {
			glfwSetScrollCallback(window, [](GLFWwindow*, double, double yoff) { s_scrollY += yoff; });
			s_scrollInit = true;
		}

		static double lastX = 0.0, lastY = 0.0;
		double curX, curY; glfwGetCursorPos(window, &curX, &curY);
		static bool first = true;
		if (first) { lastX = curX; lastY = curY; first = false; }

		const bool mmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
		if (mmb) {
			const double dx = curX - lastX;
			const double dy = curY - lastY;
			gCamera.center += glm::vec2(
				float(2.0 * dx / double(GLHelper::GameWidth)),
				float(-2.0 * dy / double(GLHelper::GameHeight))
			);
		}
		lastX = curX; lastY = curY;

		float wheel = float(s_scrollY);
		s_scrollY = 0.0f;
		if (wheel != 0.0f) {
			const float oldZ = gCamera.zoom;
			const float step = 1.12f;
			float newZ = glm::clamp(oldZ * std::pow(step, wheel), gCamera.minZoom, gCamera.maxZoom);

			glm::vec2 cNDC = ScreenToNDC(curX, curY);
			gCamera.center = cNDC - (newZ / oldZ) * (cNDC - gCamera.center);
			gCamera.zoom = newZ;
		}
	}
	else {
		// Editor OFF - use full screen with letterboxing
		int fbW = 0, fbH = 0;
		glfwGetFramebufferSize(window, &fbW, &fbH);

		// Re-apply the correct fullscreen scaling immediately
		ComputeLetterbox(fbW, fbH);
		gCamera.zoom = 1.0f;
		gCamera.center = glm::vec2(0.0f);
	}
	glm::mat3 PanZoom = BuildPanZoomNDC(gCamera);

	// Clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.5f, 0.8f, 1.0f);
	glClearDepth(1.0f);

	// Timing
	auto* g_coordinator = Coordinator::GetInstance();

	static auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
	lastTime = currentTime;

	// Draw entities
	for (auto it : entities) {
		auto& tr = g_coordinator->GetComponent<Framework::Transform>(it);
		auto& re = g_coordinator->GetComponent<Framework::Renderable>(it);
		auto& an = g_coordinator->GetComponent<Framework::Animation>(it);

		if (!re.spriteName.empty() && re.textureID == 0) {
			re.textureID = Framework::AssetsManager::GetOrLoad(re.spriteName);
			if (re.textureID == 0) {
				std::cerr << "[GLApp] Failed to load texture: " << re.spriteName << std::endl;
				continue;
			}
		}

		if (tr.isVisible && re.textureID != 0) {
			updateAnimation(an, deltaTime);
			glBindTexture(GL_TEXTURE_2D, re.textureID);
			drawTrans(re, an, tr, PanZoom);
		}
	}
	GLboolean depthWasOn = glIsEnabled(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);

	if (MyEditor::IsEnabled())
	{
		DebugDraw::Begin(glm::mat4(1.0f));

		for (auto e : entities) {
			if (!g_coordinator->HasComponent<Framework::Transform>(e))   continue;
			if (!g_coordinator->HasComponent<Framework::Renderable>(e))  continue;

			auto& tr = g_coordinator->GetComponent<Framework::Transform>(e);
			auto& re = g_coordinator->GetComponent<Framework::Renderable>(e);

			AABB aabb;
			if (re.mdl_ref == 0) {          // Background quad
				aabb.min = { -1.f, -1.f };
				aabb.max = { 1.f,  1.f };
			}
			else {                           // Sprite quad
				aabb.min = { -0.5f, -0.5f };
				aabb.max = { 0.5f,  0.5f };
			}

			glm::mat3 M = glm::make_mat3(tr.mdl_to_ndc_xform.Begin());
			glm::mat3 modelToNDC = PanZoom * M;

			const bool isHero = re.spriteName.find("hero") != std::string::npos;
			glm::vec3 color = isHero ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);

			DrawAABB_NDC(modelToNDC, aabb, color);
		}

		glLineWidth(2.0f);
		DebugDraw::End();
	}

	if (depthWasOn) glEnable(GL_DEPTH_TEST);
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

	// Use AssetsManager to clean up all textures
	Framework::AssetsManager::Clear();

	// Delete any standalone textures not managed by TextureManager
	if (textureID) {
		glDeleteTextures(1, &textureID);
		textureID = 0;
	}

	GLApp::vps.clear();

	std::cout << "Graphics Resources successfully cleaned up." << std::endl;
}

void GLApp::init_models_cont() {
	GLApp::models.emplace_back(GLApp::bg_model());
	GLApp::models.emplace_back(GLApp::box_model());
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
void GLApp::ComputeLetterbox(int fbw, int fbh)
{
	const float sx = static_cast<float>(fbw) / static_cast<float>(GLApp::VIRTUAL_W);
	const float sy = static_cast<float>(fbh) / static_cast<float>(GLApp::VIRTUAL_H);
	GLApp::PixelScale = std::min(sx, sy);

	const int vw = static_cast<int>(GLApp::VIRTUAL_W * GLApp::PixelScale);
	const int vh = static_cast<int>(GLApp::VIRTUAL_H * GLApp::PixelScale);

	GLApp::GameOffsetX = (fbw - vw) / 2;
	GLApp::GameOffsetY = (fbh - vh) / 2;
	GLApp::GameWidth = vw;
	GLApp::GameHeight = vh;

	glViewport(GLApp::GameOffsetX, GLApp::GameOffsetY, GLApp::GameWidth, GLApp::GameHeight);
}

void GLApp::OnResize(int width, int height)
{
	ComputeLetterbox(width, height);
}

// Convert vec2 in model space to NDC using your existing 2D pipeline
static inline glm::vec2 ToNDC(const glm::mat3& M, const glm::vec2& v) {
	glm::vec3 h = M * glm::vec3(v, 1.0f);
	return { h.x, h.y };               // already NDC
}

// Draw an AABB as 4 lines in NDC
void GLApp::DrawAABB_NDC(const glm::mat3& modelToNDC, const AABB& box, const glm::vec3& color) {
	const glm::vec2 a{ box.min.x, box.min.y };
	const glm::vec2 b{ box.max.x, box.min.y };
	const glm::vec2 c{ box.max.x, box.max.y };
	const glm::vec2 d{ box.min.x, box.max.y };

	glm::vec2 qa = ToNDC(modelToNDC, a);
	glm::vec2 qb = ToNDC(modelToNDC, b);
	glm::vec2 qc = ToNDC(modelToNDC, c);
	glm::vec2 qd = ToNDC(modelToNDC, d);

	DebugDraw::DrawLine(qa, qb, color);
	DebugDraw::DrawLine(qb, qc, color);
	DebugDraw::DrawLine(qc, qd, color);
	DebugDraw::DrawLine(qd, qa, color);
}