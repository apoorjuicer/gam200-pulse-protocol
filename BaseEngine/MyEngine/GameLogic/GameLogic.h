/*****************************************************************************/
/**
 * @file        GameLogic.h
 * @project     Pulse Protocol
 * @author		Chia Wei Xuan Rachael (primary) - 90%
 * @author		Reginald Lew Yee Ren (secondary) - 10%
 * 
 * @brief       Defines the scripting system for game entities.
 *              Provides the interface `Iscript` and derived `PlayerScript` and
 *              `EnemyScript` classes to handle entity-specific logic (movement,
 *              collision mode toggles, and interactions).
 *              Managed by `LogicSystem` which handles script registration,
 *              updating, and serialization of script names to JSON.
 *
 * @copyright   Copyright (C) 2025
 *              DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without
 *              prior written consent is prohibited.
 ******************************************************************************/
#pragma once
#include "pch/pch_temp.h"
#include <GLFW/glfw3.h>
#include "../CoreEngine/ECS/Types.h"
#include "../CoreEngine/ECS/System.h"
#include "../CoreEngine/ECS/Coordinator.h"
//#include "../MyEngine/Components/ScriptName.h"
#include "../MyEngine/Serialization/Serialization.h"
#include "../Input/RhythmGameplayInput.h"

//components:
#include "../Graphics/Transform.h"
#include "./Components/DisplayName.h"
#include "../Graphics/Renderable.h"


//Serialize
#pragma warning(push, 0)
#include <rapidjson/document.h>     // for Document, Value, AllocatorType
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h> // for StringBuffer
#pragma warning(pop)

// base class for scripts
class Iscript
{
public:

	// ALL SCRIPTS MUST HAVE THESE 3 FUNCTIONS
	virtual void Init(Entity ID) = 0;
	virtual void Update(Entity ID) = 0;
	virtual void Exit(Entity ID) = 0;

	// if needed, for collision msg
	//virtual void OnCollisionEnter(Entity ID) {};
	//virtual void OnCollisionUpdate(Entity ID) {};
	//virtual void OnCollisionExit(Entity ID) {};


	//ser
	virtual void Serialize() {};


	// add clone function and = 0, so all kid will have to have it.
	virtual Iscript* Clone() = 0;
	
};

class PlayerScript : public Iscript
{

public:
	void Init(Entity ID) override;
	void Update(Entity ID) override;
	void Exit(Entity ID) override;

	void Serialize() override;

	//std::map<std::string, std::any(bool,float,int,std::string)>;
	// add in game variables here.
	int health = 0;

	PlayerScript* Clone() override;

private:
	bool  isAttacking = false;
	float attackTimer = 0.0f;
	/*float autoMoveTimer = 2.0f;*/ // FOR TESTING THE STOP BUTTON FUNCTIONALITY PURPOSE
};

//PlayerScript playerScript;
//playerScript.float_fields.emplace("Health", 100.0f);


class EnemyScript : public Iscript
{
public:
	void Init(Entity ID) override;
	void Update(Entity ID) override;
	void Exit(Entity ID) override;

	EnemyScript* Clone() override;

	// --- Toggles for testing modes ---
	static bool testAABBCollision;
	static bool testOBBCollision;

	float enemyDmg;
};

// Per-button data structure
struct ButtonData
{
	std::string buttonName;
	std::string normalTexture;
	std::string hoverTexture;
	bool isHovering = false;
};

class ButtonScript : public Iscript
{
public:
	void Init(Entity ID) override;
	void Update(Entity ID) override;
	void Exit(Entity ID) override;

private:
	// Store data per entity ID
	std::unordered_map<Entity, ButtonData> buttonDataMap;
};

class PulsingBorderScript : public Iscript
{
public:
	void Init(Entity ID) override;
	void Update(Entity ID) override;
	void Exit(Entity ID) override;

private:
	float pulseTimer = 0.0f;
	float bpm = 120.0f;
	float minAlpha = 0.2f;
	float maxAlpha = 1.0f;
};

class LogicSystem: public Systems
{
public:
	LogicSystem();  //constructor
	~LogicSystem(); //destructor

	void Init()           override;
	void Update(float dt) override;
	void Exit()           override;

	// Register script and update script function
	void AddScript(std::string const& ScriptName, std::shared_ptr<Iscript> script);

	// We give script name as param and get the update function of each script(child of Iscript)
	std::shared_ptr<Iscript> GrabScript(std::string const& ScriptName);

private:
	// key is the name of script which is a component, then reward is the class parent but can also grab kid functions and data
	std::unordered_map<std::string, std::shared_ptr<Iscript>> map_of_scripts; // template
	std::unordered_map<Entity, std::unordered_map<std::string, std::shared_ptr<Iscript>>> map_of_scriptInstances; // copying from template (wrap raw ptr into smart ptr(shared) to prevent mem leak ltr )
};

struct LogicComponent
{
    std::string ScriptName;


	// Find out how to make it a script instance.
	//std::unordered_map<std::string, std::unique_ptr<Iscript>> scriptInstances;

	void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const ;

	void Deserialize(const rapidjson::Value& in);
};

bool SaveScriptName(LogicComponent const& t, std::string const& path);
bool LoadScriptName(LogicComponent& t, std::string const& path);
