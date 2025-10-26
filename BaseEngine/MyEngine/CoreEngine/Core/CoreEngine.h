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
#include "../MyEngine/Serialization/Serialization.h"
#include "../MyEngine/Resources/AssetRegistry.h"

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


	// One of the ways to glue systems tgt. 
	//void BroadcastMessage(Message* message);

	bool IsGameRunning() const { return GameRunning; }

	GLFWwindow* GetWindow() const { return window; }
	
private:
	bool GameRunning;
	bool DebugMode = false;
	unsigned LastTime; //The last time the game was updated

	void RenderDebugDraw();

	GLFWwindow* window;
};

//A global pointer to the instance of the core
extern CoreEngine* Engine;
extern Config config; //global scope
extern AssetRegistry ar; //global scope
extern int WindowWidth;
extern int WindowHeight;
//constexpr int WindowWidth = 1600;
//constexpr int WindowHeight = 900;

