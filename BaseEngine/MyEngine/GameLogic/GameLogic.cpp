#pragma once
#include "GameLogic.h"

bool EnemyScript::testAABBCollision = false;
bool EnemyScript::testOBBCollision = false;

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

    // ==============================================================
    // --- Rotation toggle control (N key) ---
    // ==============================================================
    static bool isRotating = false;      // current rotation state
    static bool wasBPressed = false;     // key press debounce

    if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_N) == GLFW_PRESS)
    {
        if (!wasBPressed)
        {
            isRotating = !isRotating;   // toggle on/off
            wasBPressed = true;
            std::cout << (isRotating ? "[PLAYER] Rotation enabled.\n"
                : "[PLAYER] Rotation disabled.\n");
        }
    }
    else
    {
        wasBPressed = false;            // reset when key released
    }

    // ==============================================================
    // --- Apply rotation if toggled on ---
    // ==============================================================
    if (isRotating)
    {
        static double lastTime = glfwGetTime();
        double currentTime = glfwGetTime();
        float dt = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        float rotationSpeed = 180.0f; // degrees per second
        hero_transform.Rotation.x += rotationSpeed * dt;

        if (hero_transform.Rotation.x >= 360.0f)
            hero_transform.Rotation.x -= 360.0f;

        std::cout << "[PLAYER ROTATE] Rotation.x = "
            << hero_transform.Rotation.x << " degrees\n";
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
    if (!g_coordinator->HasComponent<Framework::Transform>(ID) ||
        !g_coordinator->HasComponent<AABB>(ID) ||
        !g_coordinator->HasComponent<Health>(ID))
        return;

    auto& enemy_transform = g_coordinator->GetComponent<Framework::Transform>(ID);
    auto& enemy_box = g_coordinator->GetComponent<AABB>(ID);
    auto& enemy_health = g_coordinator->GetComponent<Health>(ID);

    if (!enemy_health.isAlive)
        return;

    // --- Mode toggles ---
    static bool aabbMode = false;
    static bool obbMode = false;
    static bool wasMPressed = false;
    static bool wasNPressed = false;

    // --- Handle M toggle (AABB) ---
    if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_M) == GLFW_PRESS)
    {
        if (!wasMPressed)
        {
            aabbMode = !aabbMode;
            obbMode = false; // disable OBB when AABB is on
            wasMPressed = true;
            std::cout << (aabbMode ? "[ENEMY] AABB Mode ON\n" : "[ENEMY] AABB Mode OFF\n");
        }
    }
    else
        wasMPressed = false;

    // --- Handle N toggle (OBB) ---
    if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_N) == GLFW_PRESS)
    {
        if (!wasNPressed)
        {
            obbMode = !obbMode;
            aabbMode = false; // disable AABB when OBB is on
            wasNPressed = true;
            std::cout << (obbMode ? "[ENEMY] OBB Mode ON\n" : "[ENEMY] OBB Mode OFF\n");
        }
    }
    else
        wasNPressed = false;

    // --- Find Player Entity ---
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

    auto& player_transform = g_coordinator->GetComponent<Framework::Transform>(playerEntity);
    auto& player_box = g_coordinator->GetComponent<AABB>(playerEntity);
    auto& player_health = g_coordinator->GetComponent<Health>(playerEntity);

    CollisionManager collision;

    // ====================================================================
    // === MODE 1: AABB SLIDE & EDGE COLLISION (M) ========================
    // ====================================================================
    if (aabbMode)
    {
        enemy_transform.Pos.x -= 0.5f; // move left at constant rate

        AABB enemyAABB = collision.BuildAABBFromTransform(enemy_transform, enemy_box);
        AABB playerAABB = collision.BuildAABBFromTransform(player_transform, player_box);

        float worldScale = 4.5f;
        float edgeThreshold = (playerAABB.width + enemyAABB.width) * worldScale;

        bool edgeContact =
            (playerAABB.max.x >= enemyAABB.min.x - edgeThreshold) &&
            (playerAABB.min.x < enemyAABB.min.x) &&
            (playerAABB.max.y >= enemyAABB.min.y) &&
            (playerAABB.min.y <= enemyAABB.max.y);

        if (edgeContact && enemy_health.isAlive && player_health.isAlive)
        {
            enemy_health.isAlive = false;
            player_health.hp -= enemy_health.damage;

            std::cout << ">>> COLLISION (AABB edge)! Player HP: "
                << player_health.hp << " | Enemy defeated!\n";

            if (player_health.hp <= 0)
            {
                player_health.hp = 0;
                player_health.isAlive = false;
                std::cout << ">>> PLAYER DEFEATED\n";
            }

            enemy_transform.Pos = { -9999.0f, -9999.0f };
        }
    }

    // ====================================================================
    // === MODE 2: OBB ROTATION CHASE & EDGE COLLISION (N) ===============
    // ====================================================================
    if (obbMode)
    {
        static double last = glfwGetTime();
        double now = glfwGetTime();
        float dt = static_cast<float>(now - last);
        last = now;

        // Spin enemy clockwise
        enemy_transform.Rotation.x += 180.0f * dt;
        if (enemy_transform.Rotation.x >= 360.0f)
            enemy_transform.Rotation.x -= 360.0f;

        // Move toward player
        Vector2 dir = player_transform.Pos - enemy_transform.Pos;
        float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.001f)
        {
            dir.x /= len;
            dir.y /= len;
            enemy_transform.Pos.x += dir.x * 100.0f * dt;
            enemy_transform.Pos.y += dir.y * 100.0f * dt;
        }

        // --- Build OBBs & Check ---
        OBB enemyOBB = collision.BuildOBBFromTransform(enemy_transform, enemy_box);
        OBB playerOBB = collision.BuildOBBFromTransform(player_transform, player_box);

        bool obbHit = collision.CheckOBBvsOBB(enemyOBB, playerOBB);

        if (obbHit && enemy_health.isAlive && player_health.isAlive)
        {
            enemy_health.isAlive = false;
            player_health.hp -= enemy_health.damage;

            std::cout << ">>> EDGE COLLISION DETECTED via OBB <<<\n";
            std::cout << "Player HP: " << player_health.hp << "\n";

            if (player_health.hp <= 0)
            {
                player_health.hp = 0;
                player_health.isAlive = false;
                std::cout << ">>> PLAYER DEFEATED\n";
            }

            enemy_transform.Pos = { -9999.0f, -9999.0f };
        }
    }
}

void EnemyScript::Exit(Entity ID)
{
	(void)ID;
}

