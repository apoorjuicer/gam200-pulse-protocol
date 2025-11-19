/******************************************************************************/
/**
 * @file        PrefabManager.cpp
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Implements the PrefabManager system for entity template management.
 *              Provides functionality to save entities as JSON-based prefabs,
 *              instantiate prefabs into active scenes, and enumerate available
 *              prefabs from the asset directory.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "PrefabManager.h"
#include "PrefabInstance.h"
#include "../Factory/Factory.h"
#include "../MyEngine/Components/DisplayName.h"

// include component headers for UpdateEntityFromPrefab func
#include "../MyEngine/Graphics/Transform.h"
#include "../MyEngine/Graphics/Renderable.h"
#include "../MyEngine/Graphics/Animation.h"
#include "../MyEngine/GameLogic/GameLogic.h"
#include "../MyEngine/Components/Health.h"
#include "../MyEngine/Components/AABB.h"

//#pragma warning(push, 0)
//#include <rapidjson/document.h>
//#include <rapidjson/istreamwrapper.h>
//#pragma warning(pop)

PrefabManager::PrefabManager()
{
	factory = new Factory();
}

PrefabManager::~PrefabManager()
{
	delete factory;
}

bool PrefabManager::SaveEntityAsPrefab(Entity entity, std::string const& prefabName)
{
	namespace fs = std::filesystem;

	fs::path prefabPath = prefabDirectory + prefabName + ".json";

	// if directory does not exist, create it
	fs::create_directories(fs::path(prefabDirectory));

	// use Factory's existing save single entity func
	bool success = factory->SaveEntityToJson(entity, prefabPath);

	if (success) {
		// AUTO-LINK: If this entity doesn't have PrefabInstance component, add it
		auto* coord = Coordinator::GetInstance();
		if (!coord->HasComponent<PrefabInstance>(entity)) {
			PrefabInstance prefabInstance;
			prefabInstance.prefabName = prefabName;
			prefabInstance.isLinked = true;
			coord->AddComponent<PrefabInstance>(entity, prefabInstance);

			std::cout << "[PrefabManager] Auto-linked entity " << entity
				<< " to prefab '" << prefabName << "'" << std::endl;
		}
	}

	return success;
}

std::optional<Entity> PrefabManager::InstantiatePrefab(std::string const& prefabName)
{
	// strip .json extension if prsent
	std::string cleanName = prefabName;
	if (cleanName.ends_with(".json")) {
		cleanName = cleanName.substr(0, cleanName.length() - 5);
	}

	std::string prefabPath = GetPrefabPath(cleanName);

	// if the prefab path is wrong/does not exist
	if (!std::filesystem::exists(prefabPath)) {
		std::cerr << "[PrefabManager] Prefab not found: " << prefabName << std::endl;
		return std::nullopt;
	}

	auto result = factory->CreateEntityFromJson(prefabPath);

	if (result.has_value()) {
		// set entity name without the .json extension
		Entity entity = result.value();
		auto* coord = Coordinator::GetInstance();

		// track that this entity came from this prefab
		PrefabInstance prefabInstance;
		prefabInstance.prefabName = cleanName;
		prefabInstance.isLinked = true;
		coord->AddComponent<PrefabInstance>(entity, prefabInstance);

		// if entity has name component alr then just set it to that name in the JSON file
		if (coord->HasComponent<Name>(entity)) {
			coord->GetComponent<Name>(entity).name = cleanName;
		}
		else {
			// else add Name component if it doesn't exist
			Name nm;
			nm.name = cleanName;
			coord->AddComponent<Name>(entity, nm);
		}

		//std::cout << "[PrefabManager] Successfully instantiated: " << cleanName << " (Entity " << entity << ")" << std::endl;
	}
	else {
		std::cerr << "[PrefabManager] Failed to create entity from: " << prefabPath << std::endl;
	}

	return result;
}

bool PrefabManager::DeletePrefab(std::string const& prefabName) {
	namespace fs = std::filesystem;

	// strip .json extension if present
	std::string cleanName = prefabName;
	if (cleanName.ends_with(".json")) {
		cleanName = cleanName.substr(0, cleanName.length() - 5);
	}

	fs::path prefabPath = GetPrefabPath(cleanName);

	// check if file exists
	if (!fs::exists(prefabPath)) {
		std::cerr << "[PrefabManager] Cannot delete - prefab not found: " << cleanName << std::endl;
		return false;
	}

	try {
		fs::remove(prefabPath);
		std::cout << "[PrefabManager] Successfully deleted prefab: " << cleanName << std::endl;
		return true;
	}
	catch (const fs::filesystem_error& e) {
		std::cerr << "[PrefabManager] Failed to delete prefab: " << e.what() << std::endl;
		return false;
	}
}


std::vector<PrefabData> PrefabManager::GetAvailablePrefabs()
{
	std::vector<PrefabData> prefabs;

	// automatically create a directory for prefab, if it does not exist
	if (!std::filesystem::exists(prefabDirectory)) {
		std::filesystem::create_directories(prefabDirectory);
		return prefabs;
	}

	// this loops through every file/folder in the directory
	for (auto const& entry : std::filesystem::directory_iterator(prefabDirectory)) {
		if (entry.path().extension() == ".json") {

			// create a struct to hold the data of the prefab
			PrefabData data;

			// get filename without the .json extension (etc. "EnemyPrefab.json" -> "EnemyPrefab")
			data.prefabName = entry.path().stem().string();

			// this gets the full file path
			data.filePath = entry.path().string();

			prefabs.push_back(data);
		}
	}

	return prefabs;
}

// this is just to get the path to the specific prefab
std::string PrefabManager::GetPrefabPath(std::string const& prefabName)
{
	return prefabDirectory + prefabName + ".json";
}

void PrefabManager::UpdateAllInstancesOfPrefab(std::string const& prefabName)
{
	auto* coord = Coordinator::GetInstance();

	// Collect all entities that have PrefabInstance component and match this prefab
	// We iterate through all possible entities since we don't have GetEntitiesWithComponent
	std::vector<Entity> prefabInstances;
	for (Entity entity = 0; entity < MaxEntity; ++entity) {
		if (coord->HasComponent<PrefabInstance>(entity)) {
			auto& prefabInstance = coord->GetComponent<PrefabInstance>(entity);

			// If this entity was created from this prefab and is still linked
			if (prefabInstance.prefabName == prefabName && prefabInstance.isLinked) {
				prefabInstances.push_back(entity);
			}
		}
	}

	// Now update all collected instances
	int updateCount = 0;
	for (Entity entity : prefabInstances) {
		std::string prefabPath = GetPrefabPath(prefabName);

		// Update this entity's components from the prefab file
		if (UpdateEntityFromPrefab(entity, prefabPath)) {
			updateCount++;
			std::cout << "[PrefabManager] Updated entity " << entity
				<< " from prefab: " << prefabName << std::endl;
		}
		else {
			std::cerr << "[PrefabManager] Failed to update entity " << entity << std::endl;
		}
	}

	std::cout << "[PrefabManager] Updated " << updateCount << " instances of prefab: "
		<< prefabName << std::endl;
}

bool PrefabManager::UpdateEntityFromPrefab(Entity entity, std::string const& prefabPath)
{
	auto* coord = Coordinator::GetInstance();

	std::ifstream ifs(prefabPath);
	if (!ifs.is_open()) {
		std::cerr << "[PrefabManager] Cannot open prefab file: " << prefabPath << "\n";
		return false;
	}

	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document d;
	d.ParseStream(isw);

	if (!(d.HasMember("components") && d["components"].IsArray())) {
		std::cerr << "[PrefabManager] Invalid prefab format!" << std::endl;
		return false;
	}

	const auto& components = d["components"];

	// Remove old components (except PrefabInstance, Name, LayerTag)
	if (coord->HasComponent<Framework::Transform>(entity))
		coord->RemoveComponent<Framework::Transform>(entity);
	if (coord->HasComponent<Framework::Renderable>(entity))
		coord->RemoveComponent<Framework::Renderable>(entity);
	if (coord->HasComponent<Framework::Animation>(entity))
		coord->RemoveComponent<Framework::Animation>(entity);
	if (coord->HasComponent<LogicComponent>(entity))
		coord->RemoveComponent<LogicComponent>(entity);
	if (coord->HasComponent<Health>(entity))
		coord->RemoveComponent<Health>(entity);
	if (coord->HasComponent<AABB>(entity))
		coord->RemoveComponent<AABB>(entity);
	if (coord->HasComponent<UIElement>(entity))
		coord->RemoveComponent<UIElement>(entity);

	// Add updated components from prefab
	for (rapidjson::SizeType i = 0; i < components.Size(); ++i)
	{
		const auto& component = components[i];

		if (component.HasMember("transform") && component["transform"].IsObject()) {
			Framework::Transform t;
			t.Deserialize(component["transform"]);
			coord->AddComponent<Framework::Transform>(entity, t);
		}

		if (component.HasMember("render") && component["render"].IsObject()) {
			Framework::Renderable r;
			r.Deserialize(component["render"]);
			coord->AddComponent<Framework::Renderable>(entity, r);
		}

		if (component.HasMember("animation") && component["animation"].IsObject()) {
			Framework::Animation a;
			a.Deserialize(component["animation"]);
			coord->AddComponent<Framework::Animation>(entity, a);
		}

		if (component.HasMember("script") && component["script"].IsObject()) {
			LogicComponent l;
			l.Deserialize(component["script"]);
			coord->AddComponent<LogicComponent>(entity, l);
		}

		if (component.HasMember("Health") && component["Health"].IsObject()) {
			Health h;
			h.Deserialize(component["Health"]);
			coord->AddComponent<Health>(entity, h);
		}

		if (component.HasMember("AABB") && component["AABB"].IsObject()) {
			AABB aabb;
			aabb.Deserialize(component["AABB"]);
			coord->AddComponent<AABB>(entity, aabb);
		}

		if (component.HasMember("ui") && component["ui"].IsObject()) {
			UIElement ui;
			ui.Deserialize(component["ui"]);
			coord->AddComponent<UIElement>(entity, ui);
		}
	}

	return true;
}
