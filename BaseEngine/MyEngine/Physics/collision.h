/******************************************************************************/
/**
 * @file            collision.h
 * @project         Pulse Protocol
 * @author          Reginald Lew Yee Ren
 * @brief           Collision management system (NPC, Monster, Sword, and Box-to-Line logic)
 * @copyright       Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#ifndef COLLISION_H
#define COLLISION_H

#include "../Core/Assets.h"
#include "pch/pch.h"

// -----------------------------------------------------------------------------
// CollisionManager
// -----------------------------------------------------------------------------
// Handles all collision-related systems in the ECS engine:
//  - NPC vs Monster (AABB checks)
//  - NPC sword vs Monster (AABB checks with auto-swing toggle)
//  - Box-to-Line collision detection (for timing-based systems)
// -----------------------------------------------------------------------------
class CollisionManager : public Systems
{
public:
    // -------------------------------------------------------------------------
    // System Lifecycle
    // -------------------------------------------------------------------------
    void Init() override;               // Called once at system startup
    void Update(float dt) override;     // Called every frame with delta time

    // -------------------------------------------------------------------------
    // Utility Functions for Hitbox Construction
    // -------------------------------------------------------------------------
    AABB BuildSwordHitbox(Framework::Transform& heroPos, const NPC& hero, const Vector2& targetPos); // dynamic sword hitbox
    AABB BuildHeroHitbox(Framework::Transform& pos, const NPC& npc);      // Build AABB for central NPC
    AABB BuildMonsterHitbox(Framework::Transform& pos, const NPC& npc);   // Build AABB for moving monster

    // -------------------------------------------------------------------------
    // Collision Checks
    // -------------------------------------------------------------------------
    bool CheckAABBCollision(const AABB& a, const AABB& b);  // Axis-Aligned Bounding Box collision
    bool CheckDistanceCollision(const Vector2& a, float aradius,
        const Vector2& b, float bradius); // Circle-based distance collision

    // -------------------------------------------------------------------------
    // Box-to-Line Collision System
    // -------------------------------------------------------------------------
    enum class LineHitResult { None, Bad, Good, Excellent };

    LineHitResult CheckBoxLineCollision(const AABB& box, float lineY,
        float lineStartX, float lineEndX);

    // -------------------------------------------------------------------------
    // ECS Data
    // -------------------------------------------------------------------------
    std::vector<Entity> EntityNPC; // List of NPC entities in the world

private:
    bool monsterCanMove = false; // at the start, is static
};

#endif // COLLISION_H
