/******************************************************************************/
/**
 * @file        glapp.cpp
 * @project     Pulse Protocol
 * @author      Goh Pin Kai (primary) - 90%
 *			    Reginald Lew Yee Ren (secondary) - 10%
 * @brief		Implements OpenGL application utilities for initializing
 *				shaders, models, viewports, and rendering textured entities.
 * 
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#include "glapp.h"
#include <filesystem>
#include "GameState.h"
/*                                                   objects with file scope
----------------------------------------------------------------------------- */
// define singleton containers
std::vector<GLSLShader> GLApp::shdrpgms;
std::vector<GLApp::GLViewport> GLApp::vps;

GLuint textureID;
static GLApp::BatchRender gBatch;

namespace fs = std::filesystem;

static std::string LoadFile(const char* relativeToGraphicsShader)
{
	fs::path here = fs::path(__FILE__).parent_path(); // ...\PulseEngine\Graphics
	fs::path full = here / "shader" / relativeToGraphicsShader;
	std::ifstream f(full, std::ios::binary);
	if (!f)
	{
		std::cerr << "[Shader] Open failed: " << full << "\n";
		return {};
	}
	std::ostringstream ss;
	ss << f.rdbuf();
	return ss.str();
}

const std::string test_vs = LoadFile("test_vs.vert");
const std::string test_fs = LoadFile("test_fs.frag");
const std::string bg_vs = LoadFile("bg_vs.vert");
const std::string bg_fs = LoadFile("bg_fs.frag");
const std::string sprite_vs = LoadFile("sprite_vs.vert");
const std::string sprite_fs = LoadFile("sprite_fs.frag");
const std::string font_vs = LoadFile("font_vs.vert");
const std::string font_fs = LoadFile("font_fs.frag");

double random()
{
	return static_cast<double>((rand() % 10) / 10.0f);
}

GLApp::Camera2D GLApp::gEditorCamera{};
GLApp::Camera2D GLApp::gGameCamera{};

void GLApp::init(/*float inputEditorWidth, float inputEditorHeight*/)
{
	gBatch.Init(/*maxSprites*/ 10000);
	
	GLApp::VPSS shdr_strs{
		std::make_pair(test_vs, test_fs),
		std::make_pair(bg_vs, bg_fs),
		std::make_pair(sprite_vs, sprite_fs),
		std::make_pair(font_vs, font_fs) };
	GLApp::init_shdrpgms_cont(shdr_strs);
}


void GLApp::update(GLFWwindow* window, std::set<Entity> entity) {
	if (window)
	{
		glfwSetWindowTitle(window, "Pulse Protocol");
	}
	// update(GLHelper::delta_time, entity);
	Framework::Transform transformSystem;
	transformSystem.update(GLHelper::delta_time, entity);
}
void GLApp::updateAnimation(Framework::Animation& x, float deltaTime)
{
	if (x.totalFrames <= 1 || x.animationSpeed <= 0.0f)
		return;
	float frameDuration = 1.0f / x.animationSpeed;
	x.elapsedTime += deltaTime;
	while (x.elapsedTime >= frameDuration)
	{
		x.currentFrame = (x.currentFrame + 1) % x.totalFrames;
		x.elapsedTime -= frameDuration;
	}
}

inline AABB PixelsToModelAABB(const AABB& bb, int texW, int texH) {
	AABB out = bb;
	if (texW <= 0 || texH <= 0) {            // <- prevent infs
		out.min = { -0.5f, -0.5f };
		out.max = { 0.5f,  0.5f };
		return out;
	}

	glm::vec2 minPx(bb.min.x, bb.min.y);
	glm::vec2 maxPx(bb.max.x, bb.max.y);

	glm::vec2 texHalf(texW * 0.5f, texH * 0.5f);
	glm::vec2 invTex(1.0f / float(texW), 1.0f / float(texH));

	glm::vec2 minLocal = (minPx - texHalf) * invTex;
	glm::vec2 maxLocal = (maxPx - texHalf) * invTex;

	out.min.x = minLocal.x; out.min.y = minLocal.y;
	out.max.x = maxLocal.x; out.max.y = maxLocal.y;
	return out;
}

