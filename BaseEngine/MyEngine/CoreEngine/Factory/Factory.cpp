#pragma once
#include "Factory.h"
#include "../CoreEngine/Assets.h"


using namespace rapidjson;

// change to optional for bool(if loaded succeessfully) and entity ID(for chloe)
// added file path that will be given from either scene manager/ImGui
// Read from file, loop thru a check if have type and component. Add component base on type.  
std::optional<Entity> Factory::CreateEntityFromJson(std::string const& path)
{
	auto* g_coordinator = Coordinator::GetInstance();

	// path iis the name of the prefab you want to create
	//std::string filePath = "../../MyEngine/JSON/test.json";//AssetsManager.mPrefabMap[path];
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
	
	Entity entity = g_coordinator->CreateEntity();		
	const auto& components = d["components"];  // Get the components array
	
	// run thru all the components in the component array
	for (rapidjson::SizeType i = 0; i < components.Size(); ++i)
	{
		const auto& component = components[i];
		if (component.HasMember("Typing") && component["Typing"].IsObject())
		{
			NamingComponent n;
			n.Deserialize(component["Typing"]);
			g_coordinator->AddComponent<NamingComponent>(entity, n);
		}

		if (component.HasMember("transform") && component["transform"].IsObject())
		{
			Framework::Transform t;
			t.Deserialize(component["transform"]);
			g_coordinator->AddComponent<Framework::Transform>(entity, t);
		}

		if (component.HasMember("render") && component["render"].IsObject())
		{
			Framework::Renderable r;
			r.Deserialize(component["render"]);
			g_coordinator->AddComponent<Framework::Renderable>(entity, r);
		}

		if (component.HasMember("animation") && component["animation"].IsObject())
		{
			Framework::Animation a;
			a.Deserialize(component["animation"]);
			g_coordinator->AddComponent<Framework::Animation>(entity, a);
		}

		if (component.HasMember("script") && component["script"].IsObject())
		{
			LogicComponent l;
			l.Deserialize(component["script"]);
			g_coordinator->AddComponent<LogicComponent>(entity, l);
		}

		if (component.HasMember("name") && component["name"].IsObject())
		{
			Name n;
			n.Deserialize(component["name"]);
			g_coordinator->AddComponent<Name>(entity, n);
		}

		if (component.HasMember("Health") && component["Health"].IsObject())
		{
			Health h;
			h.Deserialize(component["Health"]);
			g_coordinator->AddComponent<Health>(entity, h);
		}

		if (component.HasMember("AABB") && component["AABB"].IsObject())
		{
			AABB aabb;
			aabb.Deserialize(component["AABB"]);
			g_coordinator->AddComponent<AABB>(entity, aabb);
		}

		
	}
	// add entity to list of entity in the current scene
	auto sceneManager = g_coordinator->GetSystem<SceneManager>();
	sceneManager->IncrementEntityArray(entity);
	if (!g_coordinator->HasComponent<LayerTag>(entity)) {
		g_coordinator->AddComponent(entity, LayerTag{ LAYER_WORLD, 0, 0.0f });
	}
	return entity;
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

	Framework::Renderable r;

	Framework::Animation a;

	g_coordinator->AddComponent(entity, t);
	g_coordinator->AddComponent(entity, r);
	g_coordinator->AddComponent(entity, a);
	g_coordinator->AddComponent(entity, LayerTag{ LAYER_WORLD, 0, 0.0f });
	
	// add entity to list of entity in the current scene
	auto sceneManager = g_coordinator->GetSystem<SceneManager>();
	sceneManager->IncrementEntityArray(entity);
	return entity;
}

// Used in ImGui and for when smth dies. Making it a function so that ImGui can just grab factory related stuff
void Factory::DeleteEntity(Entity entity)
{
	auto* g_coordinator = Coordinator::GetInstance();
	auto sceneManager = g_coordinator->GetSystem<SceneManager>();
	sceneManager->DecrementEntityArray(entity); // KILL THE ENTITY FROM SCENE ENTITY CONTAINER >:D

	g_coordinator->DestroyEntity(entity);
}

