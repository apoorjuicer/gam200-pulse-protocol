/*****************************************************************************/
/**
 * @file        collision.cpp
 * @project     Pulse Protocol
 * @author      Reginald Lew Yee Ren
 * @brief       Implements pure collision logic: AABB, distance, and mouse hit tests.
 *              No gameplay logic or entity spawning occurs here.
 ******************************************************************************/

#include "pch/pch.h"

void CollisionManager::Init() {}
void CollisionManager::Update(float) {}

// =============================================================================
// BuildAABBFromTransform
// =============================================================================
AABB CollisionManager::BuildAABBFromTransform(const Framework::Transform& transform, const AABB& baseBox)
{
    AABB box;
    box.width = baseBox.width;
    box.height = baseBox.height;

    // Center-based AABB calculation
    box.min.x = transform.Pos.x - box.width * 0.5f;
    box.max.x = transform.Pos.x + box.width * 0.5f;
    box.min.y = transform.Pos.y - box.height * 0.5f;
    box.max.y = transform.Pos.y + box.height * 0.5f;

    return box;
}

// =============================================================================
// Collision Checks
// =============================================================================
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

// =============================================================================
// Mouse / GUI Collisions
// =============================================================================
bool CollisionManager::CheckMouseCollision(const Vector2& mousePos, const AABB& box)
{
    return (mousePos.x >= box.min.x && mousePos.x <= box.max.x &&
        mousePos.y >= box.min.y && mousePos.y <= box.max.y);
}

bool CollisionManager::CheckGUIButtonCollision(const glm::vec2& mouseVirtual,
    const glm::vec2& buttonPos,
    const glm::vec2& buttonSize)
{
    float left = buttonPos.x - buttonSize.x * 0.5f;
    float right = buttonPos.x + buttonSize.x * 0.5f;
    float bottom = buttonPos.y - buttonSize.y * 0.5f;
    float top = buttonPos.y + buttonSize.y * 0.5f;

    return (mouseVirtual.x >= left && mouseVirtual.x <= right &&
        mouseVirtual.y >= bottom && mouseVirtual.y <= top);
}
