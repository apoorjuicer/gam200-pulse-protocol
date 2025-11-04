/******************************************************************************/
/**
 * @file       SceneManager.cpp
 * @project    Pulse Protocol
 * @author     Chia Wei Xuan Rachael
 * @brief      Implementation of scene management system that handles loading,
 *             unloading, and transitioning between game scenes (MainMenu, Pause,
 *             Level1, Dead). Manages scene-specific entity creation/destruction
 *             and maintains a list of entities belonging to the current scene
 * 
 * @copyright  Copyright (C) 2025 DigiPen Institute of Technology.
 *             Reproduction or disclosure of this file or its contents without the
 *             prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#pragma once
#include "SceneManager.h"
#include "../GameLogic/GameLogic.h"
#include "../Graphics/Transform.h"
#include "../Graphics/Animation.h"
#include "../Graphics/Renderable.h"
//#include "../Components/AABB.h"
#include "../CoreEngine/Assets.h"
#include "../Components/Health.h"
#include "../Components/EntityType.h"
#include "../MyEngine/Components/DisplayName.h"
#include "CoreEngine/Factory/Factory.h"


//Starting scene will be Main Menu
SceneManager::SceneManager()
{
	
}

void SceneManager::Init()
{
	// Starting scene will be main menu
	LoadSceneFromJson(StartingScene);
	PreviousScene = CurrentScene;

	// testing for it it works 
	//std::string testpath = "MainMenu";
	//CurrentScene = testpath;
}

void SceneManager::Update(float dt)
{
	(void)dt;
	/*if (dt == 0)
	{
		std::cout << dt << std::endl;
	}*/

	//if (CurrentScene != PreviousScene)
	//{
	//	//SaveCurrentScene(); // normally just for ImGui but can use here for testing

	//	std::string NewScenePath = "../../MyEngine/JSON/" + CurrentScene +".json";
	//	LoadSceneFromJson(NewScenePath);
	//}

	// update previous scene for the next loop
	PreviousScene = CurrentScene;

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
	//std::filesystem::path path = static_cast<std::filesystem::path>("../../MyEngine/JSON/Saved" + PreviousScene + "Scene.json"); 
	std::filesystem::path path = "../../MyEngine/JSON/Saved" + PreviousScene + "Scene.json"; // file we want to save data into
	//std::string path = "../../MyEngine/JSON/" + CurrentScene + ".json"; // file we want to save data into

	// Save scene name first
	Value SceneName;
	SceneName.SetString(PreviousScene.c_str(), static_cast<SizeType>(PreviousScene.length()), alloc);
	d.AddMember("scene", SceneName, alloc);

	// First we make a container for entity
	Value JsonEntityArray(kArrayType);

	// save one entity one at a time into 1 Json file
	for (auto const& entity: g_coordinator->GetAllEntities())
	{
		// We make an object first for JsonEntityArray[i]
		Value entityObj(kObjectType);

		// tehn for our components, we make it an array of components
		Value componentsArray(kArrayType);

		// check all the components if they have it
		if (g_coordinator->HasComponent<NamingComponent>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value NameVal;

			auto& nam = g_coordinator->GetComponent<NamingComponent>(entity);
			nam.Serialize(NameVal, alloc);

			ComponentObj.AddMember("Typing", NameVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);
			continue;
		}

		if (g_coordinator->HasComponent<Framework::Transform>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value TransfromVal;

			auto& script = g_coordinator->GetComponent<Framework::Transform>(entity);
			script.Serialize(TransfromVal, alloc);

			ComponentObj.AddMember("transform", TransfromVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);
			continue;
		}

		if (g_coordinator->HasComponent<Framework::Renderable>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value RenderVal;

			auto& render = g_coordinator->GetComponent<Framework::Renderable>(entity);
			render.Serialize(RenderVal, alloc);

			ComponentObj.AddMember("render", RenderVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);
			continue;
		}

		if (g_coordinator->HasComponent<Framework::Animation>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value AnimationVal;

			auto& animation = g_coordinator->GetComponent<Framework::Animation>(entity);
			animation.Serialize(AnimationVal, alloc);

			ComponentObj.AddMember("animation", AnimationVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);
			continue;
		}

		if (g_coordinator->HasComponent<LogicComponent>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value ScriptVal;

			auto& script = g_coordinator->GetComponent<LogicComponent>(entity);
			script.Serialize(ScriptVal, alloc);

			ComponentObj.AddMember("script", ScriptVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);
			continue;
		}

		// purely for testing in-game GUI system
		if (g_coordinator->HasComponent<GUIButton>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value ButtonVal;

			auto& btn = g_coordinator->GetComponent<GUIButton>(entity);
			btn.Serialize(ButtonVal, alloc);

			ComponentObj.AddMember("ui", ButtonVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);
			continue;
		}

		if (g_coordinator->HasComponent<Name>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value NameVal;

			auto& name = g_coordinator->GetComponent<Name>(entity);
			name.Serialize(NameVal, alloc);

			ComponentObj.AddMember("name", NameVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);
			continue;
		}

		if (g_coordinator->HasComponent<Health>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value HealthVal;

			auto& health = g_coordinator->GetComponent<Health>(entity);
			health.Serialize(HealthVal, alloc);

			ComponentObj.AddMember("Health", HealthVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);
			continue;
		}

		if (g_coordinator->HasComponent<AABB>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value AABBVal;

			auto& aabb = g_coordinator->GetComponent<AABB>(entity);
			aabb.Serialize(AABBVal, alloc);

			ComponentObj.AddMember("AABB", AABBVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);
			continue;
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
	//fs::create_directories("../../MyEngine/JSON");  // store in folder Json
	fs::create_directories(path.parent_path());  // store in folder 

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
	std::cout << "  [SceneManager]: " << PreviousScene << " has been saved!" << std::endl;
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
		std::cout << "\n================================================" << std::endl;
		std::cout << "    [SceneManager] Scene successfully unloaded!" << std::endl;
		std::cout << "================================================" << std::endl;
	#endif
}

void SceneManager::LoadSceneFromJson(std::string const& path) // TAKES IN SCENE JSON FILE 
{
	UnloadCurrentScene();

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

		//Entity entity = factory.DefaultObj();	   // create entity with transform and render component.
		Entity entity = g_coordinator->CreateEntity();
		const auto& components = JsonEntity["components"];  // Get the components array

		// array of all the component in this entity
		for (SizeType j = 0; j < components.Size(); ++j)
		{
			const auto& component = components[j];

			if (component.HasMember("Typing") && component["Typing"].IsObject())
			{
				NamingComponent n;
				n.Deserialize(component["Typing"]);
				g_coordinator->AddComponent<NamingComponent>(entity, n);
			}

			if (component.HasMember("transform") && component["transform"].IsObject())
			{
				//void AddTransformComponent(entity)

				/*void AddTransformComponent(Entity entity);
				{
					Framework::Transform t;
					t.Deserialize(component["transform"]);
					g_coordinator->AddComponent<Framework::Transform>(entity, t);
				}*/
				Framework::Transform t;
				t.Deserialize(component["transform"]);
				g_coordinator->AddComponent<Framework::Transform>(entity, t);

				// DEBUG: Print what we just loaded
				//std::cout << "[SceneManager] Loaded transform at ("
				//	<< t.Pos.x << ", " << t.Pos.y << ")" << std::endl;
			}

			if (component.HasMember("render") && component["render"].IsObject())
			{
				Framework::Renderable r;
				r.Deserialize(component["render"]);
				g_coordinator->AddComponent<Framework::Renderable>(entity, r);
			}

			if (component.HasMember("animation") && component["animation"].IsObject())
			{
				Framework::Animation a;
				a.Deserialize(component["animation"]);
				g_coordinator->AddComponent<Framework::Animation>(entity, a);
			}

			if (component.HasMember("script") && component["script"].IsObject())
			{
				LogicComponent l;
				l.Deserialize(component["script"]);
				g_coordinator->AddComponent<LogicComponent>(entity, l);
			}

			if (component.HasMember("name") && component["name"].IsObject())
			{
				Name n;
				n.Deserialize(component["name"]);
				g_coordinator->AddComponent<Name>(entity, n);
			}

			if (component.HasMember("Health") && component["Health"].IsObject())
			{
				Health h;
				h.Deserialize(component["Health"]);
				g_coordinator->AddComponent<Health>(entity, h);
			}

			if (component.HasMember("AABB") && component["AABB"].IsObject())
			{
				AABB aabb;
				aabb.Deserialize(component["AABB"]);
				g_coordinator->AddComponent<AABB>(entity, aabb);
			}

			// this is purely for the in-game basic GUI system & on screen UI elements
			if (component.HasMember("ui") && component["ui"].IsObject())
			{
				GUIButton btn;
				btn.Deserialize(component["ui"]);
				g_coordinator->AddComponent<GUIButton>(entity, btn);
			}
		}
			
		// save obj to a vector for deleting later
		SceneEntityList.push_back(entity);
	}

		auto* coordinator = Coordinator::GetInstance();
		auto inputMgr = coordinator->GetSystem<InputManager>();

		if (inputMgr) {
			inputMgr->GUIButtonEntities.clear();

			for (auto entity : SceneEntityList) {
				if (coordinator->HasComponent<GUIButton>(entity)) {
					inputMgr->GUIButtonEntities.insert(entity);
#ifdef _DEBUG
					std::cout << "[SceneManager] Registered GUI button entity: " << entity << std::endl;
#endif
				}
			}

#ifdef _DEBUG
			std::cout << "[SceneManager] Total GUI buttons: " << inputMgr->GUIButtonEntities.size() << std::endl;
#endif
		}

		ChangeCurrentScene(d["scene"].GetString());
#ifdef _DEBUG
		std::cout << "\n===================================================" << std::endl;
		std::cout << "   [Scene Manager]: " << CurrentScene << " loaded successfully!" << std::endl;
		std::cout << "===================================================" << std::endl;
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