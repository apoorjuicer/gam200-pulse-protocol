#pragma once
#include "GameLogic.h"

LogicSystem::LogicSystem()
{

}

LogicSystem::~LogicSystem()
{

}

void LogicSystem::Init()
{
	//register scripts here. I think it it will be the Csharp / Lua game code here. Ask Eli if i did it right 
    // AddScript("PlayerScript", std::function<void(Entity)> function);
    
    AddScript("PlayerScript", std::make_shared<PlayerScript>());
    AddScript("EnemyScript", std::make_shared<EnemyScript>());
}

void LogicSystem::Update(float dt)
{
	(void)dt;
	auto* g_coordinator = Coordinator::GetInstance();

	// only update all entity that has scripts
	for (auto const& entity : EntityMember)
	{
		// get script name from each entity and feed them to its respective scripts
		auto& scriptname = g_coordinator->GetComponent<LogicComponent>(entity);

		// find reward (script update function) using script name
		auto func = map_of_scripts.find(scriptname.ScriptName);

		// if we can find it then good, we did it right XD. and now we call said function
		if (func != map_of_scripts.end())
		{
            func->second->Update(entity);
		}

		else
		{
            // THIS IS WHERE U LEFT OFF DUMBASS: CheeseBuger
			//std::cerr << "CAN'T FIND SCRIPTS UPDATE FUNCTION!" << std::endl;
		}
	}
}

void LogicSystem::Exit()
{
    auto* g_coordinator = Coordinator::GetInstance();

    for (auto const& entity : EntityMember)
    {
        auto& scriptname = g_coordinator->GetComponent<LogicComponent>(entity);
        auto func = map_of_scripts.find(scriptname.ScriptName);

        if (func != map_of_scripts.end())
        {
            func->second->Exit(entity);
        }
    }

    // Clear the script map
    map_of_scripts.clear();
}

// Might change std::function to a class of the init(), update(), and exit(). Smth like Iscript, can i sub in Iscript for std::function
void LogicSystem::AddScript(std::string const& ScriptName, std::shared_ptr<Iscript> script)
{
	// add this type of script to unlock this kind of update function. 
	// exp: name = monsterscript. Unlocks => monsters script class functions  
    map_of_scripts[ScriptName] = script;
}

std::shared_ptr<Iscript> LogicSystem::GrabScript(std::string const& ScriptName)
{
	// grab reward(script update function)
	auto func = map_of_scripts.find(ScriptName);
	if(func != map_of_scripts.end())
	{
		return func->second;
	}

	else
	{
		//std::cerr << "CAN'T FIND SCRIPTS UPDATE FUNCTION!" << std::endl;
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


// FOR SCRIPTS ONLY ONWARDS
void PlayerScript::Init(Entity ID) 
{
	(void)ID;
}

void PlayerScript::Update(Entity ID) 
{
	auto g_coordinator = Coordinator::GetInstance();
	auto& hero_transform = g_coordinator->GetComponent<Framework::Transform>(ID);
	if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_W) == GLFW_PRESS)
	{
		hero_transform.Pos.y += 5.0f;
	}

	if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_A) == GLFW_PRESS)
	{
		hero_transform.Pos.x -= 5.0f;
	}

	if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_S) == GLFW_PRESS )
	{
		hero_transform.Pos.y -= 5.0f;
	}

	if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_D) == GLFW_PRESS)
	{
		hero_transform.Pos.x += 5.0f;
	}

}

void PlayerScript::Exit(Entity ID) 
{
	(void)ID;
}

void EnemyScript::Init(Entity ID)
{
	(void)ID;
}

