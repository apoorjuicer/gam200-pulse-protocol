#pragma once
#include "pch/pch_temp.h"
#include "../CoreEngine/ECS/Types.h"
#include "../CoreEngine/ECS/System.h"
#include "../CoreEngine/ECS/Coordinator.h"
//#include "../MyEngine/Components/ScriptName.h"

//Serialize
#pragma warning(push, 0)
#include <rapidjson/document.h>     // for Document, Value, AllocatorType
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h> // for StringBuffer
#pragma warning(pop)

class LogicSystem: public Systems
{
public:
	LogicSystem();  //constructor
	~LogicSystem(); //destructor

	void Init()           override;
	void Update(float dt) override;
	void Exit()           override;

	// Register script and update script function
	void AddScript(std::string const& ScriptName, std::function<void(Entity)> function);

	// We give script name as param and get the update function of each script(child of Iscript)
	std::function<void(Entity)> GrabScriptUpdate(std::string const& ScriptName);

private:
	// key is the name of script which is a component, then reward is update function of each script(child of Iscript)
	std::unordered_map<std::string, std::function<void(Entity)>> map_of_scripts;

};

struct LogicComponent
{
    std::string ScriptName;

    void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const ;

	void Deserialize(const rapidjson::Value& in);
};

bool SaveScriptName(LogicComponent const& t, std::string const& path);
bool LoadScriptName(LogicComponent& t, std::string const& path);

// base class for scripts
class Iscript
{
public:
	// ALL SCRIPTS MUST HAVE THESE 3 FUNCTIONS
	virtual void Init(Entity ID) = 0;
	virtual void Update(Entity ID) = 0;
	virtual void Exit(Entity ID) = 0;
};
