#pragma once
#include "pch/pch_temp.h"
#include <GLFW/glfw3.h>
#include "../CoreEngine/ECS/Types.h"
#include "../CoreEngine/ECS/System.h"
#include "../CoreEngine/ECS/Coordinator.h"
//#include "../MyEngine/Components/ScriptName.h"
#include "../MyEngine/Serialization/Serialization.h"

//components:
#include "../Graphics/Transform.h"


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

class PlayerScript : public Iscript
{
public:
	void Init(Entity ID) override;
	void Update(Entity ID) override;
	void Exit(Entity ID) override;

private:
};

class EnemyScript : public Iscript
{
public:
	void Init(Entity ID) override;
	void Update(Entity ID) override;
	void Exit(Entity ID) override;
};

// ALL THIS IS JUST TEMP, LATER ON WE WILL SHIFT TO LUA SCRIPTING INSTEAD. ALL GAMELOGIC SYSTEM DOES IS 
// TO UPDATE ALL THE ENTITIES INIT, UPDATE, EXIT SCRIPTS
//class PlayerScript : public Iscript
//{
//public:
//	virtual void Init(Entity ID) override
//	{
//		(void)ID;
//	}
//
//	virtual void Update(Entity ID) override
//	{
//	    auto g_coordinator = Coordinator::GetInstance();
//		auto& hero_transform = g_coordinator->GetComponent<Framework::Transform>(ID);
//		if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_W) == GLFW_PRESS && !wPressed)
//		{
//			hero_transform.Pos.y += 5.0f;
//			wPressed = true;
//		}
//		else if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_W) == GLFW_RELEASE){wPressed = false;}
//
//		if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_A) == GLFW_PRESS && !aPressed)
//		{
//			hero_transform.Pos.x -= 5.0f;
//			aPressed = true;
//		}
//		else if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_A) == GLFW_RELEASE) { aPressed = false; }
//
//		if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_S) == GLFW_PRESS && !sPressed)
//		{
//			hero_transform.Pos.y -= 5.0f;
//			sPressed = true;
//		}
//		else if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_S) == GLFW_RELEASE) { sPressed = false; }
//
//		else if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_D) == GLFW_PRESS && !dPressed)
//		{
//			hero_transform.Pos.x += 5.0f;
//			dPressed = true;
//		}
//		else if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_D) == GLFW_RELEASE) { dPressed = false; }
//
//
//		//(void)ID;
//	}
//
//	virtual void Exit(Entity ID) override
//	{
//		(void)ID;
//	}
//
//private:
//	 bool wPressed = false, aPressed = false, sPressed = false, dPressed = false;
//};
//
//class EnemyScript : public Iscript
//{
//public:
//	virtual void Init(Entity ID) override
//	{
//		(void)ID;
//	}
//
//	virtual void Update(Entity ID) override
//	{
//		(void)ID;
//	}
//
//	virtual void Exit(Entity ID) override
//	{
//		(void)ID;
//	}
//
//private:
//};


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
