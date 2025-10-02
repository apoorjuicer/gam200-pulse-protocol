/*********************************************************************************************
 \file      glapp.h
 \par		Bloop
 \author    Goh pin Kai (pinkai.g@digipen.edu)


\copyright All content   2025 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/
#pragma once
#ifndef GLAPP_H
#define GLAPP_H
#include "glhelper.h"
#include "pch/pch.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Transform.h"
// External transforms list (used elsewhere)   
namespace Framework { class Transform; }

class GLApp {
public:
	void init(float inputEditorWidth, float inputEditorHeight);
	//void update(GLFWwindow* window);
	void update(GLFWwindow* window, std::set<Entity>);
	void draw(std::set<Entity>);
	void cleanup();
	void init_models_cont();

	GLuint vaoid; // handle to VAO
	GLuint idx_elem_cnt;
	GLenum primitive_type;
	void drawTrans(Framework::Transform& x);
	void updateAnimation(Framework::Transform& x, float deltaTime);

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

// Simple orthographic MVP helper for DebugDraw usage
inline glm::mat4 MakeOrthoMvp(float left, float right, float bottom, float top) {
	// Z range -1..1
	glm::mat4 proj = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
	return proj; // No view/model yet
}

#endif /* GLAPP_H */
