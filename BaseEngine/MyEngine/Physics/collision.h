/*****************************************************************************/
/**
 * @file        collision.h
 * @project     Pulse Protocol
 * @author      Reginald Lew Yee Ren
 * @brief       Collision management utilities (AABB, sword, distance, and line hit tests).
 *              Contains all collision-related calculations, separated from gameplay logic.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /*****************************************************************************/
#ifndef COLLISION_H
#define COLLISION_H

#include "../CoreEngine/Assets.h"
#include "Graphics/Transform.h"
#include "pch/pch.h"

// -----------------------------------------------------------------------------
// CollisionManager
// -----------------------------------------------------------------------------
namespace Framework { class Transform; }
class CollisionManager : public Systems
{
public:
    void Init() override;
    void Update(float dt) override;

    // --- Collision Helper Functions ---
    AABB BuildHeroHitbox(Framework::Transform& pos, const NPC& npc);
    AABB BuildMonsterHitbox(Framework::Transform& pos, const NPC& npc);

    // --- Collision Detection ---
    bool CheckAABBCollision(const AABB& a, const AABB& b);
    bool CheckDistanceCollision(const Vector2& a, float aradius,
        const Vector2& b, float bradius);

    // --- Mouse Collision ---
    bool CheckMouseCollision(const Vector2& mousePos, const AABB& box);

    // --- In-Game GUI System Collision Usage (Virtual/Screen Space) -> M2 Rubric ---
    bool CheckGUIButtonCollision(const glm::vec2& mouseVirtual, const glm::vec2& buttonPos, const glm::vec2& buttonSize);
};

#endif // COLLISION_H
