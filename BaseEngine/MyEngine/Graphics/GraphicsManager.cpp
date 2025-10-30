/******************************************************************************/
/**
 * @file        GraphicsManager.cpp
 * @project     Pulse Protocol
 * @author		Goh Pin Kai (primary) - 70%
 * @author		Leu Jun Yong (secondary) - 30% each
 * @brief       Implements the GraphicsManager, handling OpenGL graphics setup,
 *              text rendering via FreeType, message subscriptions, per-frame
 *              updates, and cleanup of rendering resources.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "GraphicsManager.h"

float editorHeight;
float editorWidth;
Framework::Transform trans;

std::map<char, Character> Characters;
std::map<char, Character> Characters2;

GraphicsManager::GraphicsManager() : ft(nullptr), face(nullptr), face2(nullptr), VAO(0), VBO(0) {
	glapp = new GLApp;
}

void GraphicsManager::Initialize()
{
	GLHelper::print_specs();
	glapp->init(/*editorWidth, editorHeight*/);

	// Subscribe to combo messages - DONT REMOVE, TEST FOR MESSAGE SYSTEM
	auto* msgMgr = MessageManager::GetInstance();
	msgMgr->Subscribe(Message::Type::INPUT_COMBO, "GraphicsManager",
		[this](const Message& msg) {
			const InputComboMessage& comboMsg = static_cast<const InputComboMessage&>(msg);
			std::cout << "[GRAPHICS] Received combo: " << comboMsg.comboName << std::endl;
		});

	std::cout << "[GraphicsManager] Successfully initialized for graphics." << std::endl;

	// Initialize FreeType
	if (FT_Init_FreeType(&ft)) {
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return;
	}

	// Blending for transparent text
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	// Ortho projection, top-left origin
	int width, height;
	glfwGetFramebufferSize(Engine->GetWindow(), &width, &height);
	projection = glm::ortho(0.0f, static_cast<float>(GLApp::VIRTUAL_W), static_cast<float>(GLApp::VIRTUAL_H), 0.0f);

	// Load fonts
	if (!LoadFont("../../MyEngine/Assets/Font/PatrickHand-Regular.ttf", face, Characters)) {
		std::cout << "ERROR::FREETYPE: Failed to load first font" << std::endl;
		return;
	}
	if (!LoadFont("../../MyEngine/Assets/Font/liberation-mono.ttf", face2, Characters2)) {
		std::cout << "ERROR::FREETYPE: Failed to load second font" << std::endl;
		return;
	}

	// VAO/VBO setup: pos.xy + uv.xy
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

	// position -> location 0
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	// uv -> location 1
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	unsigned char whitePixel[4] = { 255, 255, 255, 255 };
	glGenTextures(1, &whiteTexture);
	glBindTexture(GL_TEXTURE_2D, whiteTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	auto& textShader = GLApp::shdrpgms[3]; // this must be your text shader
	textShader.Use();
	GLint loc = glGetUniformLocation(textShader.GetHandle(), "text");

	if (loc != -1) glUniform1i(loc, 0); // bind 'text' sampler to GL_TEXTURE0


}

GraphicsManager::~GraphicsManager()
{

}

