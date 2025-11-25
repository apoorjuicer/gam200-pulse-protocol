/******************************************************************************/
/**
 * @file        GameLogic.cpp
 * @project     Pulse Protocol
 * @author		Chia Wei Xuan Rachael (primary) - 60%
 * @author		Reginald Lew Yee Ren (secondary) - 20%
 * @author		Chloe Lau Rey En (secondary) - 20%
 *
 * @brief		Core Components:
 *              - LogicSystem: Manages script registration, lifecycle (Init/Update/Exit),
 *                and execution for all entities with LogicComponent. Only updates during
 *                gameplay (respects Engine->IsPlaying() state).
 *              - Iscript Interface: Base class for all game scripts with Init, Update, Exit.
 *              - LogicComponent: Stores script name per entity, supports JSON serialization.
 *
 *
 *              Implements game entity logic for both Player and Enemy.
 *              Handles entity movement, rotation toggling, and two collision test modes:
 *              AABB (Axis-Aligned Bounding Box) via 'M' key and
 *              OBB (Oriented Bounding Box) via 'N' key.
 *              Includes the LogicSystem that registers, updates, and executes scripts.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#pragma once
#include "GameLogic.h"
#include "../MyEngine/CoreEngine/Factory/Factory.h"

bool EnemyScript::testAABBCollision = false;
bool EnemyScript::testOBBCollision = false;

// CRITICAL: Shared static variable for rhythm state reset
// This is accessed by both Init() and Update()
namespace {
    bool g_enemyRhythmNeedsReset = true;
}

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
    AddScript("ButtonScript", std::make_shared<ButtonScript>());
    AddScript("PulsingBorderScript", std::make_shared<PulsingBorderScript>());

    //std::cout << "=== REGISTERED SCRIPTS ===" << std::endl;
    //for (const auto& [name, script] : map_of_scripts) {
    //    std::cout << "  - " << name << std::endl;
    //}
    //std::cout << "=========================" << std::endl;
}

void LogicSystem::Update(float dt)
{
    (void)dt;

    // don't update scripts, if not playing the game
    if (!Engine->IsPlaying()) {
        return;
    }

    auto* g_coordinator = Coordinator::GetInstance();

    // make copy first
    auto entities = EntityMember;

    // only update all entity that has scripts
    for (auto const& entity : entities)
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
    if (func != map_of_scripts.end())
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


// ---------------------------------------------------------------------
// FOR SCRIPTS ONLY ONWARDS
// ---------------------------------------------------------------------

// =====================================================================
// PLAYER SCRIPT
// =====================================================================
void PlayerScript::Init(Entity ID)
{
    //autoMoveTimer = 2.0f; // reset timer
    auto* coord = Coordinator::GetInstance();
    auto  rhythm = coord->GetSystem<RhythmGameplayInput>();

    if (!rhythm) return;

    rhythm->registerInputListener("PlayerCombo",
        [this, ID](const InputEvent& evt)
        {
            if (evt.type != InputEvent::Type::COMBO_EXECUTED)
                return;

            if (evt.comboName != "Basic Strike")
                return;

            auto* c = Coordinator::GetInstance();

            if (!c->HasComponent<Framework::Renderable>(ID) ||
                !c->HasComponent<Framework::Animation>(ID))
                return;

            auto& rend = c->GetComponent<Framework::Renderable>(ID);
            auto& anim = c->GetComponent<Framework::Animation>(ID);

            // --- SWITCH TO ATTACK SPRITE ---
            rend.spriteName = "../../MyEngine/Assets/Textures/Characters/hero_spritesheet.png";
            rend.needsTextureReload = true;

            anim.totalFrames = 3;
            anim.columns = 3;
            anim.rows = 1;
            anim.animationSpeed = 240.0f;
            anim.currentFrame = 0;
            anim.elapsedTime = 0.0f;

            // enable attack timer (use PlayerScript's members)
            isAttacking = true;
            attackTimer = 0.5f;
        });
}

void PlayerScript::Update(Entity ID)
{
    auto g_coordinator = Coordinator::GetInstance();
    auto& hero_transform = g_coordinator->GetComponent<Framework::Transform>(ID);

    static double lastTime = glfwGetTime();
    double currentTime = glfwGetTime();
    float dt = static_cast<float>(currentTime - lastTime);
    lastTime = currentTime;

    // switch back to idle animation
    if (isAttacking)
    {
        attackTimer -= dt;
        if (attackTimer <= 0.0f)
        {
            isAttacking = false;

            if (g_coordinator->HasComponent<Framework::Renderable>(ID) &&
                g_coordinator->HasComponent<Framework::Animation>(ID))
            {
                auto& rend = g_coordinator->GetComponent<Framework::Renderable>(ID);
                auto& anim = g_coordinator->GetComponent<Framework::Animation>(ID);

                // idle sheet (your 2-frame image)
                rend.spriteName = "../../MyEngine/Assets/Textures/Characters/hero_idle.png";
                rend.needsTextureReload = true;

                anim.totalFrames = 2;
                anim.columns = 2;
                anim.rows = 1;
                anim.animationSpeed = 60.0f;
                anim.currentFrame = 0;
                anim.elapsedTime = 0.0f;
            }
        }
    }

    // auto-moves right after game starts (for 2 secs)
    /*if (autoMoveTimer > 0.0f) {
        hero_transform.Pos.x += 3.0f;
        autoMoveTimer -= 0.016f;
    }*/

    float moveSpeed = 300.0f;

    // this is just regular WASD
    if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_T) == GLFW_PRESS)
    {
        hero_transform.Pos.y += moveSpeed * dt;
    }

    if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_F) == GLFW_PRESS)
    {
        hero_transform.Pos.x -= moveSpeed * dt;
    }

    if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_G) == GLFW_PRESS)
    {
        hero_transform.Pos.y -= moveSpeed * dt;
    }

    if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_H) == GLFW_PRESS)
    {
        hero_transform.Pos.x += moveSpeed * dt;
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

// =====================================================================
// ENEMY SCRIPT
// =====================================================================

void EnemyScript::Init(Entity ID)
{
    std::cout << "[EnemyScript] Init() called for entity " << ID << " - marking for rhythm reset\n";

    g_enemyRhythmNeedsReset = true;
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

    // ====== Time (for time-based movement) ======
    float dt = static_cast<float>(GLHelper::delta_time);

    if (!enemy_health.isAlive)
        return;

    // ====== Manual arrow-key movement (kept for debug) ======
    float enemyMoveSpeed = 300.0f;

    if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_UP) == GLFW_PRESS)
    {
        enemy_transform.Pos.y += enemyMoveSpeed * dt;
    }

    if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        enemy_transform.Pos.x -= enemyMoveSpeed * dt;
    }

    if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        enemy_transform.Pos.y -= enemyMoveSpeed * dt;
    }

    if (glfwGetKey(Engine->GetWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        enemy_transform.Pos.x += enemyMoveSpeed * dt;
    }

    // ====== Mode toggles (your original AABB / OBB debug modes) ======
    static bool aabbMode = false;  // AABB slide + edge collision
    static bool obbMode = false;  // OBB spin + chase test
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

    // ====== Find ALL Player Entities (unchanged) ======
    std::vector<Entity> playerEntities;
    auto logicSys = g_coordinator->GetSystem<LogicSystem>();
    for (auto const& e : logicSys->EntityMember)
    {
        if (!g_coordinator->HasComponent<LogicComponent>(e))
            continue;

        auto& logic = g_coordinator->GetComponent<LogicComponent>(e);
        if (logic.ScriptName == "PlayerScript")
        {
            playerEntities.push_back(e);
        }
    }

    if (playerEntities.empty())
        return;

    CollisionManager collision;

    // ====================================================================
    // === MODE 1: AABB SLIDE & EDGE COLLISION (M) ========================
    // ====================================================================
    if (aabbMode)
    {
        float slideSpeed = 300.0f;   // units per second
        enemy_transform.Pos.x -= slideSpeed * dt;

        AABB enemyAABB = collision.BuildAABBFromTransform(enemy_transform, enemy_box);

        // Loop through ALL players to check collision
        for (Entity playerEntity : playerEntities)
        {
            if (!g_coordinator->HasComponent<Framework::Transform>(playerEntity) ||
                !g_coordinator->HasComponent<AABB>(playerEntity) ||
                !g_coordinator->HasComponent<Health>(playerEntity))
                continue;

            auto& player_transform = g_coordinator->GetComponent<Framework::Transform>(playerEntity);
            auto& player_box = g_coordinator->GetComponent<AABB>(playerEntity);
            auto& player_health = g_coordinator->GetComponent<Health>(playerEntity);

            AABB playerAABB = collision.BuildAABBFromTransform(player_transform, player_box);

            float edgeThreshold = 2.0f; // sensitivity adjustment
            bool edgeHit = collision.CheckEdgeContactCollision(playerAABB, enemyAABB, edgeThreshold);

            if (edgeHit && enemy_health.isAlive && player_health.isAlive)
            {
                // Play sound from enemy's AudioSource component
                if (g_coordinator->HasComponent<AudioSource>(ID)) {
                    auto& audioSrc = g_coordinator->GetComponent<AudioSource>(ID);
                    if (!audioSrc.audio_id.empty()) {
                        if (auto audioManager = g_coordinator->GetSystem<AudioManager>()) {
                            audioManager->PlaySound(audioSrc.audio_id);
                        }
                    }
                }

                enemy_health.isAlive = false;
                player_health.hp -= enemy_health.damage;

                std::cout << ">>> EDGE COLLISION DETECTED (AABB) <<<\n";
                std::cout << "Player HP: " << player_health.hp << "\n";

                if (player_health.hp <= 0)
                {
                    player_health.hp = 0;
                    player_health.isAlive = false;
                    std::cout << ">>> PLAYER DEFEATED\n";
                }

                Factory factory;
                factory.DeleteEntity(ID);
                return;  // Exit after collision
            }
        }

        // If in AABB debug mode, don't run rhythm logic
        return;
    }

    // ====================================================================
    // === MODE 2: OBB ROTATION CHASE & EDGE COLLISION (N) ===============
    // ====================================================================
    if (obbMode)
    {
        static double last = glfwGetTime();
        double now = glfwGetTime();
        float dtOBB = static_cast<float>(now - last);
        last = now;

        // Spin enemy clockwise
        enemy_transform.Rotation.x += 180.0f * dtOBB;
        if (enemy_transform.Rotation.x >= 360.0f)
            enemy_transform.Rotation.x -= 360.0f;

        Entity firstPlayer = playerEntities[0];
        auto& player_transform = g_coordinator->GetComponent<Framework::Transform>(firstPlayer);

        // Move toward player
        Vector2 dir = player_transform.Pos - enemy_transform.Pos;
        float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.001f)
        {
            dir.x /= len;
            dir.y /= len;
            enemy_transform.Pos.x += dir.x * 100.0f * dtOBB;
            enemy_transform.Pos.y += dir.y * 100.0f * dtOBB;
        }

        // loop through ALL players to check collision
        for (Entity playerEntity : playerEntities)
        {
            if (!g_coordinator->HasComponent<Framework::Transform>(playerEntity) ||
                !g_coordinator->HasComponent<AABB>(playerEntity) ||
                !g_coordinator->HasComponent<Health>(playerEntity))
                continue;

            auto& current_player_transform = g_coordinator->GetComponent<Framework::Transform>(playerEntity);
            auto& current_player_box = g_coordinator->GetComponent<AABB>(playerEntity);
            auto& current_player_health = g_coordinator->GetComponent<Health>(playerEntity);

            // --- Build OBBs & Check ---
            OBB enemyOBB = collision.BuildOBBFromTransform(enemy_transform, enemy_box);
            OBB playerOBB = collision.BuildOBBFromTransform(current_player_transform, current_player_box);

            bool obbHit = collision.CheckOBBvsOBB(enemyOBB, playerOBB);

            if (obbHit && enemy_health.isAlive && current_player_health.isAlive)
            {
                // Play sound from enemy's AudioSource component
                if (g_coordinator->HasComponent<AudioSource>(ID)) {
                    auto& audioSrc = g_coordinator->GetComponent<AudioSource>(ID);
                    if (!audioSrc.audio_id.empty()) {
                        if (auto audioManager = g_coordinator->GetSystem<AudioManager>()) {
                            audioManager->PlaySound(audioSrc.audio_id);
                        }
                    }
                }

                enemy_health.isAlive = false;
                current_player_health.hp -= enemy_health.damage;

                std::cout << ">>> EDGE COLLISION DETECTED via OBB <<<\n";
                std::cout << "Player HP: " << current_player_health.hp << "\n";

                if (current_player_health.hp <= 0)
                {
                    current_player_health.hp = 0;
                    current_player_health.isAlive = false;
                    std::cout << ">>> PLAYER DEFEATED\n";
                }

                Factory factory;
                factory.DeleteEntity(ID);
                return;  // Exit after collision
            }
        }

        // If in OBB debug mode, don't run rhythm logic
        return;
    }

    // ====================================================================
    // === RHYTHM MODE (DEFAULT) ==========================================
    // ====================================================================
    // Only runs when BOTH aabbMode and obbMode are OFF.
    // There are 2 enemies using this script. Second enemy starts AFTER first dies.

    // ---- Grab first player (for direction) ----
    Entity mainPlayer = playerEntities[0];
    auto& main_player_transform = g_coordinator->GetComponent<Framework::Transform>(mainPlayer);
    auto& main_player_box = g_coordinator->GetComponent<AABB>(mainPlayer);
    auto& main_player_health = g_coordinator->GetComponent<Health>(mainPlayer);

    // ---- Per-enemy rhythm state (stored in a static map) ----
    struct RhythmState
    {
        int   phase = 0;     // 0 = EnterFromRight, 1 = Combo_MoveLeft, 2 = PreAttack, 3 = FinalRush
        float phaseTimer = 0.0f;
        bool  started = false;
    };

    static std::unordered_map<Entity, RhythmState> s_rhythmStates;
    static Entity s_firstEnemyId = static_cast<Entity>(-1);
    static bool   s_firstEnemyCompleted = false;

    if (g_enemyRhythmNeedsReset) {
        std::cout << "[EnemyScript] Resetting rhythm state after scene reload\n";
        s_rhythmStates.clear();
        s_firstEnemyId = static_cast<Entity>(-1);
        g_enemyRhythmNeedsReset = false;
    }

    RhythmState& rs = s_rhythmStates[ID];

    // ------------------------------------------------------------
    // DETERMINE FIRST AND SECOND ENEMY BEHAVIOR
    // ------------------------------------------------------------
    if (!rs.started)
    {
        // FIRST enemy logic
        if (s_firstEnemyId == static_cast<Entity>(-1))
        {
            // This enemy becomes the FIRST enemy
            s_firstEnemyId = ID;
            rs.started = true;         // allow phase updates
            rs.phase = 0;
            rs.phaseTimer = 0.0f;
        }
        else
        {
            // This enemy is SECOND enemy.

            // If first enemy NOT YET completed → second enemy MUST WAIT.
            if (!s_firstEnemyCompleted)
            {
                // IMPORTANT FIX:
                // Do NOT allow phaseTimer to increase, do NOT allow rs.started to become true.
                // Enemy is completely frozen until first enemy dies.
                return;
            }

            // First enemy is completed → now second enemy may begin
            rs.started = true;
            rs.phase = 0;
            rs.phaseTimer = 0.0f;
        }
    }


    // ---- Speeds and settings for rhythm behavior ----
    float slowSpeed = 330.0f;    // entering + during combo
    float mediumSpeed = 200.0f;   // pre-attack approach
    float fastSpeed = 300.0f;   // final rush
    /*float stopDistance = 150.0f;*/  // leave space before player during pre-attack

    // ---- Rhythm Phase Machine ----
    rs.phaseTimer += dt;

    switch (rs.phase)
    {
        // =====================================================
        // PHASE 1:
        // Enter from right → move left for 2 seconds
        // Then FREEZE for 4 seconds
        // =====================================================
    case 0:
    {
        if (rs.phaseTimer < 2.0f)
        {
            // Move left for the first 2 seconds
            enemy_transform.Pos.x -= slowSpeed * dt;
        }
        else if (rs.phaseTimer < 6.0f)
        {
            // Freeze enemy for 4 seconds (no movement)
            // do nothing
        }
        else
        {
            // After 6 seconds total → move to phase 1
            rs.phase = 1;
            rs.phaseTimer = 0.0f;
        }
        break;
    }

    // =====================================================
    // PHASE 2:
    // Combo window → move left for 4 seconds
    // Then FREEZE for 4 seconds
    // =====================================================
    case 1:
    {
        if (rs.phaseTimer < 4.0f)
        {
            // Move left slowly during combo duration
            enemy_transform.Pos.x -= mediumSpeed * dt;
        }
        else if (rs.phaseTimer < 8.0f)
        {
            // Freeze enemy for 4 seconds
            // do nothing
        }
        else
        {
            // After 8 seconds → move to phase 2
            rs.phase = 2;
            rs.phaseTimer = 0.0f;
        }
        break;
    }

    // =====================================================
    // PHASE 3:
    // Final rush → full-speed toward player
    // AABB collision kills enemy
    // =====================================================
    case 2:
    {
        Vector2 diff = main_player_transform.Pos - enemy_transform.Pos;
        float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);

        if (dist > 0.001f)
        {
            diff.x /= dist;
            diff.y /= dist;
        }

        enemy_transform.Pos.x += diff.x * fastSpeed * dt;
        enemy_transform.Pos.y += diff.y * fastSpeed * dt;

        // AABB collision check
        AABB enemyAABB = collision.BuildAABBFromTransform(enemy_transform, enemy_box);
        AABB playerAABB = collision.BuildAABBFromTransform(main_player_transform, main_player_box);

        if (collision.CheckAABBCollision(enemyAABB, playerAABB))
        {
            // Play sound from enemy's AudioSource component
            if (g_coordinator->HasComponent<AudioSource>(ID)) {
                auto& audioSrc = g_coordinator->GetComponent<AudioSource>(ID);
                if (!audioSrc.audio_id.empty()) {
                    if (auto audioManager = g_coordinator->GetSystem<AudioManager>()) {
                        audioManager->PlaySound(audioSrc.audio_id);
                    }
                }
            }

            enemy_health.isAlive = false;
            main_player_health.hp -= enemy_health.damage;

            if (main_player_health.hp <= 0)
            {
                main_player_health.hp = 0;
                main_player_health.isAlive = false;
            }

            // Mark first enemy as completed
            if (ID == s_firstEnemyId)
                s_firstEnemyCompleted = true;

            Factory factory;
            factory.DeleteEntity(ID);
            return;
        }
        break;
    }

    default:
        break;
    }
}

