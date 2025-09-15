///*********************************************************************************************
// \file      glappSystem.h
// \par		Cat Feeders
// \author    Goh pin Kai (pinkai.g@digipen.edu) 75%
//			Michelle Han Jian Ping (m.han@digipen.edu) 25%
//
//
//\copyright All content   2024 DigiPen Institute of Technology Singapore. All rights reserved.
//**********************************************************************************************/
//#pragma once //Makes sure this header is only included once
//// Extension loader library's header must be included before GLFW's header!!!
//#include "glhelper.h"
//#include "glapp.h"
////#include "../Log/Log.h"
////#include <spdlog/spdlog.h>
////#include <spdlog/sinks/stdout_color_sinks.h>
//#include <iostream>
//#include <filesystem>
//#include "stb_image.h"
//#include <chrono>
//#include <vector>
//#include <string.h>
////#include "../Engine/ISystem.h"
////#include "../Engine/Core.h"
////#include "../Engine/Message.h"
////#include"../Math/Collision.h"
//
//namespace Framework
//{
//	class glappSystem : public ISystem {
//	public:
//		void SendMessageMess(Message* message);
//
//		///All systems are updated every game frame.
//		void Update(float timeslice);
//
//		///All systems provide a string name for debugging.
//		std::string GetName();
//
//		///Initialize the system.
//		void Initialize();
//
//		///All systems need a virtual destructor to have their destructor called 
//		glappSystem();
//		~glappSystem();
//
//		void NewImage();
//	private:
//		GLApp *glapp;
//	};
//}