/******************************************************************************/
/**
 * @file        glapp.h
 * @project     Pulse Protocol
 * @author      Goh Pin Kai
 * @brief		Declares OpenGL application utilities for initializing
				shaders, models, viewports, and rendering textured entities.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#pragma once
#ifndef GLAPP_H
#define GLAPP_H
#include "glhelper.h"
#include "pch/pch.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Transform.h"
#include "Renderable.h"
#include "Animation.h"
#include "CoreEngine/Assets.h"
#include "Physics/collision.h"          
// External transforms list (used elsewhere)   
namespace Framework { class Transform; class Animation; class Renderable;}

class GLApp {
public:
	static inline int VIRTUAL_W = 1920;
	static inline int VIRTUAL_H = 1080;
	void init(/*float inputEditorWidth, float inputEditorHeight*/);
	//void update(GLFWwindow* window);
	void update(GLFWwindow* window, std::set<Entity>);
	void draw(std::set<Entity>);
	void cleanup();
	void init_models_cont();

	GLuint vaoid; // handle to VAO
	GLuint idx_elem_cnt;
	GLenum primitive_type;
	void drawTrans(Framework::Renderable& x, Framework::Animation& y, Framework::Transform& z, const glm::mat3& VP);
	void updateAnimation(Framework::Animation& x, float deltaTime);

	static inline int GameWidth = VIRTUAL_W;  // actual GL viewport width after letterbox
	static inline int GameHeight = VIRTUAL_H;  // actual GL viewport height after letterbox
	static inline int GameOffsetX = 0;         // viewport X in framebuffer
	static inline int GameOffsetY = 0;         // viewport Y in framebuffer
	static inline float PixelScale = 1.0f;     // uniform scale = min(fbw/VW, fbh/VH)
	static void ComputeLetterbox(int fbw, int fbh);
	static void OnResize(int width, int height);
	void DrawAABB_NDC(const glm::mat3& modelToNDC, const AABB& box, const glm::vec3& color);
	struct Camera2D {
		glm::vec2 center{ 0.0f, 0.0f };
		float zoom = 1.0f; // 1.0f = normal, >1 = zoom in, <1 = zoom out
	};
	static Camera2D gCamera;
	// encapsulates state required to render a geometrical model
	struct GLModel {
		GLenum primitive_type; // which OpenGL primitive to be rendered?
		GLSLShader shdr_pgm; // which shader program?
		GLuint vaoid; // handle to VAO

		GLuint idx_elem_cnt; // how many elements of primitive of type
		GLuint primitive_cnt; // added for Tutorial 2
		GLuint draw_cnt; // added for Tutorial 2

		// Constructor to initialize member variables
		GLModel(GLenum prim_type = GL_TRIANGLES, GLuint elements = 0, GLuint p_cnt = 0, GLuint d_cnt = 0)
			: primitive_type(prim_type), // Default OpenGL primitive type (GL_TRIANGLES)
			vaoid(0),                    // Default to 0 for VAO
			idx_elem_cnt(elements),      // Can be set by parameter
			primitive_cnt(p_cnt),        // Can be set by parameter
			draw_cnt(d_cnt)              // Can be set by parameter
		{
		}
	};

	// Data member to represent geometric model to be rendered
	// C++ requires this object to have a definition in glapp.cpp!!!
	static GLModel mdl;

	static std::vector<GLApp::GLModel> models; // added for Tutorial 2

	GLApp::GLModel box_model();
	//GLApp::GLModel mystery_model();
	GLApp::GLModel bg_model();

	static std::vector<GLSLShader> shdrpgms; // singleton in tutorial 3

	using VPSS = std::vector<std::pair<std::string, std::string>>;
	static void init_shdrpgms_cont(GLApp::VPSS const&);

	struct TranslationBuf {
		constexpr static int instance_count = 100000;
		constexpr static float quad_scale = 0.01f;
		GLsizeiptr translationBufferSize = instance_count * sizeof(glm::vec4);
		GLuint translationBuffer = 0;
		std::vector<glm::vec4> position_scale;
		std::vector<glm::vec2> velocity;
		float boundaryRange = 0.95f; // -1.0 to 1.0 in x and y
	};
	static TranslationBuf tb;

	struct GLViewport {
		GLint x, y;
		GLsizei width, height;
	};
	static std::vector<GLViewport> vps; // container for viewports
};

#endif /* GLAPP_H */
