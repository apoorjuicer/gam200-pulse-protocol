/*****************************************************************************/
/**
 * @file        GameLogic.h
 * @project     Pulse Protocol
 * @author		Chia Wei Xuan Rachael (primary) - 90%
 * @author		Reginald Lew Yee Ren (secondary) - 5%
 * @author		Goh Pin Kai (secondary) - 5%
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

extern bool g_countdownFinished;

// base class for scripts
class Iscript
{
public:

	// ALL SCRIPTS MUST HAVE THESE 3 FUNCTIONS
	virtual void Init(Entity ID) = 0;
	virtual void Update(Entity ID) = 0;
	virtual void Exit(Entity ID) = 0;
};

class PlayerScript : public Iscript
{
public:
	void Init(Entity ID) override;
	void Update(Entity ID) override;
	void Exit(Entity ID) override;

private:
	bool  isAttacking = false;
	float attackTimer = 0.0f;
	/*float autoMoveTimer = 2.0f;*/ // FOR TESTING THE STOP BUTTON FUNCTIONALITY PURPOSE
};

class EnemyScript : public Iscript
{
public:
	void Init(Entity ID) override;
	void Update(Entity ID) override;
	void Exit(Entity ID) override;

	// --- Toggles for testing modes ---
	static bool testAABBCollision;
	static bool testOBBCollision;
};

//class EnemySpawnerScript : public Iscript
//{
//public:
//	void Init(Entity ID) override;
//	void Update(Entity ID) override;
//	void Exit(Entity ID) override;
//
//	// Track the two enemies
//	static Entity Enemy1ID;
//	static Entity Enemy2ID;
//
//	// True when Enemy 1 has died
//	static bool Enemy1Dead;
//
//	// Per-enemy hop timer (moved here)
//	static std::unordered_map<Entity, float> HopTimer;
//};

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

class CountdownScript : public Iscript
{
public:
	void Init(Entity ID) override;
	void Update(Entity ID) override;
	void Exit(Entity ID) override;

private:
	float countdownTimer = 0.0f;
};

class FeedbackScript : public Iscript
{
public:
	void Init(Entity ID) override;
	void Update(Entity ID) override;
	void Exit(Entity ID) override;

private:
	float displayTimer = 0.0f;
	bool isShowing = false;
	float startTime = 0.0f;
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
	std::unordered_map<std::string, std::shared_ptr<Iscript>> map_of_scripts;
};

struct LogicComponent
{
    std::string ScriptName;

    void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const ;

	void Deserialize(const rapidjson::Value& in);
};

bool SaveScriptName(LogicComponent const& t, std::string const& path);
bool LoadScriptName(LogicComponent& t, std::string const& path);