// Convert a framebuffer pixel to NDC inside the current game viewport
glm::vec2 GLApp::ScreenToNDC(double x, double y) {
	// x,y are OS pixels from glfwGetCursorPos
	const double rx = (x - GLHelper::GameOffsetX) / (double)GLHelper::GameWidth;
	const double ry = (y - GLHelper::GameOffsetY) / (double)GLHelper::GameHeight;
	// NDC X: 0..1 -> -1..1 ;  NDC Y: flip and map
	return glm::vec2(float(rx * 2.0 - 1.0), float(1.0 - ry * 2.0));
}

glm::mat3 GLApp::BuildPanZoomNDC(const GLApp::Camera2D& cam) {
	// column-major: glm::mat3(col0, col1, col2)
	glm::mat3 S(cam.zoom, 0, 0,
		0, cam.zoom, 0,
		0, 0, 1);
	glm::mat3 T(1, 0, 0,
		0, 1, 0,
		cam.center.x, cam.center.y, 1);
	return T * S; // translate after scaling
}


void GLApp::drawTrans(Framework::Renderable& x, Framework::Animation& y,
	Framework::Transform& z, const glm::mat3& ZoomNDC, int drawOrder)
{
	float frameWidth = 1.0f / y.columns;
	float frameHeight = 1.0f / y.rows;
	float uOffsetX = (y.currentFrame % y.columns) * frameWidth;
	float vTop = 1.0f - ((y.currentFrame / y.columns + 1) * frameHeight);

	const float u0 = uOffsetX;
	const float v0 = vTop;
	const float u1 = uOffsetX + frameWidth;
	const float v1 = vTop + frameHeight;

	glm::mat3 M = glm::make_mat3(z.mdl_to_ndc_xform.Begin());
	glm::mat3 modelNDC = ZoomNDC * M;

	// NOTE: re.shd_ref / re.mdl_ref are floats in your component; cast to uint16
	gBatch.Push(static_cast<uint16_t>(x.shd_ref),
		static_cast<uint16_t>(x.mdl_ref),
		x.textureID,
		drawOrder,
		modelNDC, u0, v0, u1, v1);
}

