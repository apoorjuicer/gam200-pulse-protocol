#include <pch/pch.h>
#include "collision.h"
#include <iostream>

static const float printInterval = 0.2f;
static float printTimer = 0.0f;

static bool spawnFromLeft = true; //true = start left move right,  false = start right move left
static bool automaticSwordSwing = true; // true = sword swinging, false = not swinging
static bool testBoxLineCollision = false; //true = boxline collision, false = monsterplayer

void CollisionManager::Init()
{
    // i threw all ur components into asset.h
    auto* g_coordinator = Coordinator::GetInstance();
    
    for (auto& monster : entities)
    {
        monster = g_coordinator->CreateEntity();
        g_coordinator->AddComponent(monster, Monster{ Vector2(500,200), 20, 20, true });
        g_coordinator->AddComponent(monster, AABB{ Vector2(500,200), Vector2(10,50)});
        g_coordinator->AddComponent(monster, Hero{ Vector2(500,200) });
    }

    //or

    Entity hero = g_coordinator->CreateEntity();
    g_coordinator->AddComponent(hero, Monster{ Vector2(500,200), 20, 20, true });
    g_coordinator->AddComponent(hero, AABB{ Vector2(500,200), Vector2(10,50) });
    g_coordinator->AddComponent(hero, Hero{ Vector2(500,200) });
    entities.push_back(hero);
    // so now there are 3 entity made and stored into ur own vector 

}

