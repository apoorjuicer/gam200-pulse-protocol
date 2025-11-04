/*****************************************************************************/
/**
 * @file        collision.h
 * @project     Pulse Protocol
 * @author      Reginald Lew Yee Ren
 * @brief       Collision management utilities (AABB, distance, mouse, and GUI hit tests).
 *              Provides reusable helper functions to build AABBs from Transform and AABB components.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 ******************************************************************************/

#ifndef COLLISION_H
#define COLLISION_H

#include "../CoreEngine/Assets.h"
#include "Graphics/Transform.h"

 // Forward declare
namespace Framework { class Transform; }

/**
 * @class CollisionManager
 * @brief Handles all collision-related computations, separated from gameplay logic.
 */
class CollisionManager : public Systems
{
public:
    void Init() override;
    void Update(float dt) override;

    // --- Core AABB Utilities ---
    /**
     * @brief Builds an axis-aligned bounding box from an entity's transform and AABB component.
     * @param transform  The entity's transform (position and scale).
     * @param baseBox    The AABB component (width, height).
     * @return AABB object with computed min/max corners.
     */
    AABB BuildAABBFromTransform(const Framework::Transform& transform, const AABB& baseBox);

    // --- Collision Detection ---
    bool CheckAABBCollision(const AABB& a, const AABB& b);
    bool CheckDistanceCollision(const Vector2& a, float aradius,
        const Vector2& b, float bradius);

    // --- Mouse and GUI Collisions ---
    bool CheckMouseCollision(const Vector2& mousePos, const AABB& box);
    bool CheckGUIButtonCollision(const glm::vec2& mouseVirtual,
        const glm::vec2& buttonPos,
        const glm::vec2& buttonSize);
};

#endif // COLLISION_H
