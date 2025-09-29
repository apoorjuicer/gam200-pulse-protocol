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
bool UseBoxLineTest = false;    // true = BoxLine test, false = Hero/Monster collision
bool SpawnLeftMonster = false;   // true = spawn monster on left, false = right
bool SwordEnabled = true;      // true = sword swings, false = sword disabled

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
        g_coordinator->AddComponent(Hero, NPC{
            Vector2(400.0f, 300.0f), 50.0f, 50.0f,
            100.0f, 50.0f, 0.0f, false, 10, true
            });
        g_coordinator->AddComponent(Hero, AABB{
            Vector2(400.0f, 300.0f),
            Vector2(450.0f, 350.0f)
            });
        EntityNPC.push_back(Hero);

        std::cout << "\n=== [Init] Hero Spawned at (400,300) ===\n";

        // --- Monster NPC: spawns left or right ---
        float startX = SpawnLeftMonster ? 100.0f : 700.0f;

        Entity Monster = g_coordinator->CreateEntity();
        g_coordinator->AddComponent(Monster, NPC{
            Vector2(startX, 300.0f), 50.0f, 50.0f,
            0.0f, 0.0f, 0.0f, false, 1, true
            });
        g_coordinator->AddComponent(Monster, AABB{
            Vector2(startX, 300.0f),
            Vector2(startX + 50.0f, 350.0f)
            });
        EntityNPC.push_back(Monster);

        std::cout << "[Init] Monster spawned at x=" << startX << "\n";
        std::cout << "============================================\n\n";
    }
    else
    {
        // --- BoxLine Test: moving box across horizontal line segments ---
        Entity boxNPC = g_coordinator->CreateEntity();
        g_coordinator->AddComponent(boxNPC, NPC{
            Vector2(0.0f, 250.0f), 50.0f, 50.0f,
            0.0f, 0.0f, 0.0f, false, 1, true
            });
        g_coordinator->AddComponent(boxNPC, AABB{
            Vector2(0.0f, 250.0f),
            Vector2(50.0f, 300.0f)
            });
        EntityNPC.push_back(boxNPC);

        std::cout << "\n=== [Init] BoxLine Test Box spawned at (0,250) ===\n";
        std::cout << "============================================\n\n";
    }
}

// ==================================================================================
// CollisionManager Update Loop
// ==================================================================================
void CollisionManager::Update(float dt)
{
    auto* g_coordinator = Coordinator::GetInstance();

    if (UseBoxLineTest)
    {
        // --- BoxLine test: move box across 4 vertical lines ---
        auto& box = g_coordinator->GetComponent<AABB>(EntityNPC[0]);
        float speed = 200.0f; // units per second

        if (box.min.x <= 600.0f)
        {
            // Move the box horizontally
            box.min.x += speed * dt;
            box.max.x += speed * dt;

            // Define 4 lines across the screen
            float lines[4] = { 100.0f, 250.0f, 400.0f, 550.0f };
            for (int i = 0; i < 4; ++i)
            {
                CollisionManager::LineHitResult result =
                    CheckBoxLineCollision(box, 250.0f, lines[i], lines[i] + 100.0f);

                if (result != LineHitResult::None)
                {
                    std::string str =
                        (result == LineHitResult::Bad ? "Bad" :
                            result == LineHitResult::Good ? "Good" :
                            result == LineHitResult::Excellent ? "Excellent" : "None");

                    std::cout << std::setw(12) << "[BoxLine] "
                        << "Line " << (i + 1) << " -> " << str << "\n";
                }
            }
        }
        return;
    }

    // --- NPC/Monster collision test ---
    auto& centre = g_coordinator->GetComponent<NPC>(EntityNPC[0]);
    auto& centreBox = g_coordinator->GetComponent<AABB>(EntityNPC[0]);
    auto& moving = g_coordinator->GetComponent<NPC>(EntityNPC[1]);
    auto& movingBox = g_coordinator->GetComponent<AABB>(EntityNPC[1]);

    if (!moving.isAlive || !centre.isAlive) return;

    // --- Move Monster ---
    float speed = 100.0f * (SpawnLeftMonster ? 1.0f : -1.0f);
    moving.pos.x += speed * dt;
    movingBox.min.x += speed * dt;
    movingBox.max.x += speed * dt;

    std::cout << std::setw(12) << "[Move] "
        << "Monster at x=" << moving.pos.x << "\n";

    // --- Sword Auto-Swing Check ---
    if (SwordEnabled)
    {
        float distanceX = std::abs(moving.pos.x - centre.pos.x);
        if (distanceX <= 150.0f)
        {
            centre.isSwinging = true;
            centre.swingProgress += 180.0f * dt; // swing speed
            if (centre.swingProgress >= 180.0f) centre.swingProgress = 0.0f;

            // Build sword hitbox and check collision
            AABB swordBox = BuildSwordHitbox(centre);
            if (CheckAABBCollision(swordBox, movingBox))
            {
                moving.isAlive = false;
                std::cout << std::setw(12) << "[SwordHit] "
                    << "Monster killed by sword!\n";
            }
        }
        else
        {
            centre.isSwinging = false;
            centre.swingProgress = 0.0f;
        }
    }

    // --- NPC vs Monster collision ---
    if (moving.isAlive && CheckAABBCollision(centreBox, movingBox))
    {
        centre.hp -= 1;
        moving.isAlive = false;
        std::cout << std::setw(12) << "[Collision] "
            << "Monster collided with Hero! Hero HP: " << centre.hp << "\n";

        if (centre.hp <= 0)
        {
            centre.isAlive = false;
            std::cout << std::setw(12) << "[Hero] "
                << "Hero has died!\n";
        }
    }
}

