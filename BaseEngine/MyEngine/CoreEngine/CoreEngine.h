#pragma once
#include "pch/pch.h"
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


	// One of the ways to glue systems tgt. 
	//void BroadcastMessage(Message* message);

	bool IsGameRunning() const { return GameRunning; }

	GLFWwindow* GetWindow() const { return window; }
	
private:
	bool GameRunning;
	bool DebugMode;
	unsigned LastTime; //The last time the game was updated

	void RenderDebugDraw();

	GLFWwindow* window;
};

//A global pointer to the instance of the core
extern CoreEngine* Engine;
extern Config config;
extern int WindowWidth;
extern int WindowHeight;
//constexpr int WindowWidth = 1600;
//constexpr int WindowHeight = 900;

