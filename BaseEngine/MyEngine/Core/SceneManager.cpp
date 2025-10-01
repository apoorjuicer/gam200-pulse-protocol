#pragma once
#include "pch/pch.h"

auto* g_coordinator = Coordinator::GetInstance();

//Starting scene will be Main Menu
SceneManager::SceneManager(): CurrentScene(Scene::MainMenu)
{
	
}

SceneManager::~SceneManager()
{
	UnloadCurrentScene();
}

void SceneManager::LoadScene(Scene scene)
{
#ifdef _DEBUG
	std::cout << "\n================================================" << std::endl;
	std::cout << "    [SceneManager] Successfully initialized." << std::endl;
	std::cout << "================================================" << std::endl;
#endif

	UnloadCurrentScene();

	CurrentScene = scene;

	switch (scene)
	{
	case Scene::MainMenu:
			LoadMainMenu();
			break;

	case Scene::Pause:
		LoadPause();
		break;

	case Scene::Level1:
		LoadLevel1();
		break;

	case Scene::Dead:
		LoadDead();
		break;

	default:
		assert("Input was not a scene!");
		break;
	}

#ifdef _DEBUG
	std::cout << "\n Scene was loaded successfully" << std::endl;
#endif
}


void SceneManager::UnloadCurrentScene()
{
#ifdef _DEBUG
	std::cout << "\n================================================" << std::endl;
	std::cout << "    [SceneManager] Unloading in progress!" << std::endl;
	std::cout << "================================================" << std::endl;
#endif

	for(auto & entity: SceneEntityList)
	{
		g_coordinator->DestroyEntity(entity);
		#ifdef _DEBUG
		std::cout << "Entity: " << entity << "was killed off." << std::endl;
		#endif
	}

	//This class own entity container is cleared
	SceneEntityList.clear();
	#ifdef _DEBUG
		std::cout << "Scene Unloaded" << std::endl;
	#endif
}


void SceneManager::LoadMainMenu()
{
#ifdef _DEBUG
	std::cout << "\n================================================" << std::endl;
	std::cout << "           Main menu loaded!" << std::endl;
	std::cout << "================================================" << std::endl;
#endif

	Entity background = g_coordinator->CreateEntity();
	AABB BackgroundComponent;

	// idk what to add here
	BackgroundComponent.min.x = { 5 };
	BackgroundComponent.max.y = { 5 };
	
	//Framework::Transform BackgroundComponent;
	// idk what to add here
	//BackgroundComponent.Pos = {};
	//BackgroundComponent.Scale = {};

	g_coordinator->AddComponent(background, BackgroundComponent);
	SceneEntityList.push_back(background);
	
	

}


void SceneManager::LoadPause()
{

	Entity PauseButton = g_coordinator->CreateEntity();
	Framework::Transform pause_button;

	// idk what to add here
	pause_button.Pos = {};
	pause_button.Scale = {};

	g_coordinator->AddComponent(PauseButton, pause_button);
	SceneEntityList.push_back(PauseButton);

}

void SceneManager::LoadLevel1()
{
	/*for (size_t i = 0; i <= SceneEntityList.size(); ++i)
	{

	}*/
}

void SceneManager::LoadDead()
{

}



