/******************************************************************************/
/**
 * @file       SceneManager.h
 * @project    Pulse Protocol
 * @author     Chia Wei Xuan Rachael - 100%
 * @brief      Scene management system that inherits from Systems base class.
 *             Provides interface for scene lifecycle operations, entity tracking,
 *             and JSON serialization of game scenes.
 *
 *             Public Interface:
 *             - Init(): Loads StartingScene (configured to Level1.json), sets initial
 *               CurrentScene and PreviousScene values
 *             - Update(float dt): Passive scene state tracking, monitors scene changes
 *             - LoadSceneFromJson(path): Parses JSON scene file, creates entities with
 *               components, auto-stops game during transition, registers GUI buttons
 *             - SaveCurrentScene(): Serializes current scene state to JSON file in
 *               ../../MyEngine/JSON/[SceneName].json format
 *             - UnloadCurrentScene(): Destroys all entities in SceneEntityList, clears
 *               tracking container
 * 
 *				Scene State Management:
 *             - GrabCurrentScene(): Returns active scene name
 *             - ChangeCurrentScene(str): Updates CurrentScene identifier
 * 
 *				Entity Tracking:
 *             - IncrementEntityArray(entity): Adds entity to SceneEntityList for
 *               ownership tracking and cleanup
 *             - DecrementEntityArray(entity): Removes specific entity from list using
 *               std::find + erase (removes ONLY first matching entity)
 * 
 * @copyright  Copyright (C) 2025 DigiPen Institute of Technology.
 *             Reproduction or disclosure of this file or its contents without the
 *             prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#pragma once
#include "pch/pch_temp.h"
//#include "../GameLogic/GameLogic.h"
//#include "../Graphics/Transform.h"
//#include "../Graphics/Animation.h"
//#include "../Graphics/Renderable.h"
////#include "../Components/AABB.h"
//#include "../CoreEngine/Assets.h"
//#include "../Components/Health.h"
//#include "../Components/EntityType.h"
//
//#include "../MyEngine/Components/DisplayName.h"
#include "../MyEngine/CoreEngine/ECS/Types.h"
#include "CoreEngine/ECS/Coordinator.h"


#pragma warning(push, 0)
#include <rapidjson/document.h>     
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h> 
#include <rapidjson/istreamwrapper.h>
#pragma warning(pop)

extern std::vector<Entity> g_entitiesToDelete;

class SceneManager: public Systems
{
public:
	SceneManager();

	void Init() override;              
	void Update(float dt) override;    
	//void Exit() override;

	void LoadSceneFromJson(std::string const& path);
	void SaveCurrentScene();
	void UnloadCurrentScene();

	std::string CurrentScene; // i need this public for me to call in other systems when i press the nbutton in hovered state
	
	std::string GrabCurrentScene() const { return CurrentScene; }
	std::string GrabLastScene() const { return LastScene; }
	void ClearLastScene() { LastScene = ""; }
	void ChangeCurrentScene(std::string const& str) { CurrentScene = str; }
	void IncrementEntityArray(Entity entity) { SceneEntityList.push_back(entity); }
	void DecrementEntityArray(Entity entity)
	{
		auto victim = std::find(SceneEntityList.begin(), SceneEntityList.end(), entity);
		if (victim != SceneEntityList.end())
		{
			SceneEntityList.erase(victim);  // Erases ONLY the first victim that matches and kill it >:D
		}
	}

private:
	std::string PreviousScene;		// For change detection in Update()
	std::string LastScene;			// For ESC navigation - the scene we came FROM
	std::vector<Entity> SceneEntityList;

	std::string StartingScene = "../../MyEngine/JSON/MainMenu.json"; 
	//std::string StartingScene = "../../MyEngine/JSON/Level1.json"; // level 1 for reg to test scripts 

	void Deserialize(const rapidjson::Value& in);
	void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const;
};
//ALL ENTITY IN THE ARRAY MUST BE EXACTLY LIKE A PREFAB. But it is not a prefab, just all its component. (for scene json)