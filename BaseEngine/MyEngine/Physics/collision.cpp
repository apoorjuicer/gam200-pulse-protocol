#include "pch/pch.h"


void collision_init()
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

void collision_update()
{

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

void collision_exit()
{

}




// WORK IN PROGRESS LOGIC
/*
if (collision_miss)
{
	if (collision_good)
	{
		if (collsiion_great)
		{
			return great;
		}

		return good;
	}
	return miss;
}

if(input == miss)
{
	inputcombo.pushback("m');
}

// from input manager VV
std::vector<keys>input_combo = "wasd;
std::vector<keys> key = { "wasd" , "wwww", "dads" };


for (int j = 0; j < 3; ++j)
{
	input_combo = key[j];
}


*/