void EnemyScript::Exit(Entity ID)
{
    (void)ID;
}

// =====================================================================
// BUTTON SCRIPT
// =====================================================================
void ButtonScript::Init(Entity ID)
{
    auto* coord = Coordinator::GetInstance();

    // Create button data for this entity
    ButtonData& data = buttonDataMap[ID];

    if (!coord->HasComponent<Name>(ID)) return;
    auto& name = coord->GetComponent<Name>(ID);
    data.buttonName = name.name;

    // Get textures from Renderable
    if (coord->HasComponent<Framework::Renderable>(ID)) {
        auto& renderable = coord->GetComponent<Framework::Renderable>(ID);
        data.normalTexture = renderable.spriteName;

        // Construct hover texture name
        size_t extPos = data.normalTexture.find_last_of('.');
        if (extPos != std::string::npos) {
            data.hoverTexture = data.normalTexture.substr(0, extPos) + "_hover" + data.normalTexture.substr(extPos);
        }
    }

    /*std::cout << "[ButtonScript] Init: " << data.buttonName << " Normal: " << data.normalTexture  << " Hover: " << data.hoverTexture << std::endl;*/
}

void ButtonScript::Update(Entity ID)
{
    auto* coord = Coordinator::GetInstance();
    auto inputMgr = coord->GetSystem<InputManager>();
    if (!inputMgr) return;

    // Get this button's data - if missing, initialize it
    auto it = buttonDataMap.find(ID);
    if (it == buttonDataMap.end()) {
        /*std::cout << "[ButtonScript] Re-initializing button entity " << ID << std::endl;*/
        Init(ID);  // Re-initialize if missing
        it = buttonDataMap.find(ID);
        if (it == buttonDataMap.end()) return; // Still missing, bail out
    }
    ButtonData& data = it->second;

    if (!coord->HasComponent<Framework::Transform>(ID)) return;
    if (!coord->HasComponent<Framework::Renderable>(ID)) return;

    auto& transform = coord->GetComponent<Framework::Transform>(ID);
    auto& renderable = coord->GetComponent<Framework::Renderable>(ID);
    if (!transform.isVisible) return;

    // Get mouse position
    glm::vec2 mouseVirtual = inputMgr->getMousePositionVirtual();
    Vector2 mousePos{ mouseVirtual.x, mouseVirtual.y };

    // Calculate AABB directly (DON'T use BuildAABBFromTransform)
    AABB buttonAABB;
    buttonAABB.width = transform.Scale.x;
    buttonAABB.height = transform.Scale.y;
    buttonAABB.min.x = transform.Pos.x - (buttonAABB.width * 0.5f);
    buttonAABB.max.x = transform.Pos.x + (buttonAABB.width * 0.5f);
    buttonAABB.min.y = transform.Pos.y - (buttonAABB.height * 0.5f);
    buttonAABB.max.y = transform.Pos.y + (buttonAABB.height * 0.5f);

    // DEBUG - print every second for NEW GAME button
    //static int debugFrameCount = 0;
    //debugFrameCount++;
    //if (debugFrameCount % 60 == 0) {
    //    std::cout << "\n===== " << data.buttonName << " =====" << std::endl;
    //    std::cout << "Transform.Pos: (" << transform.Pos.x << ", " << transform.Pos.y << ")" << std::endl;
    //    std::cout << "Transform.Scale: (" << transform.Scale.x << ", " << transform.Scale.y << ")" << std::endl;
    //    std::cout << "AABB: [" << buttonAABB.min.x << " to " << buttonAABB.max.x
    //        << ", " << buttonAABB.min.y << " to " << buttonAABB.max.y << "]" << std::endl;
    //    std::cout << "Mouse: (" << mousePos.x << ", " << mousePos.y << ")" << std::endl;
    //    std::cout << "========================\n" << std::endl;
    //}

    // Check if mouse is in bounds
    bool wasHovering = data.isHovering;
    bool inX = (mousePos.x >= buttonAABB.min.x && mousePos.x <= buttonAABB.max.x);
    bool inY = (mousePos.y >= buttonAABB.min.y && mousePos.y <= buttonAABB.max.y);
    // UI buttons work without viewport check (fixes Level1 camera issue)
    data.isHovering = inX && inY;

    // Handle texture swap
    if (data.isHovering && !wasHovering) {
        /*std::cout << "[ButtonScript] Trying to load hover texture: " << data.hoverTexture << std::endl;*/
        renderable.spriteName = data.hoverTexture;
        renderable.needsTextureReload = true;
        /*std::cout << "[ButtonScript] Hover: " << data.buttonName << std::endl;*/
    }
    else if (!data.isHovering && wasHovering) {
        renderable.spriteName = data.normalTexture;
        renderable.needsTextureReload = true;
    }

    // Handle click - track button press state per entity
    static std::unordered_map<Entity, bool> wasClickedLastFrame;
    bool isClickedThisFrame = data.isHovering && inputMgr->isMouseButtonPressed(0);
    bool wasClickedPreviously = wasClickedLastFrame[ID];

    // Trigger action on button RELEASE while still hovering (prevents double-triggers)
    if (data.isHovering && wasClickedPreviously && !isClickedThisFrame) {
        /*std::cout << "[ButtonScript] Clicked: " << data.buttonName << std::endl;*/

        auto sceneMgr = coord->GetSystem<SceneManager>();
        if (!sceneMgr) return;

        if (data.buttonName == "NewGameBtn") {
            sceneMgr->CurrentScene = "Level1";
        }
        else if (data.buttonName == "ContinueBtn") {
            std::cout << "[ButtonScript] Continue not implemented yet" << std::endl;
        }
        else if (data.buttonName == "SettingsBtn") {
            std::cout << "[ButtonScript] Settings not implemented yet" << std::endl;
        }
        else if (data.buttonName == "BackBtn") {
            sceneMgr->CurrentScene = "MainMenu";
        }
        else if (data.buttonName == "QuitBtn") {
            if (Engine) {
                glfwSetWindowShouldClose(Engine->GetWindow(), GLFW_TRUE);
            }
        }
    }

    wasClickedLastFrame[ID] = isClickedThisFrame;
}