void EnemyScript::Update(Entity ID)
{
    auto g_coordinator = Coordinator::GetInstance();

    // --- Step 1: Check essential components ---
    if (!g_coordinator->HasComponent<Framework::Transform>(ID) ||
        !g_coordinator->HasComponent<AABB>(ID) ||
        !g_coordinator->HasComponent<Health>(ID))
        return;

    auto& enemy_transform = g_coordinator->GetComponent<Framework::Transform>(ID);
    auto& enemy_box = g_coordinator->GetComponent<AABB>(ID);
    auto& enemy_health = g_coordinator->GetComponent<Health>(ID);

    // --- Step 2: Skip update if enemy is already dead ---
    if (!enemy_health.isAlive)
        return;

    // --- Step 3: Wait for key 'M' before moving enemy ---
    static bool enemyShouldMove = false;

    if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_M) == GLFW_PRESS)
    {
        enemyShouldMove = true; // Start moving once M is pressed
    }

    if (enemyShouldMove)
    {
        enemy_transform.Pos.x -= 0.5f; // constant leftward speed
    }

    // --- Step 4: Find the player entity ---
    Entity playerEntity = static_cast<Entity>(-1);
    auto logicSys = g_coordinator->GetSystem<LogicSystem>();

    for (auto const& e : logicSys->EntityMember)
    {
        if (!g_coordinator->HasComponent<LogicComponent>(e))
            continue;

        auto& logic = g_coordinator->GetComponent<LogicComponent>(e);
        if (logic.ScriptName == "PlayerScript")
        {
            playerEntity = e;
            break;
        }
    }

    if (playerEntity == static_cast<Entity>(-1))
        return;

    // --- Step 5: Retrieve player components ---
    if (!g_coordinator->HasComponent<Framework::Transform>(playerEntity) ||
        !g_coordinator->HasComponent<AABB>(playerEntity) ||
        !g_coordinator->HasComponent<Health>(playerEntity))
        return;

    auto& player_transform = g_coordinator->GetComponent<Framework::Transform>(playerEntity);
    auto& player_box = g_coordinator->GetComponent<AABB>(playerEntity);
    auto& player_health = g_coordinator->GetComponent<Health>(playerEntity);

    // --- Step 6: Build both AABBs using CollisionManager ---
    CollisionManager collision;
    AABB enemy_hitbox = collision.BuildAABBFromTransform(enemy_transform, enemy_box);
    AABB player_hitbox = collision.BuildAABBFromTransform(player_transform, player_box);

    // --- Step 7: Size-based edge collision threshold ---
    float playerWidth = player_hitbox.width;
    float enemyWidth = enemy_hitbox.width;

    float worldScale = 4.5f; // adjust for your world scale
    float edgeThreshold = (playerWidth + enemyWidth) * worldScale;

    bool edgeContact =
        (player_hitbox.max.x >= enemy_hitbox.min.x - edgeThreshold) &&
        (player_hitbox.min.x < enemy_hitbox.min.x) &&
        (player_hitbox.max.y >= enemy_hitbox.min.y) &&
        (player_hitbox.min.y <= enemy_hitbox.max.y);

    // --- Debug output ---
    std::cout << std::fixed << std::setprecision(2)
        << "Enemy Box: [(" << enemy_hitbox.min.x << ", " << enemy_hitbox.min.y << ") -> ("
        << enemy_hitbox.max.x << ", " << enemy_hitbox.max.y << ")]\n"
        << "Player Box: [(" << player_hitbox.min.x << ", " << player_hitbox.min.y << ") -> ("
        << player_hitbox.max.x << ", " << player_hitbox.max.y << ")]\n"
        << "worldScale=" << worldScale
        << " edgeThreshold=" << edgeThreshold
        << " | EdgeContact=" << (edgeContact ? "TRUE" : "FALSE")
        << std::endl;

    // --- Step 8: Handle collision ---
    if (edgeContact && enemy_health.isAlive && player_health.isAlive)
    {
        enemy_health.isAlive = false;

        player_health.hp -= enemy_health.damage;
        if (player_health.hp <= 0)
        {
            player_health.hp = 0;
            player_health.isAlive = false;
            std::cout << ">>> PLAYER DEFEATED\n";
        }

        std::cout << ">>> COLLISION (edge contact)! Player HP: "
            << player_health.hp << " | Enemy defeated!\n";

        // Move enemy off-screen
        enemy_transform.Pos.x = -9999.0f;
        enemy_transform.Pos.y = -9999.0f;
    }
}

void EnemyScript::Exit(Entity ID)
{
	(void)ID;
}