void GLApp::draw(std::set<Entity> entities, LayerMask includeMask) {

	glm::mat3 PanZoom = BuildPanZoomNDC(gEditorCamera);

	// Build final camera matrix from the active camera
	//glm::mat3 PanZoom = BuildPanZoomNDC(activeCam);

	// ========================== REST OF YOUR DRAW ===========================
	auto* g_coordinator = Coordinator::GetInstance();

	static auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float deltaTime =
		std::chrono::duration<float>(currentTime - lastTime).count();
	lastTime = currentTime;
	if (!Framework::GameState::IsPlaying())
		deltaTime = 0.0f;
	gBatch.Begin();

	std::vector<Entity> drawList;
	drawList.reserve(entities.size());

	// Build draw list by layer
	for (auto it : entities) {
		// BEFORE (ORIGINAL CODE):
		//     auto& lt = g_coordinator->GetComponent<LayerTag>(it);
		//     if ((lt.mask & includeMask) == 0) continue;
		//

		// AFTER:
		// Check if entity has LayerTag component first
		if (!g_coordinator->HasComponent<LayerTag>(it)) {
			// No LayerTag - treat as LAYER_ALL (always visible)
			// Skip to normal rendering checks below
		}
		else {
			// Has LayerTag - check if it matches the filter
			auto& lt = g_coordinator->GetComponent<LayerTag>(it);
			if ((lt.mask & includeMask) == 0) continue;  // Layer filtered out
		}

		auto& tr = g_coordinator->GetComponent<Framework::Transform>(it);
		auto& re = g_coordinator->GetComponent<Framework::Renderable>(it);
		if (!tr.isVisible) continue;

		// ensure texture is ready BEFORE push
		if (re.textureID == 0 && !re.spriteName.empty()) {
			int w = 0, h = 0;
			re.textureID = mAssets.GetOrLoadTexture(re.spriteName);
			re.texW = w;
			re.texH = h;
			re.loadedSpriteName = re.spriteName;
		}

		if (re.textureID == 0) continue;
		drawList.push_back(it);
	}

	std::stable_sort(drawList.begin(), drawList.end(), [&](Entity a, Entity b) {
		// Defaults (if no LayerTag)
		int sortingLayerA = 1, sortingLayerB = 1;  // Default to World
		int orderA = 0, orderB = 0;

		if (g_coordinator->HasComponent<LayerTag>(a)) {
			auto& A = g_coordinator->GetComponent<LayerTag>(a);
			sortingLayerA = A.sortingLayer;
			orderA = A.orderInLayer;
		}

		if (g_coordinator->HasComponent<LayerTag>(b)) {
			auto& B = g_coordinator->GetComponent<LayerTag>(b);
			sortingLayerB = B.sortingLayer;
			orderB = B.orderInLayer;
		}

		// Sort by sorting layer first (Background=0 < World=1 < UI=2)
		if (sortingLayerA != sortingLayerB)
			return sortingLayerA < sortingLayerB;

		// Then by orderInLayer within same sorting layer
		return orderA < orderB;
	});

	// ═══════════════════════════════════════════════════════════════
	// DEBUG: Print draw order every 5 seconds
	// ═══════════════════════════════════════════════════════════════
	static int debugFrame = 0;
	if (debugFrame++ % 300 == 0 && drawList.size() > 0) {
		std::cout << "\n=== DRAW ORDER DEBUG ===\n";
		for (size_t i = 0; i < drawList.size(); ++i) {
			Entity e = drawList[i];
			std::string name = "Entity_" + std::to_string(e);
			int sl = 1, order = 0;

			if (g_coordinator->HasComponent<Name>(e)) {
				name = g_coordinator->GetComponent<Name>(e).name;
			}
			if (g_coordinator->HasComponent<LayerTag>(e)) {
				auto& lt = g_coordinator->GetComponent<LayerTag>(e);
				sl = lt.sortingLayer;
				order = lt.orderInLayer;
			}

			std::cout << i << ": " << name
				<< " | SortingLayer=" << sl
				<< " | OrderInLayer=" << order << "\n";
		}
		std::cout << "========================\n";
	}

	// Draw
	int drawOrder = 0;
	for (auto e : drawList) {
		auto& tr = g_coordinator->GetComponent<Framework::Transform>(e);
		auto& re = g_coordinator->GetComponent<Framework::Renderable>(e);
		auto& an = g_coordinator->GetComponent<Framework::Animation>(e);

		if (!re.spriteName.empty()) {
			if (re.textureID == 0 ||
				re.needsTextureReload ||
				re.spriteName != re.loadedSpriteName)
			{
				re.textureID = mAssets.GetOrLoadTexture(re.spriteName);
				re.loadedSpriteName = re.spriteName;
				re.needsTextureReload = false;
			}
		}

		if (tr.isVisible && re.textureID != 0)
		{
			const int rows = std::max(1, an.rows);
			const int columns = std::max(1, an.columns);
			const bool isSingle =
				(rows * columns <= 1) || (an.totalFrames <= 1);

			if (isSingle)
			{
				an.rows = 1;
				an.columns = 1;
				an.totalFrames = 1;
				an.currentFrame = 0;
				an.elapsedTime = 0.0f;
			}
			else
			{
				updateAnimation(an, deltaTime);
				an.rows = std::max(1, an.rows);
				an.columns = std::max(1, an.columns);
				an.totalFrames = std::max(1, an.totalFrames);
				if (an.currentFrame >= an.totalFrames)
					an.currentFrame %= an.totalFrames;
			}
		}

		glBindTexture(GL_TEXTURE_2D, re.textureID);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &re.texW);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &re.texH);

		drawTrans(re, an, tr, PanZoom, drawOrder);
		drawOrder++;
	}

	gBatch.Flush(shdrpgms);
	GLboolean depthWasOn = glIsEnabled(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);

	if (depthWasOn)
		glEnable(GL_DEPTH_TEST);

}