// check if entity has what components, then just serialize to Json file
bool Factory::SaveEntityToJson(Entity entity, std::filesystem::path const& path)
{
	auto* g_coordinator = Coordinator::GetInstance();
	using namespace rapidjson;

	Document d;
	d.SetObject();
	auto& alloc = d.GetAllocator();

	// we want our components to be an array
	Value componentsArray(kArrayType);

	if (g_coordinator->HasComponent<NamingComponent>(entity))
	{
		Value ComponentObj(rapidjson::kObjectType);
		Value TypeVal;

		auto& objtype = g_coordinator->GetComponent<NamingComponent>(entity);
		objtype.Serialize(TypeVal, alloc);

		ComponentObj.AddMember("Typing", TypeVal, alloc);	
		componentsArray.PushBack(ComponentObj, alloc);	
	}

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
	if (g_coordinator->HasComponent<Framework::Renderable>(entity))
	{
		Value ComponentObj(rapidjson::kObjectType);
		Value RenderVal;

		auto& render = g_coordinator->GetComponent<Framework::Renderable>(entity);
		render.Serialize(RenderVal, alloc);

		ComponentObj.AddMember("render", RenderVal, alloc);	
		componentsArray.PushBack(ComponentObj, alloc);		
	}
	
	if (g_coordinator->HasComponent<Framework::Animation>(entity))
	{
		Value ComponentObj(rapidjson::kObjectType);
		Value AnimationVal;

		auto& animation = g_coordinator->GetComponent<Framework::Animation>(entity);
		animation.Serialize(AnimationVal, alloc);

		ComponentObj.AddMember("animation", AnimationVal, alloc);
		componentsArray.PushBack(ComponentObj, alloc);
	}

	if (g_coordinator->HasComponent<LogicComponent>(entity))
	{
		Value ComponentObj(rapidjson::kObjectType);
		Value ScriptVal;

		auto& script = g_coordinator->GetComponent<LogicComponent>(entity);
		script.Serialize(ScriptVal, alloc);

		ComponentObj.AddMember("script", ScriptVal, alloc);
		componentsArray.PushBack(ComponentObj, alloc);
	}

	if (g_coordinator->HasComponent<Name>(entity))
	{
		Value ComponentObj(rapidjson::kObjectType);
		Value NameVal;

		auto& name = g_coordinator->GetComponent<Name>(entity);
		name.Serialize(NameVal, alloc);

		ComponentObj.AddMember("name", NameVal, alloc);
		componentsArray.PushBack(ComponentObj, alloc);
	}

	if (g_coordinator->HasComponent<Health>(entity))
	{
		Value ComponentObj(rapidjson::kObjectType);
		Value HealthVal;

		auto& health = g_coordinator->GetComponent<Health>(entity);
		health.Serialize(HealthVal, alloc);

		ComponentObj.AddMember("Health", HealthVal, alloc);
		componentsArray.PushBack(ComponentObj, alloc);
	}

	if (g_coordinator->HasComponent<AABB>(entity))
	{
		Value ComponentObj(rapidjson::kObjectType);
		Value AABBVal;

		auto& aabb = g_coordinator->GetComponent<AABB>(entity);
		aabb.Serialize(AABBVal, alloc);

		ComponentObj.AddMember("AABB", AABBVal, alloc);
		componentsArray.PushBack(ComponentObj, alloc);
	}


	// ADD AS ONE BIG ARRAY WITH ALL THE COMPONENTS AVAILABLE ABOVE! >:]
	d.AddMember("components", componentsArray, alloc);

	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer); // make it nice
	d.Accept(writer);
	namespace fs = std::filesystem;
	fs::create_directories(path.parent_path()); // make sure u save in Json and not build 

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

