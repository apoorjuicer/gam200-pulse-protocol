#pragma once
#include "pch/pch.h"
#include <vector>

// Hero struct for collision system
struct Hero {
    Vector2 pos;
    float width, height;
    float swordWidth, swordHeight;
    float swingProgress;
    bool isSwinging;
    int hp;
};

struct Monster {
    Vector2 pos;
    float width, height;
    bool isAlive;
};

struct AABB {
    Vector2 min;
    Vector2 max;
};

void collision_init();
void collision_update(Hero& hero, std::vector<Monster>& monsters);
void collision_exit();

// Utility functions
AABB BuildSwordHitbox(const Hero& hero);
AABB BuildHeroHitbox(const Hero& hero);
AABB BuildMonsterHitbox(const Monster& monster);
bool CheckAABBCollision(const AABB& a, const AABB& b);
bool CheckDistanceCollision(const Vector2& a, float aradius, const Vector2& b, float bradius);

