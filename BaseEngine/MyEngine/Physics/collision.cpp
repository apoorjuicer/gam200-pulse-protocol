// ==================================================================================
// collision.cpp  
// Author: Reginald
// Brief: AABB collision handling between NPCs, Monsters, and BoxLine tests
// ==================================================================================

#include <pch/pch.h>
#include <iostream>
#include <vector>
#include <iomanip> // for std::setw and formatting

// --- Toggles (can be modified to control behavior) ---
bool UseBoxLineTest = false;       // true = BoxLine test, false = Hero/Monster collision
bool SpawnLeftMonster = false;     // true = spawn monster on left, false = right
bool SwordEnabled = true;          // true = sword swings, false = sword disabled
bool EnableTestCases = true;       // true = print test case output, false = silent

// ==================================================================================
// CollisionManager Initialization
// ==================================================================================
void CollisionManager::Init()
{
    auto* g_coordinator = Coordinator::GetInstance();

    if (!UseBoxLineTest)
    {
        // --- Hero NPC: stationary at center of screen ---
        Entity Hero = g_coordinator->CreateEntity();
        Framework::Transform heroTransform{};
        heroTransform.Pos = { 0,0 };
        heroTransform.Scale = { 200.0f, 200.0f };
        heroTransform.mdl_ref = 1.0f;
        heroTransform.shd_ref = 0.0f;
        heroTransform.isVisible = true;
        heroTransform.currentFrame = 0;
        g_coordinator->AddComponent(Hero, heroTransform);

        g_coordinator->AddComponent(Hero, NPC{
            100.0f, 100.0f,       // width, height
            200.0f, 100.0f,      // swordWidth, swordHeight
            0.0f, false, 10, true
            });

        g_coordinator->AddComponent(Hero, BuildHeroHitbox(heroTransform, g_coordinator->GetComponent<NPC>(Hero)));
        EntityNPC.push_back(Hero);
        std::cout << "Hero spawned at center.\n";

        // --- Monster NPC: spawns left or right ---
        float startX = SpawnLeftMonster ? -500.0f : 500.0f; // relative to center

        Entity Monster = g_coordinator->CreateEntity();
        Framework::Transform MonsterTransform{};
        MonsterTransform.Pos = { startX, 0 };
        MonsterTransform.Scale = { 200.0f, 200.0f };
        MonsterTransform.mdl_ref = 1.0f;
        MonsterTransform.shd_ref = 0.0f;
        MonsterTransform.isVisible = true;
        MonsterTransform.currentFrame = 0;
        g_coordinator->AddComponent(Monster, MonsterTransform);

        g_coordinator->AddComponent(Monster, NPC{
            100.0f, 100.0f,       // width, height
            200.0f, 100.0f,      // swordWidth, swordHeight
            0.0f, false, 10, true
            });

        g_coordinator->AddComponent(Monster, BuildMonsterHitbox(MonsterTransform, g_coordinator->GetComponent<NPC>(Monster)));
        EntityNPC.push_back(Monster);
        std::cout << "[Init] Monster spawned at x=" << startX << "\n";
        std::cout << "============================================\n\n";
    }
    else
    {
        // --- BoxLine Test ---
        Entity boxNPC = g_coordinator->CreateEntity();
        g_coordinator->AddComponent(boxNPC, NPC{
            50.0f, 50.0f, 0.0f,0.0f,0.0f,false,1,true
            });
        g_coordinator->AddComponent(boxNPC, AABB{
            Vector2(0.0f, 250.0f),
            Vector2(50.0f, 300.0f)
            });
        EntityNPC.push_back(boxNPC);

        if (EnableTestCases)
        {
            std::cout << "\n=== [Init] BoxLine Test Box spawned at (0,250) ===\n";
            std::cout << "============================================\n\n";
        }
    }
}