// ==================================================================================
// Utility Functions
// ==================================================================================

// Build an AABB for the sword swing
AABB CollisionManager::BuildSwordHitbox(const NPC& npc)
{
    Vector2 min(npc.pos.x - npc.swordWidth / 2.0f + npc.swingProgress,
        npc.pos.y - npc.swordHeight / 2.0f);
    Vector2 max = min + Vector2(npc.swordWidth, npc.swordHeight);
    return { min, max };
}

// Build AABB for Hero
AABB CollisionManager::BuildHeroHitbox(const NPC& npc)
{
    Vector2 min(npc.pos.x - npc.width / 2.0f, npc.pos.y - npc.height / 2.0f);
    Vector2 max = min + Vector2(npc.width, npc.height);
    return { min, max };
}

// Build AABB for Monster
AABB CollisionManager::BuildMonsterHitbox(const NPC& npc)
{
    Vector2 min(npc.pos.x - npc.width / 2.0f, npc.pos.y - npc.height / 2.0f);
    Vector2 max = min + Vector2(npc.width, npc.height);
    return { min, max };
}

// Standard AABB overlap check
bool CollisionManager::CheckAABBCollision(const AABB& a, const AABB& b)
{
    return (a.min.x < b.max.x && a.max.x > b.min.x &&
        a.min.y < b.max.y && a.max.y > b.min.y);
}

// Circle-based distance collision
bool CollisionManager::CheckDistanceCollision(const Vector2& a, float aradius,
    const Vector2& b, float bradius)
{
    float dist = distance(a, b);
    return dist < (aradius + bradius);
}

// Normalized Box-to-Line Collision
CollisionManager::LineHitResult CollisionManager::CheckBoxLineCollision(
    const AABB& box, float lineY, float lineStartX, float lineEndX)
{
    // Check Y overlap
    if (box.max.y < lineY || box.min.y > lineY) return LineHitResult::None;

    float boxCenterX = (box.min.x + box.max.x) * 0.5f;

    // Check X bounds
    if (boxCenterX < lineStartX || boxCenterX > lineEndX)
        return LineHitResult::None;

    // Normalize position into [0,1]
    float relative = (boxCenterX - lineStartX) / (lineEndX - lineStartX);

    // Assign zones based on percentage
    if (relative <= 0.2f)   return LineHitResult::Bad;
    if (relative <= 0.4f)   return LineHitResult::Good;
    if (relative <= 0.6f)   return LineHitResult::Excellent;
    if (relative <= 0.8f)   return LineHitResult::Good;
    return LineHitResult::Bad;
}
