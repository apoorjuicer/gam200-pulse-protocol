#pragma once
#include "pch/pch.h"


class CoreEngine
{
public:
	CoreEngine();
	~CoreEngine();

	// Engine 3 states
	void Init();
	void Update();
	void Exit();

	// where application loop is. Just gonna name is game loop since its easier to understand
	void GameLoop();

	// One of the ways to glue systems tgt. 
	//void BroadcastMessage(Message* message);

	static int WindowWidth, WindowHeight;

private:
	bool GameRunning;
	//std::vector<System*> Systems;

};

//A global pointer to the instance of the core
extern CoreEngine* Core;

