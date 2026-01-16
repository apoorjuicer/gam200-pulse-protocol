/******************************************************************************/
/**
 * @file       SceneManager.cpp
 * @project    Pulse Protocol
 * @author     Chia Wei Xuan Rachael (primary) - 95%
 * @author     Goh Pin Kai (secondary) - 5%
 * @brief      Manages scene lifecycle, serialization, and entity ownership across
 *             game scenes (MainMene and level1 for now) .
 *
 *             Core Responsibilities:
 *             - Scene Loading: Parses JSON scene files, creates entities with components,
 *               registers GUI buttons, and assigns default LayerTag if missing
 *             - Scene Unloading: Destroys all entities in SceneEntityList and clears tracking
 *             - Scene Saving: Serializes current scene state to JSON with all entity
 *               components (Transform, Renderable, Animation, LogicComponent, Health,
 *               AABB, GUIButton, Name, NamingComponent)
 *             - Scene Tracking: Maintains SceneEntityList for ownership, PreviousScene
 *               for save operations, CurrentScene for active scene name
 *
 * @copyright  Copyright (C) 2025 DigiPen Institute of Technology.
 *             Reproduction or disclosure of this file or its contents without the
 *             prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#pragma once
#include "SceneManager.h"
#include "GameLogic/GameLogic.h"
#include "Graphics/Transform.h"
#include "Graphics/Animation.h"
#include "Graphics/Renderable.h"
#include "Graphics/Text.h"
#include "CoreEngine/Assets.h"
#include "CoreEngine/Prefab/PrefabInstance.h"
#include "Components/Health.h"
#include "Components/EntityType.h"
#include "Components/DisplayName.h"
#include "CoreEngine/Factory/Factory.h"
#include "Graphics/Layer.h"
#include "CoreEngine/Core/CoreEngine.h"  // For GetFunctionPtr
//#include "../Components/AABB.h"

extern bool g_countdownFinished;

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

	if (CurrentScene != PreviousScene)
	{
		// Save the scene we're LEAVING for ESC navigation
		LastScene = PreviousScene;

		std::string NewScenePath = "../../PulseEngine/JSON/" + CurrentScene + ".json";
		LoadSceneFromJson(NewScenePath);

		// Update PreviousScene for change detection next frame
		PreviousScene = CurrentScene;
	}
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

	// Use the stored path from when we loaded the scene
	std::filesystem::path path = LoadedScenePath;

	// Fallback if path is empty (shouldn't happen, but just in case)
	if (path.empty())
	{
		path = "../../../PulseEngine/JSON/" + CurrentScene + ".json";
		std::cerr << "[SceneManager] Warning: Using fallback path!\n";
	}

#ifdef _DEBUG
	std::cout << "[SceneManager] Saving to: " << path << std::endl;
#endif

	// Save scene name first
	Value SceneName;
	SceneName.SetString(CurrentScene.c_str(), static_cast<SizeType>(CurrentScene.length()), alloc);
	d.AddMember("scene", SceneName, alloc);

	// First we make a container for entity
	Value JsonEntityArray(kArrayType);

	// save one entity one at a time into 1 Json file
	for (auto const& entity : SceneEntityList)
	{
		// Skip engine-managed entities (names starting with underscore)
		if (g_coordinator->HasComponent<Name>(entity)) {
			auto& name = g_coordinator->GetComponent<Name>(entity).name;
			if (!name.empty() && name[0] == '_') {
				continue;  // Don't save _FPS_Display, _TopSystem_Display, etc.
			}
		}

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

		}

		if (g_coordinator->HasComponent<Framework::Transform>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value TransfromVal;

			auto& script = g_coordinator->GetComponent<Framework::Transform>(entity);
			script.Serialize(TransfromVal, alloc);

			ComponentObj.AddMember("transform", TransfromVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);

		}

		if (g_coordinator->HasComponent<Framework::Renderable>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value RenderVal;

			auto& render = g_coordinator->GetComponent<Framework::Renderable>(entity);

			/*std::cout << "[DEBUG] Saving entity with spriteName: " << render.spriteName << std::endl;*/

			render.Serialize(RenderVal, alloc);

			ComponentObj.AddMember("render", RenderVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);

		}

		if (g_coordinator->HasComponent<Framework::Animation>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value AnimationVal;

			auto& animation = g_coordinator->GetComponent<Framework::Animation>(entity);
			animation.Serialize(AnimationVal, alloc);

			ComponentObj.AddMember("animation", AnimationVal, alloc);
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

		if (g_coordinator->HasComponent<Name>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value NameVal;

			auto& name = g_coordinator->GetComponent<Name>(entity);
			name.Serialize(NameVal, alloc);

			ComponentObj.AddMember("name", NameVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);

		}

		if (g_coordinator->HasComponent<Health>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value HealthVal;

			auto& health = g_coordinator->GetComponent<Health>(entity);
			health.Serialize(HealthVal, alloc);

			ComponentObj.AddMember("Health", HealthVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);

		}

		if (g_coordinator->HasComponent<AABB>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value AABBVal;

			auto& aabb = g_coordinator->GetComponent<AABB>(entity);
			aabb.Serialize(AABBVal, alloc);

			ComponentObj.AddMember("AABB", AABBVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);

		}

		// Save PrefabInstance component if entity has it
		if (g_coordinator->HasComponent<PrefabInstance>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value PrefabInstanceVal;

			auto& prefabInstance = g_coordinator->GetComponent<PrefabInstance>(entity);
			prefabInstance.Serialize(PrefabInstanceVal, alloc);

			ComponentObj.AddMember("prefabInstance", PrefabInstanceVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);
		}

		// Save LayerTag component if entity has it
		if (g_coordinator->HasComponent<LayerTag>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value LayerTagVal;

			auto& layerTag = g_coordinator->GetComponent<LayerTag>(entity);
			layerTag.Serialize(LayerTagVal, alloc);

			ComponentObj.AddMember("layerTag", LayerTagVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);
		}

		// Save AudiOsource component if entity has it
		if (g_coordinator->HasComponent<AudioSource>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value AudioSourceVal;

			auto& audioSrc = g_coordinator->GetComponent<AudioSource>(entity);
			audioSrc.Serialize(AudioSourceVal, alloc);

			ComponentObj.AddMember("audiosource", AudioSourceVal, alloc);
			componentsArray.PushBack(ComponentObj, alloc);
		}

		// Save TextComponent if entity has it
		if (g_coordinator->HasComponent<TextComponent>(entity))
		{
			Value ComponentObj(rapidjson::kObjectType);
			Value TextVal;

			auto& textComp = g_coordinator->GetComponent<TextComponent>(entity);
			textComp.Serialize(TextVal, alloc);

			ComponentObj.AddMember("TextComponent", TextVal, alloc);
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
	//fs::create_directories("../../PulseEngine/JSON");  // store in folder Json
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
	std::cout << "[SceneManager::UnloadCurrentScene] Got coordinator: " << (g_coordinator ? "YES" : "NO") << std::endl;
	std::cout << "[SceneManager::UnloadCurrentScene] SceneEntityList size: " << SceneEntityList.size() << std::endl;

	// Call Exit() on all scripts BEFORE destroying entities
	auto logicSystem = g_coordinator->GetSystem<LogicSystem>();
	std::cout << "[SceneManager::UnloadCurrentScene] Got LogicSystem: " << (logicSystem ? "YES" : "NO") << std::endl;

	/*if (logicSystem) {
		std::cout << "[SceneManager::UnloadCurrentScene] Calling Exit() on scripts..." << std::endl;
		int exitCount = 0;
		for (auto const& entity : SceneEntityList) {
			std::cout << "  -> Checking entity " << entity << std::endl;
			if (g_coordinator->HasComponent<LogicComponent>(entity)) {
				std::cout << "  -> Entity " << entity << " has LogicComponent" << std::endl;
				auto& logic = g_coordinator->GetComponent<LogicComponent>(entity);
				auto script = logicSystem->GrabScript(logic.ScriptName);
				if (script) {
					std::cout << "  -> Calling Exit() on " << logic.ScriptName << " for entity " << entity << std::endl;
					script->Exit(entity);
					exitCount++;
				}
				else {
					std::cout << "  -> WARNING: Could not get script for " << logic.ScriptName << std::endl;
				}
			}
			else {
				std::cout << "  -> Entity " << entity << " does NOT have LogicComponent" << std::endl;
			}
		}
		std::cout << "[SceneManager::UnloadCurrentScene] Total Exit() calls: " << exitCount << std::endl;
	}
	else {
		std::cout << "[SceneManager::UnloadCurrentScene] WARNING: LogicSystem not found!" << std::endl;
	}*/

	// Stop AudioSource channels before destroying entities
	auto* g_coord = Coordinator::GetInstance();
	auto audioMgr = g_coord->GetSystem<AudioManager>();
	if (audioMgr) {
		for (auto const& entity : SceneEntityList) {
			if (g_coord->HasComponent<AudioSource>(entity)) {
				auto& audioSrc = g_coord->GetComponent<AudioSource>(entity);
				if (audioSrc.channel) {
					audioSrc.channel->stop();
					audioSrc.channel = nullptr;
				}
			}
		}
	}


	// Clear the Csharp script when scene change
	std::cout << "[SceneManager::UnloadCurrentScene] Clearing C# scripts..." << std::endl;
	if (m_coreEngine && m_coreEngine->clearScriptsFunc)
	{
		m_coreEngine->clearScriptsFunc();
	}

	else
	{
		std::cout << "[SceneManager::UnloadCurrentScene] WARNING: CoreEngine pointer not set!" << std::endl;
	}


	// Delete all the entity from SceneEntity Container
	for (auto const& entity : SceneEntityList)
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
	// Reset countdown/beat state on scene load
	g_countdownFinished = false;

	UnloadCurrentScene();

	// Extract just the filename (e.g., "MainMenu.json", "Level1.json", etc.)
	std::filesystem::path inputPath = path;
	std::string filename = inputPath.filename().string();

	// Build correct absolute path to source folder
	LoadedScenePath = std::filesystem::absolute("../../../PulseEngine/JSON") / filename;

#ifdef _DEBUG
	std::cout << "\n================================================" << std::endl;
	std::cout << "       [SceneManager] Loading in progress!" << std::endl;
	std::cout << "================================================" << std::endl;
#endif

	// just a for loop going thru all the entity arrray list and calling the factory function to create them 
	auto* g_coordinator = Coordinator::GetInstance();
	using namespace rapidjson;
	Factory factory;

	// USE LoadedScenePath instead of path for reading the file
	std::ifstream ifs(LoadedScenePath);
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

			// Load PrefabInstance component if present
			if (component.HasMember("prefabInstance") && component["prefabInstance"].IsObject())
			{
				PrefabInstance pi;
				pi.Deserialize(component["prefabInstance"]);
				g_coordinator->AddComponent<PrefabInstance>(entity, pi);
			}

			// Load LayerTag
			if (component.HasMember("layerTag") && component["layerTag"].IsObject())
			{
				LayerTag layerTag;
				layerTag.Deserialize(component["layerTag"]);
				g_coordinator->AddComponent<LayerTag>(entity, layerTag);
			}

			if (component.HasMember("audiosource") && component["audiosource"].IsObject())
			{
				AudioSource audioSrc;
				audioSrc.Deserialize(component["audiosource"]);
				g_coordinator->AddComponent<AudioSource>(entity, audioSrc);
			}

			if (component.HasMember("TextComponent") && component["TextComponent"].IsObject())
			{
				TextComponent txtComp;
				txtComp.Deserialize(component["TextComponent"]);
				g_coordinator->AddComponent<TextComponent>(entity, txtComp);
			}
		}

		if (!g_coordinator->HasComponent<LayerTag>(entity)) {
			LayerTag tag{};

			// Sensible defaults:
			//if (g_coordinator->HasComponent<GUIButton>(entity)) {
			//	// UI stuff goes to UI layer and on top
			//	tag.mask = LAYER_UI;
			//	tag.orderInLayer = 2;
			//	tag.z = 0.0f;
			//}
			//else 

			// check for UI first
			//if (g_coordinator->HasComponent<UIElement>(entity)) {
			//	tag.mask = LAYER_UI;
			//	tag.orderInLayer = 10;
			//	tag.z = 100.0f;
			//}

			if (g_coordinator->HasComponent<Framework::Renderable>(entity)) {
				// Background quad heuristic: mdl_ref==0
				auto& r = g_coordinator->GetComponent<Framework::Renderable>(entity);
				const bool isBackground = (static_cast<int>(r.mdl_ref) == 0);
				tag.mask = isBackground ? LAYER_BACKGROUND : LAYER_WORLD;
				tag.orderInLayer = isBackground ? 0 : 1;
				tag.z = 0.0f;
			}
			else {
				// Everything else defaults to world
				tag.mask = LAYER_WORLD;
				tag.orderInLayer = 1;
				tag.z = 0.0f;
			}

			g_coordinator->AddComponent(entity, tag);
		}

		if (g_coordinator->HasComponent<Name>(entity)) {
			auto& name = g_coordinator->GetComponent<Name>(entity).name;
			if (!name.empty() && name[0] == '_') {
				//std::cout << "[SceneManager] Skipping engine-managed entity: " << name << std::endl;
				g_coordinator->DestroyEntity(entity);
				continue;  // Skip to next entity in JSON
			}
		}

		// save obj to a vector for deleting later
		SceneEntityList.push_back(entity);

		//std::cout << "[SceneManager] Created " << SceneEntityList.size() << " entities" << std::endl;
		//std::cout << "[SceneManager] Total entities in system: "
		//	<< g_coordinator->GetAllEntities().size() << std::endl;
	}

	ChangeCurrentScene(d["scene"].GetString());


	// Add them to C# script container 
	auto addScriptFunc = m_coreEngine->GetFunctionPtr<bool(*)(int, const char*)>(
		"ScriptAPI",
		"ScriptAPI.EngineInterface",
		"AddScriptViaName"
	);

	std::cout << "[SceneManager] addScriptFunc = " << (addScriptFunc ? "FOUND" : "NULL") << "\n";
	std::cout << "[SceneManager] SceneEntityList.size() = " << SceneEntityList.size() << "\n";


	/*for (Entity entity : SceneEntityList)
	{
		if (g_coordinator->HasComponent<LogicComponent>(entity))
		{
			auto& logic = g_coordinator->GetComponent<LogicComponent>(entity);

			for (const auto& scriptName : logic.ScriptName)
			{
				if (!scriptName.empty())
				{
					addScriptFunc(entity, scriptName.c_str());
				}
			}
		}
	}*/

	if (!addScriptFunc)
	{
		std::cerr << "[SceneManager] CRITICAL ERROR: AddScriptViaName not found!\n";
		std::cerr << "  Script engine may not be initialized!\n";
	}
	else
	{
		int totalAttached = 0;

		for (Entity entity : SceneEntityList)
		{
			std::cout << "[SceneManager] Checking entity " << entity << "\n";

			if (g_coordinator->HasComponent<LogicComponent>(entity))
			{
				auto& logic = g_coordinator->GetComponent<LogicComponent>(entity);
				std::cout << "  -> Has LogicComponent\n";
				std::cout << "  -> ScriptName.size() = " << logic.ScriptName.size() << "\n";

				for (size_t i = 0; i < logic.ScriptName.size(); ++i)
				{
					const auto& scriptName = logic.ScriptName[i];
					std::cout << "  -> ScriptName[" << i << "] = '" << scriptName << "'\n";

					if (!scriptName.empty())
					{
						std::cout << "  -> Calling AddScriptViaName(" << entity << ", '" << scriptName << "')\n";

						bool success = addScriptFunc(entity, scriptName.c_str());

						std::cout << "  -> Result: " << (success ? "SUCCESS" : "FAILED") << "\n";

						if (success)
						{
							totalAttached++;
						}
						else
						{
							std::cerr << "  -> ERROR: Failed to attach '" << scriptName << "' to entity " << entity << "\n";
						}
					}
					else
					{
						std::cout << "  -> Skipped (empty name)\n";
					}
				}
			}
			else
			{
				std::cout << "  -> No LogicComponent\n";
			}
		}

		std::cout << "\n[SceneManager] ========== SUMMARY ==========\n";
		std::cout << "[SceneManager] Total entities checked: " << SceneEntityList.size() << "\n";
		std::cout << "[SceneManager] Total scripts attached: " << totalAttached << "\n";
		std::cout << "[SceneManager] ============================\n";
	}


	// Initialize all scripts for entities with LogicComponent
	//auto logicSystem = g_coordinator->GetSystem<LogicSystem>();
	//if (logicSystem) {
	//	//std::cout << "[SceneManager] Initializing scripts for loaded entities..." << std::endl;
	//	//std::cout << "[SceneManager] Total entities in SceneEntityList: " << SceneEntityList.size() << std::endl;

	//	// DEBUG: Show ALL entities in list
	//	for (Entity entity : SceneEntityList) {
	//		std::string name = "Unnamed";
	//		if (g_coordinator->HasComponent<Name>(entity)) {
	//			name = g_coordinator->GetComponent<Name>(entity).name;
	//		}
	//		/*bool hasScript = g_coordinator->HasComponent<LogicComponent>(entity);*/
	//	}

	//	int scriptCount = 0;
	//	for (Entity entity : SceneEntityList) {
	//		if (g_coordinator->HasComponent<LogicComponent>(entity)) {
	//			auto& logic = g_coordinator->GetComponent<LogicComponent>(entity);
	//			auto script = logicSystem->GrabScript(logic.ScriptName);
	//			if (script) {
	//				script->Init(entity);
	//				scriptCount++;
	//			}
	//		}
	//	}
	//}

	PreviousScene = CurrentScene;

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