void GraphicsManager::Update()
{
	int width, height;
	glfwGetFramebufferSize(Engine->GetWindow(), &width, &height);

	glfwPollEvents();
	GLHelper::update_time(1.0);
	glapp->update(Engine->GetWindow(), EntityMember);
	glapp->draw(EntityMember);                     // 1) draw your scene first

	RenderGUIButtons();

	// 2) then draw text on top
	RenderText(GLApp::shdrpgms[3], "Team BLOOP", 600.0f, 850.0f, 1.0f, glm::vec3(1, 1, 1), Characters);

	// Render button labels
	auto* coordinator = Coordinator::GetInstance();
	auto inputMgr = coordinator->GetSystem<InputManager>();

	// TEMPORARY: Apply zoom to projection for visual test
	if (inputMgr) {
		float zoom = inputMgr->getZoomLevel();
		float viewWidth = GLApp::VIRTUAL_W / zoom;
		float viewHeight = GLApp::VIRTUAL_H / zoom;

		projection = glm::ortho(0.0f, viewWidth, viewHeight, 0.0f);
	}

	if (inputMgr) {
		float zoom = inputMgr->getZoomLevel();
		float viewWidth = GLApp::VIRTUAL_W / zoom;
		float viewHeight = GLApp::VIRTUAL_H / zoom;

		projection = glm::ortho(0.0f, viewWidth, viewHeight, 0.0f);

		for (auto entity : inputMgr->GUIButtonEntities) {
			if (!coordinator->HasComponent<GUIButton>(entity)) continue;
			if (!coordinator->HasComponent<Framework::Transform>(entity)) continue;

			auto& button = coordinator->GetComponent<GUIButton>(entity);
			auto& transform = coordinator->GetComponent<Framework::Transform>(entity);

			// Use SAME coordinate conversion as RenderGUIButtons
			// World: (-800..800, -450..450) -> Virtual: (0..1600, 0..900)
			float virtualX = transform.Pos.x + 800.0f;
			float virtualY = 450.0f - transform.Pos.y;  // Flip Y

			// Render button label (centered on button)
			RenderText(GLApp::shdrpgms[3], button.label,
				virtualX + 10.0f,   // Offset from left edge of button
				virtualY + 30.0f,   // Offset from top edge of button  
				0.4f,               // Text scale
				glm::vec3(1, 1, 1), Characters2);
		}
	}

	// Build FPS string and heaviest system string and render it
	if (!MyEditor::IsEnabled()) {
		std::ostringstream fpsText;
		fpsText << "FPS: " << std::fixed << std::setprecision(1) << GLHelper::fps;
		RenderText(GLApp::shdrpgms[3], fpsText.str(), 100.0f, 850.0f, 1.0f, glm::vec3(0.5f, 0, 0), Characters2);

		// Add top system below FPS
		auto* profiler = ProfilingManager::GetInstance();
		const auto& profiles = profiler->GetDisplayProfiles();

		std::string topSystem = "None";
		double topPercentage = 0.0;
		for (const auto& [systemName, profile] : profiles) {
			if (profile.percentage > topPercentage) {
				topPercentage = profile.percentage;
				topSystem = systemName;
			}
		}

		std::ostringstream topText;
		topText << topSystem << ": " << std::fixed << std::setprecision(2) << topPercentage << "%";
		RenderText(GLApp::shdrpgms[3], topText.str(), 100.0f, 880.0f, 0.8f, glm::vec3(0.5f, 0.5f, 0), Characters2);
	}

	// Controls display - right side or top corner
	float startX = width - 350.0f;  // 350 pixels from right edge
	float startY = height - 200.0f; // Start 250 pixels from bottom
	float lineHeight = 25.0f;

	RenderText(GLApp::shdrpgms[3], "Controls:", startX, startY, 0.5f, glm::vec3(1, 1, 0), Characters2);
	RenderText(GLApp::shdrpgms[3], "M - Start monsters", startX, startY + lineHeight * 1, 0.4f, glm::vec3(1, 1, 1), Characters2);
	RenderText(GLApp::shdrpgms[3], "W/A/S/D - Input", startX, startY + lineHeight * 2, 0.4f, glm::vec3(1, 1, 1), Characters2);
	RenderText(GLApp::shdrpgms[3], "K/L - Scale hero", startX, startY + lineHeight * 3, 0.4f, glm::vec3(1, 1, 1), Characters2);
	RenderText(GLApp::shdrpgms[3], "H/J - Rotate monsters", startX, startY + lineHeight * 4, 0.4f, glm::vec3(1, 1, 1), Characters2);
	RenderText(GLApp::shdrpgms[3], "O - Toggle editor", startX, startY + lineHeight * 5, 0.4f, glm::vec3(1, 1, 1), Characters2);
	RenderText(GLApp::shdrpgms[3], "F1-F6 - Debug draw", startX, startY + lineHeight * 6, 0.4f, glm::vec3(1, 1, 1), Characters2);
}

void GraphicsManager::Shutdown()
{
	// Unsubscribe from messages DONT REMOVE
	auto* msgMgr = MessageManager::GetInstance();
	msgMgr->Unsubscribe(Message::Type::INPUT_COMBO, "GraphicsManager");

	glapp->cleanup();
	delete glapp;

	if (face)  FT_Done_Face(face);
	if (face2) FT_Done_Face(face2);
	if (ft)    FT_Done_FreeType(ft);
	for (auto& [key, ch] : Characters) glDeleteTextures(1, &ch.TextureID);
	for (auto& [key, ch] : Characters2) glDeleteTextures(1, &ch.TextureID);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	std::cout << "[GraphicsManager] Shutdown completed" << std::endl;
}