void CollisionManager::Update(float dt)
{
    auto* g_coordinator = Coordinator::GetInstance();

    for (auto& monster : EntityMember)
    {
       
        // note: there are 2 monsters so thats y u see it twice on console
        auto& health1 = g_coordinator->GetComponent<Monster>(monster);
        if (health1.isAlive == true)
        {
            health1.isAlive = false;
            std::cout << health1.isAlive << std::endl;
        }

        auto& box = g_coordinator->GetComponent<AABB>(monster);
        
        if (box.min.x < 505)
        {
            box.min.x += 5;
            std::cout << box.min.x << "\n";
        }
    }
   
    // IF YOU WANNA USE THIS, THE std::vector<Entity> must be in class private:
    Entity entity1 = entities[0];  
    auto& hero_AABB = g_coordinator->GetComponent<AABB>(entity1);
    hero_AABB.min.x += 50;
    //auto& heros_health = g_coordinator->GetComponent<Health>(entity1);  
    //hero.hp -= 5;




   /*std::cout << "\n=== TEST SETTINGS ===\n";
   std::cout << "Monster Spawn Side: " << (spawnFromLeft ? "LEFT" : "RIGHT") << "\n";
   std::cout << "Automatic Sword Swing: " << (automaticSwordSwing ? "ON" : "OFF") << "\n";
   std::cout << "Testing Mode: " << (testBoxLineCollision ? "BOX-TO-LINE" : "HERO/MONSTER") << "\n";
   std::cout << "=====================\n";*/

   //if (!testBoxLineCollision)
   //{
   //    // HERO/MONSTER TEST SETUP
   //    Hero hero;
   //    hero.pos = { 400.0f, 700.0f };
   //    hero.width = 50.0f;
   //    hero.height = 50.0f;
   //    hero.swordWidth = 60.0f;
   //    hero.swordHeight = 20.0f;
   //    hero.swingProgress = 0.0f;
   //    hero.isSwinging = automaticSwordSwing;
   //    hero.hp = 10;

   //    Monster monsterTemplate;
   //    monsterTemplate.width = 50.0f;
   //    monsterTemplate.height = 50.0f;
   //    monsterTemplate.isAlive = true;
   //    std::vector<Monster> monsters = { monsterTemplate };
   //    float monsterSpeed = 200.0f;

   //    if (spawnFromLeft)
   //        monsters[0].pos = { 0.0f + monsterTemplate.width / 2.0f, 700.0f };
   //    else
   //        monsters[0].pos = { WindowWidth - monsterTemplate.width / 2.0f, 700.0f };

   //    bool collisionOccurred = false;
   //    bool heroMonsterCollided = false;
   //    bool swordMonsterCollided = false;

   //    while (!glfwWindowShouldClose(window) && !collisionOccurred)
   //    {
   //        unsigned currenttime = timeGetTime();
   //        float dt = (currenttime - LastTime) / 1000.0f;
   //        LastTime = currenttime;
   //        printTimer += dt;

   //        glfwPollEvents();
   //        Update(dt);

   //        if (monsters[0].isAlive)
   //        {
   //            float monsterVelX = (spawnFromLeft ? monsterSpeed : -monsterSpeed);
   //            monsters[0].pos.x += monsterVelX * dt;
   //            std::string direction = (monsterVelX > 0) ? "RIGHT" : "LEFT";

   //            if (printTimer >= printInterval)
   //            {
   //                std::cout << "[TEST] Monster moving " << direction
   //                    << ". x = " << monsters[0].pos.x << "\n";
   //                printTimer = 0.0f;
   //            }

   //            if (!automaticSwordSwing)
   //            {
   //                if (!heroMonsterCollided && CheckAABBCollision(BuildHeroHitbox(hero), BuildMonsterHitbox(monsters[0])))
   //                {
   //                    monsters[0].isAlive = false;
   //                    hero.hp--;
   //                    heroMonsterCollided = true;
   //                    std::cout << "[Monster-Hero] Collision! Hero HP: " << hero.hp << "\n";
   //                    collisionOccurred = true;
   //                }
   //            }
   //            else
   //            {
   //                hero.swingProgress = (hero.pos.x < monsters[0].pos.x) ? hero.width : -hero.swordWidth;
   //                if (!swordMonsterCollided && CheckAABBCollision(BuildSwordHitbox(hero), BuildMonsterHitbox(monsters[0])))
   //                {
   //                    monsters[0].isAlive = false;
   //                    swordMonsterCollided = true;
   //                    std::cout << "[Sword-Monster] Collision! Hero HP: " << hero.hp << "\n";
   //                    collisionOccurred = true;
   //                }
   //            }
   //        }

   //        if ((spawnFromLeft && monsters[0].pos.x - monsterTemplate.width / 2.0f > WindowWidth) ||
   //            (!spawnFromLeft && monsters[0].pos.x + monsterTemplate.width / 2.0f < 0))
   //        {
   //            std::cout << "[TEST] Monster left screen. No collision.\n";
   //            break;
   //        }

   //        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   //        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
   //        glfwSwapBuffers(window);
   //    }
   //}
   //else
   //{
   //    // BOX-TO-LINE TEST SETUP
   //    struct Line { std::string name; float x, yStart, yEnd; };
   //    std::vector<Line> lines = {
   //        {"Line1", 200.0f, 180.0f, 280.0f},
   //        {"Line2", 300.0f, 180.0f, 280.0f},
   //        {"Line3", 400.0f, 180.0f, 280.0f},
   //        {"Line4", 500.0f, 180.0f, 280.0f}
   //    };

   //    AABB movingBox;
   //    movingBox.min = { 0.0f, 180.0f };
   //    movingBox.max = { 100.0f, 280.0f };
   //    float boxSpeed = 150.0f;
   //    bool boxLeftScreen = false;

   //    while (!glfwWindowShouldClose(window) && !boxLeftScreen)
   //    {
   //        unsigned currenttime = timeGetTime();
   //        float dt = (currenttime - LastTime) / 1000.0f;
   //        LastTime = currenttime;

   //        glfwPollEvents();
   //        Update(dt);

   //        movingBox.min.x += boxSpeed * dt;
   //        movingBox.max.x += boxSpeed * dt;

   //        for (auto& line : lines)
   //        {
   //            bool xOverlap = (movingBox.min.x <= line.x && movingBox.max.x >= line.x);
   //            bool yOverlap = (movingBox.max.y >= line.yStart && movingBox.min.y <= line.yEnd);

   //            if (xOverlap && yOverlap)
   //            {
   //                float boxCenter = (movingBox.min.x + movingBox.max.x) / 2.0f;
   //                LineHitResult result;
   //                if (boxCenter >= line.x - 10 && boxCenter <= line.x + 10) result = LineHitResult::Excellent;
   //                else if (boxCenter >= line.x - 30 && boxCenter <= line.x + 30) result = LineHitResult::Good;
   //                else result = LineHitResult::Bad;

   //                std::cout << "[BOX-TO-LINE] Box X: " << boxCenter
   //                    << " collided with " << line.name
   //                    << " -> Result: ";
   //                switch (result)
   //                {
   //                case LineHitResult::Bad:       std::cout << "Bad"; break;
   //                case LineHitResult::Good:      std::cout << "Good"; break;
   //                case LineHitResult::Excellent: std::cout << "Excellent"; break;
   //                default: break;
   //                }
   //                std::cout << "\n";
   //            }
   //        }

   //        if (movingBox.min.x > WindowWidth)
   //        {
   //            std::cout << "[BOX-TO-LINE] Box left screen. Test finished.\n";
   //            boxLeftScreen = true;
   //        }

   //        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   //        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
   //        glfwSwapBuffers(window);
   //    }
   //}

   //std::cout << "Press ENTER to exit the test..." << std::endl;
   //std::cin.get();
}

