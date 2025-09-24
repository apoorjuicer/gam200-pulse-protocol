#pragma once
#include "pch/pch.h"


class CoreEngine
{
public:
	CoreEngine();
	~CoreEngine();

	// Engine 3 states
	void Init();
	void Setup();
	void Update(float dt);
	void Render();
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

	GLFWwindow* window;
	/*static const char WindowTitle[];
	static const int WindowHeight;
	static const int WindowWidth;*/
};

//A global pointer to the instance of the core
extern CoreEngine* Core;
