// ==================================================================================
// GraphicsManager.h  
// Author: Goh Pin Kai
// Brief: 
// ==================================================================================

#pragma once
#include "pch/pch.h"
#include "glapp.h"
#include "Core/System.h"
#include "Core/SystemManager.h"
#include <iostream>
#include <filesystem>
#include "stb_image.h"
#include <chrono>
#include <vector>
#include <string.h>
#include "Transform.h"
#include"../Physics/collision.h"

class GLApp;                    
namespace Framework { class Transform; } 
namespace Framework { class Systems; }   
class GraphicsManager : public Systems
{
public:
	GraphicsManager();
	~GraphicsManager(); // destructor

	void Initialize();
	void Update();
	void Shutdown();


private:
	GLApp* glapp;
};

