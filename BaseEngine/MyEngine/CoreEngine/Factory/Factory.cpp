#pragma once
#include "Factory.h"

using namespace rapidjson;

// change to bool, and add file path that will be given from either scene manager/ImGui
// Read from file, loop thru a check if have type and component. Add component base on type.  
bool Factory::CreateEntityFromJson(std::string const& path)
{
	auto* g_coordinator = Coordinator::GetInstance();

	// path iis the name of the prefab you want to create
	//std::string filePath = "../../MyEngine/JSON/test.json";//AssetManager.mPrefabMap[path];
	//std::string savefilePath = "../../MyEngine/JSON/test2.json";

	std::ifstream ifs(path);
	if (!ifs.is_open()) 
	{
		std::cerr << "Error: Cannot open file for reading: " << path << "\n";
		return false;
	}

	IStreamWrapper isw(ifs);
	Document d;
	d.ParseStream(isw);

	//check if Json file has component array
	if (!(d.HasMember("components") && d["components"].IsArray()))
	{
		std::cerr << "Components are missing/wrong!" << std::endl;
		return false;
	}
	
	Entity entity = DefaultObj();			   // create entity with transform and render component.
	const auto& components = d["components"];  // Get the components array
	LogicComponent l;
	l.ScriptName = "no scripts inside";
	g_coordinator->AddComponent(entity, l);
	
	// run thru all the components in the component array
	for (rapidjson::SizeType i = 0; i < components.Size(); ++i)
	{
		const auto& component = components[i];

		if (component.HasMember("transform") && component["transform"].IsObject()) { g_coordinator->GetComponent<Framework::Transform>(entity).Deserialize(component["transform"]); }
		if (component.HasMember("script") && component["script"].IsObject()) 
		{
			g_coordinator->GetComponent<LogicComponent>(entity).Deserialize(component["script"]); 
		}
	}

	return true;
}

// ONLY FOR ImGui
Entity Factory::DefaultObj()
{
	auto* g_coordinator = Coordinator::GetInstance();
	Entity entity = g_coordinator->CreateEntity();
	Framework::Transform t; 
	t.Scale = { 200.f,200.f };
	t.Rotation = {0,0};
	t.Pos = {0,0};

	/*
	Render r;
	r.angle_disp = 0.f;
	r.angle_speed = 0.f;
	r.mdl_ref = 0.f;
	r.shd_ref = 0.f;
	r.textureID = 0;
	r.spriteName;
	*/

	g_coordinator->AddComponent(entity, t);
	//g_coordinator->AddComponent(entity, r);
	return entity;
}

// Used in ImGui and for when smth dies. Making it a function so that ImGui can just grab factory related stuff
void Factory::DeleteEntity(Entity entity)
{
	auto* g_coordinator = Coordinator::GetInstance();
	g_coordinator->DestroyEntity(entity);
}

// check if entity has what components, then just serialize to Json file
bool Factory::SaveEntityToJson(Entity entity, std::string const& path)
{
	using namespace rapidjson;
	auto* g_coordinator = Coordinator::GetInstance();
	Document d;
	d.SetObject();
	auto& alloc = d.GetAllocator();

	// we want our components to be an array
	Value componentsArray(kArrayType);

	// check if entity have these compoennts, if have then serialize to json file
	if (g_coordinator->HasComponent<Framework::Transform>(entity))
	{ 
		Value ComponentObj(rapidjson::kObjectType); 
		Value TransformVal;

		auto& transform = g_coordinator->GetComponent<Framework::Transform>(entity);
		transform.Serialize(TransformVal, alloc);

		ComponentObj.AddMember("transform", TransformVal, alloc);	// so now it will be "transform":{}
		componentsArray.PushBack(ComponentObj, alloc);				// push the entire transform data into component array
	}


	// check if entity have these compoennts, if have then serialize to json file
	if (g_coordinator->HasComponent<LogicComponent>(entity))
	{
		Value ComponentObj(rapidjson::kObjectType);
		Value ScriptVal;

		auto& script = g_coordinator->GetComponent<LogicComponent>(entity);
		script.Serialize(ScriptVal, alloc);

		ComponentObj.AddMember("script", ScriptVal, alloc);	
		componentsArray.PushBack(ComponentObj, alloc);		
	}

	// ADD AS ONE BIG ARRAY WITH ALL THE COMPONENTS AVAILABLE ABOVE! >:]
	d.AddMember("components", componentsArray, alloc);

	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer); // make it nice
	d.Accept(writer);
	namespace fs = std::filesystem;
	fs::create_directories("../../MyEngine/JSON"); // make sure u save in Json and not build 

	std::ofstream ofs{ path };

	if (!ofs.is_open()) 
	{
		std::cerr << "Error: Cannot open file for writing: " << path << "\n";
		return false;
	}

	ofs << buffer.GetString();
	if (!ofs.good()) 
	{
		std::cerr << "Error: Failed to write JSON to file: " << path << "\n";
		return false;
	}
	return true;
}