void GLApp::cleanup()
{
	GLApp::shdrpgms.clear();

	// // Delete translation buffer
	// if (tb.translationBuffer) {
	// 	glDeleteBuffers(1, &tb.translationBuffer);
	// 	tb.translationBuffer = 0;
	// }

	// Use GraphicsManager to clean up all textures
	// Framework::GraphicsManager::Clear();
	mAssets.getGraphics().Clear();

	// Delete any standalone textures not managed by TextureManager
	if (textureID)
	{
		glDeleteTextures(1, &textureID);
		textureID = 0;
	}

	GLApp::vps.clear();

	std::cout << "Graphics Resources successfully cleaned up." << std::endl;
}

// Shader Program Container
void GLApp::init_shdrpgms_cont(GLApp::VPSS const& vpss)
{
	shdrpgms.clear();
	shdrpgms.reserve(vpss.size());
	int i = 0;
	for (auto const& x : vpss)
	{
		if (x.first.empty())
		{
			std::cerr << "[Shader] Empty VERT source for tag\n";
			std::exit(EXIT_FAILURE);
		}
		if (x.second.empty())
		{
			std::cerr << "[Shader] Empty FRAG source for tag\n";
			std::exit(EXIT_FAILURE);
		}
		GLSLShader shdr_pgm;
		if (!shdr_pgm.CompileShaderFromString(GL_VERTEX_SHADER, x.first)) {
			std::cout << "Vertex shader failed to compile: ";
			std::cout << shdr_pgm.GetLog() << std::endl;
			std::exit(EXIT_FAILURE);
		}
		if (!shdr_pgm.CompileShaderFromString(GL_FRAGMENT_SHADER, x.second))
		{
			std::cout << "Fragment shader failed to compile: ";
			std::cout << shdr_pgm.GetLog() << std::endl;
			std::exit(EXIT_FAILURE);
		}
		if (!shdr_pgm.Link())
		{
			std::cout << "Shader program failed to link!" << shdr_pgm.GetLog() << std::endl;
			std::exit(EXIT_FAILURE);
		}
		if (!shdr_pgm.Validate())
		{
			std::cout << "Shader program failed to validate!" << shdr_pgm.GetLog() << std::endl;
			std::exit(EXIT_FAILURE);
		}
		// insert shader program into container
		GLApp::shdrpgms.emplace_back(std::move(shdr_pgm));
		++i;
	}
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

	GLHelper::GameOffsetX = GLApp::GameOffsetX;
	GLHelper::GameOffsetY = GLApp::GameOffsetY;
	GLHelper::GameWidth = GLApp::GameWidth;
	GLHelper::GameHeight = GLApp::GameHeight;

	glViewport(GLApp::GameOffsetX, GLApp::GameOffsetY, GLApp::GameWidth, GLApp::GameHeight);
}

void GLApp::OnResize(int width, int height)
{
	ComputeLetterbox(width, height);
}

// Convert vec2 in model space to NDC using your existing 2D pipeline
static inline glm::vec2 ToNDC(const glm::mat3 & M, const glm::vec2 & v)
{
	glm::vec3 h = M * glm::vec3(v, 1.0f);
	return { h.x, h.y }; // already NDC
}

// Draw an AABB as 4 lines in NDC
void GLApp::DrawAABB_NDC(const glm::mat3 & modelToNDC, const AABB & box, const glm::vec3 & color)
{
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