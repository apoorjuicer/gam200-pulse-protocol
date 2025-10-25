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
#include "CoreEngine/Factory/Factory.h"


//Starting scene will be Main Menu
SceneManager::SceneManager(): CurrentScene(Scene::MainMenu), PreviousScene(Scene::MainMenu)
{
	
}

void SceneManager::LoadScene(Scene scene)//,const std::string& path)
{
#ifdef _DEBUG
	std::cout << "\n================================================" << std::endl;
	std::cout << "    [SceneManager] Successfully initialized." << std::endl;
	std::cout << "================================================" << std::endl;
#endif
	// unload previous scene first
	UnloadCurrentScene();

	// check if there was a saved scene that current scene is not the same as previous scene

	//std::optional<Scene> SavedScene = GrabSavedScene();
	//if(SavedScene.has_value() && GrabCurrentScene() != GrabPreviousScene())
	//{
	//	std::cout << "Found saved scene. Loading saved scene instead!" << std::endl;
	//	LoadSavedScene(SavedScene.value());
	//}

	//std::ifstream ifs(path);
	//if (!ifs.is_open()) 
	//{
	//	std::cerr << "[LoadScene] Error: Cannot open file: " << path << "\n";
	//	return;
	//}



	CurrentScene = scene;
	switch (scene)
	{
	case Scene::MainMenu:
			LoadMainMenu();
			break;

	case Scene::Pause:
		LoadPause();
		break;

	default:
		assert("Input was not a scene!");
		break;
	}

	PreviousScene = CurrentScene; 
#ifdef _DEBUG
	std::cout << "\n Scene was loaded successfully" << std::endl;
#endif
}

void SceneManager::LoadSavedScene([[maybe_unused]] Scene scene)
{

}

void SceneManager::SaveCurrentScene()
{
	// go thru all of the entity and save them to json file
	for ([[maybe_unused]] auto& x: EntityMember)
	{
		//these are causing the warnings
		//if ("type" == "Hero")
		//{
		//	//call serialize 
		//}
		//if ("type" == "Monster")
		//{
		//}
	}

	////auto component = ;

	//std::string saved_path = "../../MyEngine/JSON/SavedScene.json";
	//if (!Framework::SaveTransform(component, saved_path)) 
	//{
	//	std::cerr << "Failed to save CurrentScene!\n";
	//}
	//
	//else 
	//{
	//	std::cout << "Saved to: " << std::filesystem::absolute(saved_path) << "\n";
	//}
}

void SceneManager::UnloadCurrentScene()
{
#ifdef _DEBUG
	std::cout << "\n================================================" << std::endl;
	std::cout << "    [SceneManager] Unloading in progress!" << std::endl;
	std::cout << "================================================" << std::endl;
#endif
	auto* g_coordinator = Coordinator::GetInstance();
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

	auto* g_coordinator = Coordinator::GetInstance();
	Entity PauseButton = g_coordinator->CreateEntity();
	Framework::Transform pause_button;

	//std::string path = "../../MyEngine/JSON/save_PauseButton.json";

	//if (!Framework::LoadTransform(pause_button, path)) {
	//	std::cerr << "Failed to load new_feedbackTransform!\n";
	//}
	//else {
	//	std::cout << "Loaded new_feedbackTransform From: " << std::filesystem::absolute(path) << "\n";
	//}

	// idk what to add here
	 pause_button.Pos = { 0,0};
	 pause_button.Scale = {50,50};

	g_coordinator->AddComponent(PauseButton, pause_button);
	SceneEntityList.push_back(PauseButton);
	
	std::cout << pause_button.Pos;
	std::cout << pause_button.Scale;


	std::string saved_path = "../../MyEngine/JSON/save_PauseButton.json";
	if (!Framework::SaveTransform(pause_button, saved_path)) 
	{
		std::cerr << "Failed to save save_PauseButton!\n";
	}
	
	else 
	{
		std::cout << "Saved to: " << std::filesystem::absolute(saved_path) << "\n";
	}
}





