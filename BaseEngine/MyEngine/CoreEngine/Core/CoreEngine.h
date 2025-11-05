/******************************************************************************/
/**
 * @file        CoreEngine.h
 * @project     Pulse Protocol
 * @author		Chia Wei Xuan Rachael
 * @brief		Declares the core engine class that manages subsystem
				initialization, the main game loop, and development utilities.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#pragma once
#include "pch/pch.h"
//#include "../Components/AABB.h"
#include "../Components/Health.h"
#include "../Components/EntityType.h"
#include "../MyEngine/Serialization/Serialization.h"

class CoreEngine
{
public:
	CoreEngine();
	~CoreEngine();

	// Engine 3 states
	void Init();
	void Setup();
	void Update(float dt);
	void GameLoop();
	void Exit();

	//void SetPaused(bool p);
	//void TogglePaused();
	//bool IsPaused() const { return m_paused; }

	// One of the ways to glue systems tgt. 
	//void BroadcastMessage(Message* message);


	// for controlling main game loop
	bool IsGameRunning() const { return GameRunning; }

	// for controlling scripts
	bool IsPlaying() const { return m_isPlaying; }

	GLFWwindow* GetWindow() const { return window; }


	void PlayGame();	// play button functionality
	void StopGame();	// stop button functionality

	void RestartRuntimeForPlay();

	
private:
	bool GameRunning;
	bool DebugMode = false;
	unsigned LastTime; //The last time the game was updated
	bool m_paused = true;
	bool m_isPlaying = false;

	void RenderDebugDraw();

	GLFWwindow* window;
};

//A global pointer to the instance of the core
extern CoreEngine* Engine;
extern Config config;
extern int WindowWidth;
extern int WindowHeight;