void GraphicsManager::RenderGUIButtons()
{
	auto* coordinator = Coordinator::GetInstance();
	auto inputMgr = coordinator->GetSystem<InputManager>();

	if (!inputMgr) return;

	auto& shader = GLApp::shdrpgms[2]; // Adjust if needed
	shader.Use();

	for (auto entity : inputMgr->GUIButtonEntities) {
		if (!coordinator->HasComponent<GUIButton>(entity)) continue;
		if (!coordinator->HasComponent<Framework::Transform>(entity)) continue;

		auto& button = coordinator->GetComponent<GUIButton>(entity);
		auto& transform = coordinator->GetComponent<Framework::Transform>(entity);

		// Choose color based on button state
		glm::vec4 color = (button.currentState == ButtonState::Hover)
			? button.hoverColor
			: button.normalColor;

		// Convert world coords to screen coords
		// World: (-800..800, -450..450), Screen: (0..1600, 0..900)
		float screenX = transform.Pos.x + 800.0f;
		float screenY = 450.0f - transform.Pos.y; // Flip Y

		// Draw colored rectangle
		RenderColoredQuad(screenX, screenY, transform.Scale.x, transform.Scale.y, color);
	}
}

void GraphicsManager::RenderColoredQuad(float x, float y, float width, float height, glm::vec4 color)
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	auto& shader = GLApp::shdrpgms[3];
	shader.Use();

	glUniformMatrix4fv(glGetUniformLocation(shader.GetHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform3f(glGetUniformLocation(shader.GetHandle(), "textColor"), color.r, color.g, color.b);

	// Bind white texture instead of unbinding
	glBindTexture(GL_TEXTURE_2D, whiteTexture);
	glBindVertexArray(VAO);

	float vertices[6][4] = {
		{ x,         y,          0.0f, 0.0f },
		{ x,         y + height, 0.0f, 1.0f },
		{ x + width, y + height, 1.0f, 1.0f },
		{ x,         y,          0.0f, 0.0f },
		{ x + width, y + height, 1.0f, 1.0f },
		{ x + width, y,          1.0f, 0.0f }
	};

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool GraphicsManager::LoadFont(const std::string& fontPath,
	FT_Face& fontFace,
	std::map<char, Character>& characterMap)
{
	if (FT_New_Face(this->ft, fontPath.c_str(), 0, &fontFace)) return false;

	FT_Set_Pixel_Sizes(fontFace, 0, 48);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (unsigned char c = 0; c < 128; ++c) {
		if (FT_Load_Char(fontFace, c, FT_LOAD_RENDER)) continue;
		//if (FT_Render_Glyph(fontFace->glyph, FT_RENDER_MODE_NORMAL)) continue;

		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// upload glyph bitmap as single-channel
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8,
			fontFace->glyph->bitmap.width,
			fontFace->glyph->bitmap.rows,
			0, GL_RED, GL_UNSIGNED_BYTE,
			fontFace->glyph->bitmap.buffer);

		// filtering & wrap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		Character ch{
			texture,
			{ (int)fontFace->glyph->bitmap.width,  (int)fontFace->glyph->bitmap.rows },
			{ (int)fontFace->glyph->bitmap_left,   (int)fontFace->glyph->bitmap_top  },
			(unsigned int)fontFace->glyph->advance.x
		};
		characterMap.emplace((char)c, ch);
	}
	return true;
}

void GraphicsManager::RenderText(GLSLShader& shader,
	std::string text,
	float x, float y,
	float scale,
	glm::vec3 color,
	std::map<char, Character>& fontCharacters)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shader.Use();
	glUniformMatrix4fv(glGetUniformLocation(shader.GetHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform3f(glGetUniformLocation(shader.GetHandle(), "textColor"), color.x, color.y, color.z);
	glUniform1i(glGetUniformLocation(shader.GetHandle(), "text"), 0);

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	for (unsigned char c : text) {
		auto it = fontCharacters.find(c);
		if (it == fontCharacters.end()) continue;
		const Character& ch = it->second;

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - ch.Bearing.y * scale + ch.Size.y * scale;
		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		// vertices: pos.xy, uv.xy
		float vertices[6][4] = {
			{ xpos,     ypos,     0.0f, 1.0f },
			{ xpos,     ypos - h, 0.0f, 0.0f },
			{ xpos + w, ypos - h, 1.0f, 0.0f },

			{ xpos,     ypos,     0.0f, 1.0f },
			{ xpos + w, ypos - h, 1.0f, 0.0f },
			{ xpos + w, ypos,     1.0f, 1.0f }
		};

		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (ch.Advance >> 6) * scale;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GraphicsManager::OnResize(int width, int height)
{
	// Tells GLApp to update viewport and projection
	GLApp::OnResize(width, height);
}
