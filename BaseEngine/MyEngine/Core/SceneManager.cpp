/******************************************************************************/
/**
 * @file       SceneManager.cpp
 * @project    Pulse Protocol
 * @author     Chia Wei Xuan Rachael
 * @brief      Implementation of scene management system that handles loading,
 *             unloading, and transitioning between game scenes (MainMenu, Pause,
 *             Level1, Dead). Manages scene-specific entity creation/destruction
 *             and maintains a list of entities belonging to the current scene
 * @copyright  Copyright (C) 2025 DigiPen Institute of Technology.
 *             Reproduction or disclosure of this file or its contents without the
 *             prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#pragma once
#include "pch/pch.h"

auto* g_coordinator = Coordinator::GetInstance();

//Starting scene will be Main Menu
SceneManager::SceneManager(): CurrentScene(Scene::Pause)
{
	
}

SceneManager::~SceneManager()
{

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

	//Entity background = g_coordinator->CreateEntity();
	//Framework::Transform BackgroundComponent;

	//// idk what to add here
	//BackgroundComponent.Scale = { 5 , 10 };
	//BackgroundComponent.Rotation =  { 5, 10} ;
	//BackgroundComponent.Pos= { 5,10 };
	//
	//g_coordinator->AddComponent(background, BackgroundComponent);
	//SceneEntityList.push_back(background);
	

	

}


void SceneManager::LoadPause()
{
#ifdef _DEBUG
	std::cout << "\n================================================" << std::endl;
	std::cout << "           Paused menu loaded!" << std::endl;
	std::cout << "================================================" << std::endl;
#endif

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
	
}

void SceneManager::LoadDead()
{

}



