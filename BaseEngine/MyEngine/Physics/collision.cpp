/*****************************************************************************/
/**
 * @file        collision.cpp
 * @project     Pulse Protocol
 * @author      Reginald Lew Yee Ren
 * @brief       Implements pure collision logic: AABB, sword, distance, and line hit tests.
 *              No gameplay logic or entity spawning occurs here.
 */
 /*****************************************************************************/

#include "pch/pch.h"

// ==================================================================================
// Empty system lifecycle (required by engine structure)
// ==================================================================================
void CollisionManager::Init() {}
void CollisionManager::Update(float) {}

//AABB CollisionManager::BuildHeroHitbox(Framework::Transform& pos, const NPC& npc)
//{
//    Vector2 min(pos.Pos.x - npc.width / 2.0f, pos.Pos.y - npc.height / 2.0f);
//    Vector2 max = min + Vector2(npc.width, npc.height);
//    return { min, max };
//}

AABB CollisionManager::BuildHeroHitbox(Framework::Transform& pos, const NPC& npc)
{
    // Adjust offsets to match where the hero appears visually
    constexpr float heroOffsetX = 220.0f;   // move right
    constexpr float heroOffsetY = -280.0f;  // move down
    constexpr float hitboxScale = 2.0f;     // make hitbox larger if sprite is scaled up

    float halfWidth = (npc.width * hitboxScale) / 2.0f;
    float halfHeight = (npc.height * hitboxScale) / 2.0f;

    Vector2 min((pos.Pos.x + heroOffsetX) - halfWidth,
        (pos.Pos.y + heroOffsetY) - halfHeight);
    Vector2 max((pos.Pos.x + heroOffsetX) + halfWidth,
        (pos.Pos.y + heroOffsetY) + halfHeight);

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

bool CollisionManager::CheckMouseCollision(const Vector2& mousePos, const AABB& box)
{
    // Checks if the mouse cursor (x, y) lies within the AABB boundaries.
    return (mousePos.x >= box.min.x && mousePos.x <= box.max.x &&
        mousePos.y >= box.min.y && mousePos.y <= box.max.y);
}

bool CollisionManager::CheckGUIButtonCollision(const glm::vec2& mouseVirtual,
    const glm::vec2& buttonPos,
    const glm::vec2& buttonSize)
{
    // Button is centered at buttonPos with size buttonSize
    float left = buttonPos.x - buttonSize.x * 0.5f;
    float right = buttonPos.x + buttonSize.x * 0.5f;
    float bottom = buttonPos.y - buttonSize.y * 0.5f;
    float top = buttonPos.y + buttonSize.y * 0.5f;

    return (mouseVirtual.x >= left && mouseVirtual.x <= right &&
        mouseVirtual.y >= bottom && mouseVirtual.y <= top);
}
