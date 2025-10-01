// ==================================================================================
// File: collision.h
// Author: Reginald
// Brief: Collision management system (NPC, Monster, Sword, and Box-to-Line logic)
// ==================================================================================

#ifndef COLLISION_H
#define COLLISION_H

#include "../Core/Assets.h"
#include "pch/pch.h"

// -----------------------------------------------------------------------------
// CollisionManager
// -----------------------------------------------------------------------------
// Handles all collision-related systems in the ECS engine:
//  - NPC vs Monster (AABB checks at edges)
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
    AABB BuildSwordHitbox(Framework::Transform& pos, const NPC& npc);     // Build AABB for NPC's sword
    AABB BuildHeroHitbox(Framework::Transform& pos, const NPC& npc);      // Build AABB for central NPC
    AABB BuildMonsterHitbox(Framework::Transform& pos, const NPC& npc);   // Build AABB for moving monster

    // -------------------------------------------------------------------------
    // Collision Checks
    // -------------------------------------------------------------------------
    bool CheckAABBCollision(const AABB& a, const AABB& b);  // Axis-Aligned Bounding Box collision at edges
    bool CheckDistanceCollision(const Vector2& a, float aradius,
        const Vector2& b, float bradius); // Circle-based distance collision

    // -------------------------------------------------------------------------
    // Box-to-Line Collision System
    // -------------------------------------------------------------------------
    enum class LineHitResult { None, Bad, Good, Excellent };

    // Checks if a box collides with a horizontal line segment.
    // Parameters:
    //   box        -> The AABB of the moving box
    //   lineY      -> Y position of the line
    //   lineStartX -> Start X of the line
    //   lineEndX   -> End X of the line
    // Returns:
    //   LineHitResult::Bad / Good / Excellent depending on relative position
    LineHitResult CheckBoxLineCollision(const AABB& box, float lineY,
        float lineStartX, float lineEndX);

    // -------------------------------------------------------------------------
    // ECS Data
    // -------------------------------------------------------------------------
    std::vector<Entity> EntityNPC; // List of NPC entities in the world
};

#endif // COLLISION_H
