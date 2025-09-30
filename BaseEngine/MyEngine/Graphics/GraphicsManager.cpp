/*********************************************************************************************
// GraphicsManager.cpp  
\par		Bloop
\author    Goh pin Kai(pinkai.g@digipen.edu)


\copyright All content   2025 DigiPen Institute of Technology Singapore.All rights reserved.
* *********************************************************************************************/

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

	Framework::AssetsManager::Instance().LoadAllImage();

	std::cout << "[GraphicsManager] Successfully initialized for graphics." << std::endl;


}
// dtor
GraphicsManager::~GraphicsManager()
{

}



void GraphicsManager::Update()
{

	//std::cout << "[GraphicsManager] UPDATED." << std::endl;
	glfwPollEvents();
	GLHelper::update_time(1.0);
	glapp->update(Engine->GetWindow());
	glapp->draw(EntityMember);

}


void GraphicsManager::Shutdown()
{	
	Framework::AssetsManager::Instance().UnloadAllImage();
	transObj.clear();
	glapp->cleanup();
	delete glapp;
	std::cout << "[GraphicsManager] Shutdown completed" << std::endl;

}