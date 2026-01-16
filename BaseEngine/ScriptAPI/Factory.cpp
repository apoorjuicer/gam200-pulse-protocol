/******************************************************************************/
/**
* @file        Factory.cpp
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
#include <optional>
#include "Factory.h"
#include <msclr/marshal_cppstd.h>   // for converting C# to C++ string
#include "../CoreEngine/Factory/Factory.h"
#include "../CoreEngine/ECS/Coordinator.h"
#include "../CoreEngine/Prefab/PrefabManager.h"
#include "CoreEngine/Core/ScriptingBridge.h"

#undef generic

namespace ScriptAPI
{
	// In: Json name of the prefab u wanna spawn in 
	// Out: Entity ID that u can play with in script 
	int Factory::Instantiate(System::String^ scriptname)
	{
		if (System::String::IsNullOrEmpty(scriptname))
		{
			System::Console::WriteLine("Prefab is either empty or null\n");
			return -1;
		}

		// change c# string to c++ string
		msclr::interop::marshal_context context;
		std::string pathway = context.marshal_as<std::string>(scriptname);

		// Use prefab manager to load entity and create it from .json file 
		::PrefabManager prefab;
		auto new_entity = prefab.InstantiatePrefab(pathway);
		
		System::Console::WriteLine(new_entity.value());

		int entityID = static_cast<int>(new_entity.value());;
		return entityID;
	}

	// delete entity using factory delete function :D
	void Factory::Destroy(int entityID)
	{
		::Factory factory;
		factory.DeleteEntity(entityID);
	}


}