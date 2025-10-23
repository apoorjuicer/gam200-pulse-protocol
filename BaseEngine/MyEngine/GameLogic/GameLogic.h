#pragma once
#include "pch/pch.h"


class LogicSystem: public Systems
{
public:
	LogicSystem();  //constructor
	~LogicSystem(); //destructor

	void init();
	void update();
	void exit();

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
	virtual void init(Entity ID) = 0;
	virtual void update(Entity ID) = 0;
	virtual void exit(Entity ID) = 0;
};
