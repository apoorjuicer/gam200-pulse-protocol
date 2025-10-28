/******************************************************************************/
/**
 * @file       SceneManager.cpp
 * @project    Pulse Protocol
 * @author     Chia Wei Xuan Rachael
 * @brief      Implementation of scene management system that handles loading,
 *             unloading, and transitioning between game scenes (MainMenu, Pause,
 *             Level1, Dead). Manages scene-specific entity creation/destruction
 *             and maintains a list of entities belonging to the current scene
 * @copyright  Copyright (C) 2025 DigiPen Institute of Technology.
 *             Reproduction or disclosure of this file or its contents without the
 *             prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#pragma once
#include "pch/pch.h"
#include "SceneManager.h"
#include "CoreEngine/Factory/Factory.h"

#pragma warning(push, 0)
#include <rapidjson/document.h>     
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h> 
#include <rapidjson/istreamwrapper.h>
#pragma warning(pop)


//Starting scene will be Main Menu
SceneManager::SceneManager()
{
	
}

void SceneManager::SaveCurrentScene()
{
#ifdef _DEBUG
	std::cout << "\n================================================" << std::endl;
	std::cout << "   [SceneManager]: Saving scene in progress!" << std::endl;
	std::cout << "================================================" << std::endl;
#endif

	using namespace rapidjson;
	auto* g_coordinator = Coordinator::GetInstance();
	Factory factory;

	Document d;
	d.SetObject();
	auto& alloc = d.GetAllocator();

	// grab scene name for the json file name
	std::string path = "../../MyEngine/JSON/Saved" + CurrentScene + "Scene.json"; // file we want to save data into

	// Save scene name first
	Value SceneName;
	SceneName.SetString(CurrentScene.c_str(), static_cast<SizeType>(CurrentScene.length()), alloc);
	d.AddMember("scene", SceneName, alloc);

	// First we make a container for entity
	Value JsonEntityArray(kArrayType);

	// save one entity one at a time into 1 Json file
	for (auto const& entity: SceneEntityList)
	{
		// We make an object first for JsonEntityArray[i]
		Value entityObj(kObjectType);

		// tehn for our components, we make it an array of components
		Value componentsArray(kArrayType);

		// check all the components if they have it
		if (g_coordinator->HasComponent<Framework::Transform>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value TransfromVal;

			auto& script = g_coordinator->GetComponent<Framework::Transform>(entity);
			script.Serialize(TransfromVal, alloc);

			ComponentObj.AddMember("transfrom", TransfromVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);
		}

		if (g_coordinator->HasComponent<LogicComponent>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value ScriptVal;

			auto& script = g_coordinator->GetComponent<LogicComponent>(entity);
			script.Serialize(ScriptVal, alloc);

			ComponentObj.AddMember("script", ScriptVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);
		}

		// So for this entity, we add compoennt array
		entityObj.AddMember("components", componentsArray, alloc);

		// we add the entity type and its compoents into 1 JsonEntityArray[i]
		JsonEntityArray.PushBack(entityObj, alloc); // add to entity array list
	}

	//make it to an actual array in Json file
	d.AddMember("entities", JsonEntityArray, alloc);

	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer); // Use PrettyWriter for readable JSON
	d.Accept(writer);

	namespace fs = std::filesystem;
	fs::create_directories("../../MyEngine/JSON");  // store in folder Json

	// check if can open, if can then put data into Json file
	std::ofstream ofs{ path };
	if (!ofs.is_open())
	{
		std::cerr << "Error: Cannot open file for writing: " << path << "\n";
		return;
	}

	ofs << buffer.GetString();
	if (!ofs.good())
	{
		std::cerr << "Error: Failed to write JSON to file: " << path << "\n";
		return;
	}
	
#ifdef _DEBUG
	std::cout << "\n================================================" << std::endl;
	std::cout << "  [SceneManager]: " << CurrentScene << " has been saved!" << std::endl;
	std::cout << "================================================" << std::endl;
#endif
}

void SceneManager::UnloadCurrentScene()
{
#ifdef _DEBUG
	std::cout << "\n================================================" << std::endl;
	std::cout << "       [SceneManager] Unloading in progress!" << std::endl;
	std::cout << "================================================" << std::endl;
#endif

	auto* g_coordinator = Coordinator::GetInstance();
	for(auto const& entity: SceneEntityList)
	{
		g_coordinator->DestroyEntity(entity);
	}

	//This class owns entity container is cleared
	SceneEntityList.clear();
	#ifdef _DEBUG
		std::cout << "Scene Unloaded" << std::endl;
	#endif
}

void SceneManager::LoadSceneFromJson(std::string const& path) // TAKES IN SCENE JSON FILE 
{
#ifdef _DEBUG
	std::cout << "\n================================================" << std::endl;
	std::cout << "       [SceneManager] Loading in progress!" << std::endl;
	std::cout << "================================================" << std::endl;
#endif

	// just a for loop going thru all the entity arrray list and calling the factory function to create them 
	auto* g_coordinator = Coordinator::GetInstance();
	using namespace rapidjson;
	Factory factory;

	std::ifstream ifs(path);
	if (!ifs.is_open())
	{
		std::cerr << "Error: Cannot open file for reading: " << path << "\n";
		return;
	}

	IStreamWrapper isw(ifs);
	Document d;
	d.ParseStream(isw);

	//check if Json file has a scene
	if (!(d.HasMember("scene")))
	{
		std::cerr << "scene is missing/wrong!" << std::endl;
		return;
	}

	// check if Json file has array of entities
	if (!(d.HasMember("entities")))
	{
		std::cerr << "entity array is missing/wrong!" << std::endl;
		return;
	}

	// Loop thru all the entity in the entity list
	const auto& SceneEntityArray = d["entities"];
	for (SizeType i = 0; i < SceneEntityArray.Size(); ++i)
	{
		auto const& JsonEntity = SceneEntityArray[i];
		// just creatre entity here so u dont have to call function 

		// check if there are compoent for each entity
		if (!(JsonEntity.HasMember("components") && JsonEntity["components"].IsArray()))
		{
			std::cerr << "Components for entities are missing/wrong!" << std::endl;
			continue;
		}

		Entity entity = factory.DefaultObj();	   // create entity with transform and render component.
		const auto& components = JsonEntity["components"];  // Get the components array

		// array of all the component in this entity
		for (SizeType j = 0; j < components.Size(); ++j)
		{
			const auto& component = components[j];

			if (component.HasMember("transform") && component["transform"].IsObject())
			{
				//void AddTransformComponent(entity)

				/*void AddTransformComponent(Entity entity);
				{
					Framework::Transform t;
					t.Deserialize(component["script"]);
					g_coordinator->AddComponent<Framework::Transform>(entity, t);
				}*/
				auto& change = g_coordinator->GetComponent<Framework::Transform>(entity);
				change.Deserialize(component["transform"]);
			}

			if (component.HasMember("script") && component["script"].IsObject())
			{
				LogicComponent l;
				l.Deserialize(component["script"]);
				g_coordinator->AddComponent<LogicComponent>(entity, l);
			}

		}

		// save obj to a vector for deleting later
		SceneEntityList.push_back(entity);
	}

		ChangeCurrentScene(d["scene"].GetString());
		
#ifdef _DEBUG
		std::cout << "\n===================================================" << std::endl;
		std::cout << "   [Scene Manager]: " << CurrentScene << " loaded successfully!" << std::endl;
		std::cout << "===================================================" << std::endl;
#endif
#ifdef DEBUG
		//std::cout << GrabCurrentScene << " Loaded Successfully" << std::endl;
#endif
}

void SceneManager::Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const
{
	using namespace rapidjson;
	out.SetObject();
	out.AddMember("SceneName", Value(CurrentScene.c_str(), alloc), alloc);
}

void SceneManager::Deserialize(const rapidjson::Value& in)
{
	if (!in.IsObject()) throw std::runtime_error("Invalid JSON for LogicComponent");
	if (in.HasMember("SceneName")) CurrentScene = in["SceneName"].GetString(); // already assigns it
}