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
	~SceneManager();

	void LoadScene(Scene scene);

	Scene GrabCurrentScene() const { return CurrentScene; };

private:
	Scene CurrentScene;
	std::vector<Entity> SceneEntityList;

	void UnloadCurrentScene();
	void LoadMainMenu();
	void LoadPause();
	void LoadLevel1();
	void LoadDead();
};

