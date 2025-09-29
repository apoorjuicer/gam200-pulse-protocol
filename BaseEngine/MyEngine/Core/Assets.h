// ==================================================================================
// File: Assets.h
// Author: Reginald
// Brief: Core asset data structures used by the ECS system
// ==================================================================================
#pragma once


// CHANGE THIS SO THAT IT IS ONLY COMPONENT, HERO AND MONSTERS ARE NOT COMPONENTS.
struct NPC {
    Vector2 pos;
    float width;
    float height;
    float swordWidth, swordHeight;
    float swingProgress;
    bool isSwinging;
    int hp;
    bool isAlive;
};


struct AABB {
    Vector2 min;
    Vector2 max;
};

