#pragma once
#include "pch/pch_temp.h"
#include "../CoreEngine/ECS/Types.h"
#include "../CoreEngine/ECS/System.h"
#include "../CoreEngine/ECS/Coordinator.h"
//#include "../MyEngine/Components/ScriptName.h"

//components:
#include "../Graphics/Transform.h"
#include "../GameLogic/GameLogic.h"

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
};

// ALL THIS IS JUST TEMP, LATER ON WE WILL SHIFT TO LUA SCRIPTING INSTEAD. ALL GAMELOGIC SYSTEM DOES IS 
// TO UPDATE ALL THE ENTITIES INIT, UPDATE, EXIT SCRIPTS
class PlayerScript : public Iscript
{
	virtual void Init(Entity ID) override
	{
		(void)ID;
	}

	virtual void Update(Entity ID) override
	{
		//std::cout << ID << std::endl;

		//if (keypress)
		//{
		//	get transform
		//		move entity

		//}
		(void)ID;
	}

	virtual void Exit(Entity ID) override
	{
		(void)ID;
	}
};

class EnemyScript : public Iscript
{
	virtual void Init(Entity ID) override
	{
		(void)ID;
	}

	virtual void Update(Entity ID) override
	{
		(void)ID;
	}

	virtual void Exit(Entity ID) override
	{
		(void)ID;
	}
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
