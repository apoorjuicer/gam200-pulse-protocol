// ==================================================================================
// GraphicsManager.cpp  
// Author: Goh Pin Kai
// Brief: 
// ==================================================================================

#include "GraphicsManager.h"

float editorHeight;
float editorWidth;
Framework::Transform trans;
GraphicsManager::GraphicsManager() {
	glapp = new GLApp;
}
void GraphicsManager::Initialize()
{
	GLHelper::print_specs();
	glapp->init(editorWidth, editorHeight);
	std::cout << "[GraphicsManager] Successfully initialized for graphics." << std::endl;
}
// dtor
GraphicsManager::~GraphicsManager()
{
	transObj.clear();
	glapp->cleanup();
	delete glapp;
}



void GraphicsManager::Update()
{

	//std::cout << "[GraphicsManager] UPDATED." << std::endl;
	glfwPollEvents();
	GLHelper::update_time(1.0);
	glapp->update();
	glapp->draw();

}


void GraphicsManager::Shutdown()
{
		std::cout << "[GraphicsManager] Shutdown completed" << std::endl;

}