// Utility Functions
AABB CollisionManager::BuildSwordHitbox(const Hero& hero)
{
    Vector2 min(hero.pos.x + hero.swingProgress, hero.pos.y);
    Vector2 max = min + Vector2(hero.swordWidth, hero.swordHeight);
    std::cout << "[BuildSwordHitbox] min" << min << " max" << max << "\n";
    return { min, max };
}

AABB CollisionManager::BuildHeroHitbox(const Hero& hero)
{
    Vector2 min(hero.pos.x - hero.width / 2.0f, hero.pos.y - hero.height / 2.0f);
    Vector2 max = min + Vector2(hero.width, hero.height);
    std::cout << "[BuildHeroHitbox] min" << min << " max" << max << "\n";
    return { min, max };
}

AABB CollisionManager::BuildMonsterHitbox(const Monster& monster)
{
    Vector2 min(monster.pos.x - monster.width / 2.0f, monster.pos.y - monster.height / 2.0f);
    Vector2 max = min + Vector2(monster.width, monster.height);
    std::cout << "[BuildMonsterHitbox] min" << min << " max" << max << "\n";
    return { min, max };
}

bool CollisionManager::CheckAABBCollision(const AABB& a, const AABB& b)
{
    bool collision = (a.min.x < b.max.x &&
        a.max.x > b.min.x &&
        a.min.y < b.max.y &&
        a.max.y > b.min.y);
    std::cout << "[CheckAABBCollision] collision result: " << (collision ? "TRUE" : "FALSE") << "\n";
    return collision;
}

bool CollisionManager::CheckDistanceCollision(const Vector2& a, float aradius, const Vector2& b, float bradius)
{
    float dist = distance(a, b);
    bool collision = dist < (aradius + bradius);
    std::cout << "[CheckDistanceCollision] distance: " << dist << " result: " << (collision ? "TRUE" : "FALSE") << "\n";
    return collision;
}

// Box-to-Line Collision
CollisionManager::LineHitResult CollisionManager::CheckBoxLineCollision(const AABB& box, float lineY, float lineStartX, float lineEndX)
{
    // Check vertical overlap
    if (box.max.y < lineY || box.min.y > lineY) {
        return LineHitResult::None;
    }

    // Box center x-coordinate
    float boxCenterX = (box.min.x + box.max.x) / 2.0f;

    // Check if center is inside line range
    if (boxCenterX < lineStartX || boxCenterX > lineEndX) {
        return LineHitResult::None;
    }

    // Width of box
    float boxWidth = box.max.x - box.min.x;

    // Determine position relative to box width
    float relativePos = boxCenterX - box.min.x; // 0 to boxWidth

    if (relativePos >= 0 && relativePos <= 19)
        return LineHitResult::Bad;
    else if (relativePos >= 20 && relativePos <= 39)
        return LineHitResult::Good;
    else if (relativePos >= 40 && relativePos <= 60)
        return LineHitResult::Excellent;
    else if (relativePos >= 61 && relativePos <= 80)
        return LineHitResult::Good;
    else if (relativePos >= 81 && relativePos <= 100)
        return LineHitResult::Bad;

    return LineHitResult::None;
}


