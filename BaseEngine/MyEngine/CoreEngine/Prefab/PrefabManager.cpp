/******************************************************************************/
/**
 * @file        PrefabManager.cpp
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Implements the InputManager system, handling GLFW-based
 *              keyboard input for rhythm gameplay. Provides real-time input
 *              state tracking, combo detection, rhythm timing feedback,
 *              event broadcasting to subscribed systems, debug toggles.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "PrefabManager.h"
#include "../Factory/Factory.h"

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
	return factory->SaveEntityToJson(entity, prefabPath);
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