// ==================================================================================
// CollisionManager Update Loop
// ==================================================================================
void CollisionManager::Update(float dt)
{
    auto* g_coordinator = Coordinator::GetInstance();

    if (UseBoxLineTest) return; // BoxLine logic unchanged

    // --- NPC/Monster references ---
    auto& centre = g_coordinator->GetComponent<NPC>(EntityNPC[0]);
    auto& moving = g_coordinator->GetComponent<NPC>(EntityNPC[1]);
    auto& hero_pos = g_coordinator->GetComponent<Framework::Transform>(EntityNPC[0]);
    auto& monster_pos = g_coordinator->GetComponent<Framework::Transform>(EntityNPC[1]);

    // Rebuild AABBs from center positions (edge collision)
    AABB centreBox = BuildHeroHitbox(hero_pos, centre);
    AABB movingBox = BuildMonsterHitbox(monster_pos, moving);

    if (!moving.isAlive || !centre.isAlive) return;

    // --- Move Monster ---
    float speed = 100.0f * (SpawnLeftMonster ? 1.0f : -1.0f);
    monster_pos.Pos.x += speed * dt;

    if (EnableTestCases)
        std::cout << std::setw(12) << "[Move] "
        << "Monster at x=" << monster_pos.Pos.x << "\n";

    // --- Sword Auto-Swing Check ---
    if (SwordEnabled)
    {
        centre.isSwinging = true;
        centre.swingProgress += 180.0f * dt;
        if (centre.swingProgress >= 180.0f) centre.swingProgress = 0.0f;

        // Build sword hitbox towards monster
        AABB swordBox = BuildSwordHitbox(hero_pos, centre, monster_pos.Pos);

        // Edge-based offset collision for sword
        float SwordOffset = centre.width / 2.0f;
        float MonsterOffset = moving.width / 2.0f;

        if (moving.isAlive &&
            (swordBox.max.x + SwordOffset >= movingBox.min.x - MonsterOffset &&
                swordBox.min.x - SwordOffset <= movingBox.max.x + MonsterOffset) &&
            (swordBox.max.y >= movingBox.min.y && swordBox.min.y <= movingBox.max.y))
        {
            moving.isAlive = false;
            if (EnableTestCases)
                std::cout << std::setw(12) << "[SwordHit] "
                << "Monster killed by sword at edge!\n";
        }
    }

    // --- NPC vs Monster collision at edges ---
    float HeroOffset = centre.width / 2.0f;
    float MonsterOffset = moving.width / 2.0f;

    if (moving.isAlive &&
        (centreBox.max.x + HeroOffset >= movingBox.min.x - MonsterOffset &&
            centreBox.min.x - HeroOffset <= movingBox.max.x + MonsterOffset) &&
        (centreBox.max.y >= movingBox.min.y && centreBox.min.y <= movingBox.max.y))
    {
        centre.hp -= 1;
        moving.isAlive = false;

        if (EnableTestCases)
            std::cout << std::setw(12) << "[Collision] "
            << "Monster collided with Hero! Hero HP: " << centre.hp << "\n";

        if (centre.hp <= 0)
        {
            centre.isAlive = false;
            if (EnableTestCases)
                std::cout << std::setw(12) << "[Hero] "
                << "Hero has died!\n";
        }
    }
}

// ==================================================================================
// Utility Functions
// ==================================================================================
AABB CollisionManager::BuildSwordHitbox(Framework::Transform& heroPos, const NPC& hero, const Vector2& targetPos)
{
    // Determine direction: right (+1) or left (-1)
    float dir = (targetPos.x >= heroPos.Pos.x) ? 1.0f : -1.0f;

    Vector2 min(heroPos.Pos.x + dir * hero.swingProgress - (dir < 0 ? hero.swordWidth : 0.0f),
        heroPos.Pos.y - hero.swordHeight / 2.0f);
    Vector2 max = min + Vector2(hero.swordWidth, hero.swordHeight);
    return { min, max };
}

AABB CollisionManager::BuildHeroHitbox(Framework::Transform& pos, const NPC& npc)
{
    Vector2 min(pos.Pos.x - npc.width / 2.0f, pos.Pos.y - npc.height / 2.0f);
    Vector2 max = min + Vector2(npc.width, npc.height);
    return { min, max };
}

AABB CollisionManager::BuildMonsterHitbox(Framework::Transform& pos, const NPC& npc)
{
    Vector2 min(pos.Pos.x - npc.width / 2.0f, pos.Pos.y - npc.height / 2.0f);
    Vector2 max = min + Vector2(npc.width, npc.height);
    return { min, max };
}

bool CollisionManager::CheckAABBCollision(const AABB& a, const AABB& b)
{
    return (a.min.x <= b.max.x && a.max.x >= b.min.x &&
        a.min.y <= b.max.y && a.max.y >= b.min.y);
}

bool CollisionManager::CheckDistanceCollision(const Vector2& a, float aradius,
    const Vector2& b, float bradius)
{
    float dist = distance(a, b);
    return dist < (aradius + bradius);
}

CollisionManager::LineHitResult CollisionManager::CheckBoxLineCollision(
    const AABB& box, float lineY, float lineStartX, float lineEndX)
{
    if (box.max.y < lineY || box.min.y > lineY) return LineHitResult::None;

    float boxCenterX = (box.min.x + box.max.x) * 0.5f;
    if (boxCenterX < lineStartX || boxCenterX > lineEndX)
        return LineHitResult::None;

    float relative = (boxCenterX - lineStartX) / (lineEndX - lineStartX);

    if (relative <= 0.2f)   return LineHitResult::Bad;
    if (relative <= 0.4f)   return LineHitResult::Good;
    if (relative <= 0.6f)   return LineHitResult::Excellent;
    if (relative <= 0.8f)   return LineHitResult::Good;
    return LineHitResult::Bad;
}