//void collision_init()
//{
//    std::cout << "[Collision] Init\n";
//}
//
//void collision_update(Hero& hero, std::vector<Monster>& monsters)
//{
//    std::cout << "[Collision] Update called\n";
//    for (size_t i = 0; i < monsters.size(); ++i)
//    {
//        Monster& monster = monsters[i];
//        if (!monster.isAlive) {
//            std::cout << "Monster " << i << " is dead, skipping.\n";
//            continue;
//        }
//
//        // Sword vs Monster collision
//        if (hero.isSwinging)
//        {
//            AABB swordBox = BuildSwordHitbox(hero);
//            AABB monsterBox = BuildMonsterHitbox(monster);
//
//            std::cout << "Checking sword collision for Monster " << i << "...\n";
//            std::cout << "Sword Hitbox: min" << swordBox.min << " max" << swordBox.max << "\n";
//            std::cout << "Monster Hitbox: min" << monsterBox.min << " max" << monsterBox.max << "\n";
//            bool swordHit = CheckAABBCollision(swordBox, monsterBox);
//            std::cout << "Sword collision result: " << (swordHit ? "HIT" : "NO HIT") << "\n";
//
//            if (swordHit)
//            {
//                monster.isAlive = false;
//                std::cout << "Monster " << i << " hit by sword!\n";
//                continue;
//            }
//        }
//
//        // Hero vs Monster collision
//        AABB heroBox = BuildHeroHitbox(hero);
//        AABB monsterBox = BuildMonsterHitbox(monster);
//
//        std::cout << "Checking hero collision for Monster " << i << "...\n";
//        std::cout << "Hero Hitbox: min" << heroBox.min << " max" << heroBox.max << "\n";
//        std::cout << "Monster Hitbox: min" << monsterBox.min << " max" << monsterBox.max << "\n";
//        bool heroHit = CheckAABBCollision(heroBox, monsterBox);
//        std::cout << "Hero collision result: " << (heroHit ? "HIT" : "NO HIT") << "\n";
//
//        if (heroHit)
//        {
//            hero.hp--;
//            monster.isAlive = false;
//            std::cout << "Hero hit by Monster " << i << "! HP = " << hero.hp << "\n";
//        }
//    }
//
//    // Reset swinging after update
//    hero.isSwinging = false;
//}
//
//void collision_exit()
//{
//    std::cout << "[Collision] Exit\n";
//}
//
//// Utility Functions
//AABB BuildSwordHitbox(const Hero& hero)
//{
//    Vector2 min(hero.pos.x + hero.swingProgress, hero.pos.y);
//    Vector2 max = min + Vector2(hero.swordWidth, hero.swordHeight);
//    std::cout << "[BuildSwordHitbox] min" << min << " max" << max << "\n";
//    return { min, max };
//}
//
//AABB BuildHeroHitbox(const Hero& hero)
//{
//    Vector2 min(hero.pos.x - hero.width / 2.0f, hero.pos.y - hero.height / 2.0f);
//    Vector2 max = min + Vector2(hero.width, hero.height);
//    std::cout << "[BuildHeroHitbox] min" << min << " max" << max << "\n";
//    return { min, max };
//}
//
//AABB BuildMonsterHitbox(const Monster& monster)
//{
//    Vector2 min(monster.pos.x - monster.width / 2.0f, monster.pos.y - monster.height / 2.0f);
//    Vector2 max = min + Vector2(monster.width, monster.height);
//    std::cout << "[BuildMonsterHitbox] min" << min << " max" << max << "\n";
//    return { min, max };
//}
// 

//
//bool CheckAABBCollision(const AABB& a, const AABB& b)
//{
//    bool collision = (a.min.x < b.max.x &&
//        a.max.x > b.min.x &&
//        a.min.y < b.max.y &&
//        a.max.y > b.min.y);
//    std::cout << "[CheckAABBCollision] collision result: " << (collision ? "TRUE" : "FALSE") << "\n";
//    return collision;
//}
//
//bool CheckDistanceCollision(const Vector2& a, float aradius, const Vector2& b, float bradius)
//{
//    float dist = distance(a, b);
//    bool collision = dist < (aradius + bradius);
//    std::cout << "[CheckDistanceCollision] distance: " << dist << " result: " << (collision ? "TRUE" : "FALSE") << "\n";
//    return collision;
//}
//
//// Box-to-Line Collision
//LineHitResult CheckBoxLineCollision(const AABB& box, float lineY, float lineStartX, float lineEndX)
//{
//    // Check vertical overlap
//    if (box.max.y < lineY || box.min.y > lineY) {
//        return LineHitResult::None;
//    }
//
//    // Box center x-coordinate
//    float boxCenterX = (box.min.x + box.max.x) / 2.0f;
//
//    // Check if center is inside line range
//    if (boxCenterX < lineStartX || boxCenterX > lineEndX) {
//        return LineHitResult::None;
//    }
//
//    // Width of box
//    float boxWidth = box.max.x - box.min.x;
//
//    // Determine position relative to box width
//    float relativePos = boxCenterX - box.min.x; // 0 to boxWidth
//
//    if (relativePos >= 0 && relativePos <= 19)
//        return LineHitResult::Bad;
//    else if (relativePos >= 20 && relativePos <= 39)
//        return LineHitResult::Good;
//    else if (relativePos >= 40 && relativePos <= 60)
//        return LineHitResult::Excellent;
//    else if (relativePos >= 61 && relativePos <= 80)
//        return LineHitResult::Good;
//    else if (relativePos >= 81 && relativePos <= 100)
//        return LineHitResult::Bad;
//
//    return LineHitResult::None;
//}


