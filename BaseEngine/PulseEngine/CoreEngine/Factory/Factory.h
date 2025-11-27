/******************************************************************************/
/**
 * @file        Factory.h
 * @project     Pulse Protocol
 * @author		Chia Wei Xuan Rachael
 * @brief		Entity factory system providing CRUD operations for individual game
 *              entities via JSON prefab files and programmatic creation. Serves as
 *              the primary interface for entity instantiation in both editor (ImGui)
 *              and runtime contexts (spawning bullets, enemies, etc.).
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#pragma once
#include "pch/pch_temp.h"	
#include "../MyEngine/Components/EntityType.h"
//#include "../Components/AABB.h"
#include "../MyEngine/Components/Health.h"
#include "../MyEngine/Graphics/Transform.h"
#include "../MyEngine/Graphics/Animation.h"
#include "../MyEngine/Graphics/Renderable.h"
#include "../MyEngine/GameLogic/GameLogic.h"
#include "../MyEngine/CoreEngine/ECS/Types.h"
#include "../MyEngine/CoreEngine/ECS/Coordinator.h"
#include "../MyEngine/Audio/AudioSource.h"
#include "../MyEngine/Graphics/Text.h"
//Serialize
#pragma warning(push, 0)
#include <rapidjson/document.h>     // for Document, Value, AllocatorType
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h> // for StringBuffer
#pragma warning(pop)


class Factory
{
public: 

	std::optional<Entity> CreateEntityFromJson(std::string const& path);
	// call onto default object function to get the basic SRT and render
	// then we check the file path from param and check for any components it wanna add 
	// call component deserialize functions with json filepath and entity as param

	// Creates entity with component SRT and RENDER. ITS ALL HARD CODED SO AS TO NOT BE CHANGE IT. ALL entity has these.
	Entity DefaultObj(); 

	// Delete the entity u want. I know that i can just use g_coordinator but i want the option in here so its neater and for ImGui to call this function instead
	void DeleteEntity(Entity entity);

	// Save this entity into the json file, might make a save then delete function if needed.
	bool SaveEntityToJson(Entity entity, std::filesystem::path const& path);

private:

	// calls onto prefab json file only, it will just load what the param PrefabName is.
	//bool LoadPrefab(Entity entity, std::string const& PrefabName);
	std::string PrefabPathway = "../../MyEngine/JSON/PrefabList.json";


	// name of prefab, reward is the container inside of this prefab. DO WE NEED THIS? 
	std::unordered_map<std::string, rapidjson::Document> map_of_prefabs;
};

// Prefab: something u want reuse for runtime(enemy, bullet), OH U SHOT A GUN, BULLET APPEARS, NOT PREFAB THEN modify
// default obj: all entity has SRT and render. anything else u add urself
// SCENE WILL JUST ONE SHOT RENDER ALL THE DIFF LOCATION AND STUFF. IT MUST ALL BE FILLED!!!!!
// For deserialize function, it will create the object, grab from json, then call g_coordinator->addcomponent(entity,obj), param is entity and json file
// save scene, change scene. ADD COMPONENT, REMOVE COMPONENT, ADD AND DELETE ENTITY