/******************************************************************************/
/**
* @file        Scene.cpp
* @project     Pulse Protocol
* @author      Chia Wei Xuan Rachael - 100%
* @brief
*
* @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/
#pragma once
#define generic generic_workaround
#include "Scene.h"
#include <msclr/marshal_cppstd.h>   // for converting C# to C++ string
#include "../CoreEngine/ECS/Coordinator.h"
#include "../CoreEngine/Scene/SceneManager.h"
#include "CoreEngine/Core/ScriptingBridge.h"
#undef generic

namespace ScriptAPI
{
	void Scene::LoadScene(System::String^ Filename)
	{
		if (System::String::IsNullOrEmpty(Filename))
		{
			System::Console::WriteLine("Filename is either empty or null\n");
		}


		// change c# string to c++ string
		msclr::interop::marshal_context context;
		std::string Filename_pathway = "../../PulseEngine/JSON/"  + context.marshal_as<std::string>(Filename) + ".json";

		auto* coord = Coordinator::GetInstance();
		auto sceneManager = coord->GetSystem<SceneManager>();
		sceneManager->LoadSceneFromJson(Filename_pathway);

		System::Console::WriteLine("Loaded new scene\n");
	}
}