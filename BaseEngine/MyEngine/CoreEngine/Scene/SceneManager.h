/******************************************************************************/
/**
 * @file       SceneManager.h
 * @project    Pulse Protocol
 * @author     Chia Wei Xuan Rachael
 * @brief      Scene management system class that inherits from Systems base class.
 *             Defines Scene enum for game states and provides interface for
 *             scene loading, unloading, and transitioning. Maintains current
 *             scene state and tracks scene-specific entities for cleanup
 * @copyright  Copyright (C) 2025 DigiPen Institute of Technology.
 *             Reproduction or disclosure of this file or its contents without the
 *             prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#pragma once
#include "pch/pch.h"
#include "CoreEngine/ECS/Singleton.h"

enum struct Scene { MainMenu, Pause, Level1, Dead };

class SceneManager: public Systems
{
public:
	SceneManager();

	//void Init() override;              
	//void Update(float dt) override;    
	//void Exit() override;

	void LoadScene(Scene scene);//, const std::string& path);
	void LoadSavedScene(Scene scene);
	void SaveCurrentScene();
	void UnloadCurrentScene();

	Scene GrabCurrentScene() const { return CurrentScene; };
	Scene GrabPreviousScene() const { return PreviousScene; };
	std::optional<Scene> GrabSavedScene() const { return SavedScene; };

private:
	Scene CurrentScene;
	Scene PreviousScene;
	std::optional<Scene> SavedScene;

	// Hard coding pathway to file that will always be available to save scene and load prefab.
	std::string SaveEntityPathway = "../../MyEngine/JSON/SavedScene.json";// where to save data

	void LoadMainMenu();
	void LoadPause();
	std::vector<Entity> SceneEntityList;
};

