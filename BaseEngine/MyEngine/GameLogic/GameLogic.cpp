#pragma once
#include "GameLogic.h"
#include "pch/pch.h"

LogicSystem::LogicSystem()
{

}

LogicSystem::~LogicSystem()
{

}

void LogicSystem::init()
{
	//register scripts here. I think it it will be the Csharp / Lua game code here. Ask Eli if i did it right 
}

void LogicSystem::update()
{
	auto g_coordinator = Coordinator::GetInstance();

	// only update all entity that has scripts
	for (auto const& entity : EntityMember)
	{
		// get script name from each entity and feed them to its respective scripts
		auto& scriptname = g_coordinator->GetComponent<LogicComponent>(entity);

		// find reward (script update function) using script name
		auto function = map_of_scripts.find(scriptname.ScriptName);

		// if we can find it then good, we did it right XD. and now we call said function
		if (function != map_of_scripts.end())
		{
			function->second(entity);
		}

		else
		{
			std::cerr << "CAN'T FIND SCRIPTS UPDATE FUNCTION!" << std::endl;
		}
	}
}

void LogicSystem::exit()
{
	// clear it just incase of mem leak. TBH im not sure if it will happen but just to be safe :p
	map_of_scripts.clear();
}

// Might change std::function to a class of the init(), update(), and exit(). Smth like Iscript, can i sub in Iscript for std::function
void LogicSystem::AddScript(std::string const& ScriptName, std::function<void(Entity)> function)
{
	// add this type of script to unlock this kind of update function. 
	// exp: name = monsterscript. Unlocks => monsters script update function  
	map_of_scripts[ScriptName] = function;
}

std::function<void(Entity)> LogicSystem::GrabScriptUpdate(std::string const& ScriptName)
{
	// grab reward(script update function)
	auto function = map_of_scripts.find(ScriptName);
	if(function != map_of_scripts.end())
	{
		return function->second;
	}

	else
	{
		std::cerr << "CAN'T FIND SCRIPTS UPDATE FUNCTION!" << std::endl;
		return nullptr;
	}
}

void LogicComponent::Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const
{
    //using namespace rapidjson;
    out.SetObject();
    //out["ScriptName"] = ScriptName;
    out.AddMember("ScriptName", rapidjson::Value(ScriptName.c_str(), alloc), alloc);
    //out.AddMember("scriptname", ScriptName, alloc);
}

void LogicComponent::Deserialize(const rapidjson::Value& in)
{
    if (!in.IsObject()) throw std::runtime_error("Invalid JSON for LogicComponent");
    if (in.HasMember("ScriptName")) ScriptName = in["ScriptName"].GetString(); // already assigns it

    //return *this;
}

bool SaveScriptName(LogicComponent const& t, std::string const& path)
{
    rapidjson::Document d;
    d.SetObject();
    auto& alloc = d.GetAllocator();

    rapidjson::Value ScriptComponents;
    t.Serialize(ScriptComponents, alloc);
   // d.AddMember("ScriptName", ScriptComponents, alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);

    // MUST HAVE TO RETURN BACK TO MYENGINE INSTEAD OF BUILD(AKA GETS DELETE WHEN CLEANED)
    namespace fs = std::filesystem;
    fs::create_directories("../../MyEngine/JSON");

    std::ofstream ofs(path);
    if (!ofs.is_open()) {
        std::cerr << "Error: Cannot open file for writing: " << path << "\n";
        return false;
    }
    ofs << buffer.GetString();
    if (!ofs.good()) {
        std::cerr << "Error: Failed to write JSON to file: " << path << "\n";
        return false;
    }
    return true;
}

bool LoadScriptName(LogicComponent& t, std::string const& path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open())
    {
        std::cerr << "Error: Cannot open file for reading: " << path << "\n";
        return false;
    }
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document d;
    d.ParseStream(isw);

    if (d.HasParseError()) {
        std::cerr << "Error: Failed to parse JSON in file: " << path << "\n";
        return false;
    }

    if (!d.HasMember("ScriptName")) {
        std::cerr << "Error: No 'transform' field in file: " << path << "\n";
        return false;
    }

    t.Deserialize(d["ScriptName"]);
    return true;
}