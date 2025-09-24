#include "collision.h"
#include <iostream>

void collision_init()
{
    std::cout << "[Collision] Init\n";
}

void collision_update(Hero& hero, std::vector<Monster>& monsters)
{
    std::cout << "[Collision] Update called\n";
    for (size_t i = 0; i < monsters.size(); ++i)
    {
        Monster& monster = monsters[i];
        if (!monster.isAlive) {
            std::cout << "Monster " << i << " is dead, skipping.\n";
            continue;
        }

        // Sword vs Monster collision
        if (hero.isSwinging)
        {
            AABB swordBox = BuildSwordHitbox(hero);
            AABB monsterBox = BuildMonsterHitbox(monster);

            std::cout << "Checking sword collision for Monster " << i << "...\n";
            std::cout << "Sword Hitbox: min" << swordBox.min << " max" << swordBox.max << "\n";
            std::cout << "Monster Hitbox: min" << monsterBox.min << " max" << monsterBox.max << "\n";
            bool swordHit = CheckAABBCollision(swordBox, monsterBox);
            std::cout << "Sword collision result: " << (swordHit ? "HIT" : "NO HIT") << "\n";

            if (swordHit)
            {
                monster.isAlive = false;
                std::cout << "Monster " << i << " hit by sword!\n";
                continue; // skip hero damage if monster is dead
            }
        }

        // Hero vs Monster collision
        AABB heroBox = BuildHeroHitbox(hero);
        AABB monsterBox = BuildMonsterHitbox(monster);

        std::cout << "Checking hero collision for Monster " << i << "...\n";
        std::cout << "Hero Hitbox: min" << heroBox.min << " max" << heroBox.max << "\n";
        std::cout << "Monster Hitbox: min" << monsterBox.min << " max" << monsterBox.max << "\n";
        bool heroHit = CheckAABBCollision(heroBox, monsterBox);
        std::cout << "Hero collision result: " << (heroHit ? "HIT" : "NO HIT") << "\n";

        // Optionally: circle collision based on center and radius
        // float heroRadius = hero.width * 0.5f;
        // float monsterRadius = monster.width * 0.5f;
        // bool heroHit = CheckDistanceCollision(hero.pos, heroRadius, monster.pos, monsterRadius);

        if (heroHit)
        {
            hero.hp--;
            monster.isAlive = false;
            std::cout << "Hero hit by Monster " << i << "! HP = " << hero.hp << "\n";
        }
    }

    // Reset swinging after update
    hero.isSwinging = false;
}

void collision_exit()
{
    std::cout << "[Collision] Exit\n";
}

// Utility Functions

AABB BuildSwordHitbox(const Hero& hero)
{
    // Sword swings horizontally from hero position
    Vector2 min(hero.pos.x + hero.swingProgress, hero.pos.y);
    Vector2 max = min + Vector2(hero.swordWidth, hero.swordHeight);
    std::cout << "[BuildSwordHitbox] min" << min << " max" << max << "\n";
    return { min, max };
}

AABB BuildHeroHitbox(const Hero& hero)
{
    // Hero hitbox centered on position
    Vector2 min(hero.pos.x - hero.width / 2.0f, hero.pos.y - hero.height / 2.0f);
    Vector2 max = min + Vector2(hero.width, hero.height);
    std::cout << "[BuildHeroHitbox] min" << min << " max" << max << "\n";
    return { min, max };
}

AABB BuildMonsterHitbox(const Monster& monster)
{
    // Monster hitbox centered on position (consistent with hero)
    Vector2 min(monster.pos.x - monster.width / 2.0f, monster.pos.y - monster.height / 2.0f);
    Vector2 max = min + Vector2(monster.width, monster.height);
    std::cout << "[BuildMonsterHitbox] min" << min << " max" << max << "\n";
    return { min, max };
}

bool CheckAABBCollision(const AABB& a, const AABB& b)
{
    bool collision = (a.min.x < b.max.x &&
        a.max.x > b.min.x &&
        a.min.y < b.max.y &&
        a.max.y > b.min.y);
    std::cout << "[CheckAABBCollision] collision result: " << (collision ? "TRUE" : "FALSE") << "\n";
    return collision;
}

bool CheckDistanceCollision(const Vector2& a, float aradius, const Vector2& b, float bradius)
{
    float dist = distance(a, b);
    bool collision = dist < (aradius + bradius);
    std::cout << "[CheckDistanceCollision] distance: " << dist << " result: " << (collision ? "TRUE" : "FALSE") << "\n";
    return collision;
}
