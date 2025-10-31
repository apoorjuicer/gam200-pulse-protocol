/******************************************************************************/
/**
 * @file       SceneManager.h
 * @project    Pulse Protocol
 * @author     Chia Wei Xuan Rachael
 * @brief      Scene management system class that inherits from Systems base class.
 *             Defines Scene enum for game states and provides interface for
 *             scene loading, unloading, and transitioning. Maintains current
 *             scene state and tracks scene-specific entities for cleanup
 * @copyright  Copyright (C) 2025 DigiPen Institute of Technology.
 *             Reproduction or disclosure of this file or its contents without the
 *             prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#pragma once
#include "pch/pch_temp.h"
#include "../Components/Naming.h"
#include "../GameLogic/GameLogic.h"
#include "../Graphics/Transform.h"
#include "../Graphics/Animation.h"
#include "../Graphics/Renderable.h"

#include "../CoreEngine/ECS/Types.h"
#include "CoreEngine/ECS/Coordinator.h"
#include "CoreEngine/Factory/Factory.h"

#pragma warning(push, 0)
#include <rapidjson/document.h>     
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h> 
#include <rapidjson/istreamwrapper.h>
#pragma warning(pop)

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

	
	std::string GrabCurrentScene() const { return CurrentScene; }
	void ChangeCurrentScene(std::string const& str) { CurrentScene = str; }

private:
	std::string CurrentScene;
	std::string PreviousScene;
	std::vector<Entity> SceneEntityList;

	//std::string StartingScene = "../../MyEngine/JSON/MainMenu.json"; 
	std::string StartingScene = "../../MyEngine/JSON/Level1.json"; // level 1 for reg to test scripts 

	void Deserialize(const rapidjson::Value& in);
	void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const;
};
//ALL ENTITY IN THE ARRAY MUST BE EXACTLY LIKE A PREFAB. But it is not a prefab, just all its component. (for scene json)