void ButtonScript::Exit(Entity ID)
{
    // Clean up button data for this entity
    buttonDataMap.erase(ID);
}

// =====================================================================
// PULSING BORDER SCRIPT
// =====================================================================
void PulsingBorderScript::Init(Entity ID)
{
    auto* g_coordinator = Coordinator::GetInstance();

    auto rhythmInput = g_coordinator->GetSystem<RhythmGameplayInput>();
    if (rhythmInput)
    {
        bpm = rhythmInput->songBPM;
    }

    pulseTimer = 0.0f;

    // Hide border at start
    if (g_coordinator->HasComponent<Framework::Transform>(ID))
    {
        auto& transform = g_coordinator->GetComponent<Framework::Transform>(ID);
        transform.isVisible = false;
    }
}

void PulsingBorderScript::Update(Entity ID)
{
    auto* g_coordinator = Coordinator::GetInstance();

    auto audioSystem = g_coordinator->GetSystem<AudioManager>();
    if (!audioSystem)
        return;

    double musicPosition = audioSystem->GetBGMPositionSeconds();

    if (musicPosition <= 0.0)
    {
        if (g_coordinator->HasComponent<Framework::Transform>(ID))
        {
            auto& transform = g_coordinator->GetComponent<Framework::Transform>(ID);
            transform.isVisible = false;
        }
        return;
    }

    float beatInterval = 0.5f;  // Every 1 second
    float normalizedTime = fmodf((float)musicPosition, beatInterval) / beatInterval;
    float beatWindow = 0.15f;

    bool shouldBeVisible = (normalizedTime < beatWindow || normalizedTime >(1.0f - beatWindow));

    if (g_coordinator->HasComponent<Framework::Transform>(ID))
    {
        auto& transform = g_coordinator->GetComponent<Framework::Transform>(ID);

        bool wasVisible = transform.isVisible;
        if (shouldBeVisible && !wasVisible)
        {
            if (g_coordinator->HasComponent<AudioSource>(ID))
            {
                auto& audioSrc = g_coordinator->GetComponent<AudioSource>(ID);
                if (!audioSrc.audio_id.empty())
                {
                    audioSystem->PlayFromAudioSource(audioSrc);
                }
            }
        }

        transform.isVisible = shouldBeVisible;
    }
}

void PulsingBorderScript::Exit(Entity ID)
{
    std::cout << "[PulsingBorderScript] Shutdown for entity " << ID << std::endl;
}