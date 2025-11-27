/******************************************************************************/
/**
 * @file        collision.h
 * @project     Pulse Protocol
 * @author      Reginald Lew Yee Ren (primary) - 98%
 * @author      Goh Pin Kai (secondary) - 2%
 * @brief       Collision management utilities handling AABB, OBB, and distance-based
 *              collision detection. Provides reusable helpers for mouse/GUI hit testing
 *              and Transform-to-bounding-box conversions. Used throughout gameplay logic
 *              for all physical interactions.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without
 *              the prior written consent of DigiPen Institute of Technology is
 *              prohibited.
 */
 /******************************************************************************/

#ifndef COLLISION_H
#define COLLISION_H

#include "CoreEngine/Assets.h"
#include "Graphics/Transform.h"

 // Forward declare
namespace Framework { class Transform; }

/**
 * @struct OBB
 * @brief Represents an Oriented Bounding Box for rotational collision detection.
 */
struct OBB
{
    Vector2 center;        // Center of the box
    Vector2 halfExtents;   // Half width / height
    float rotation;        // Rotation in degrees (use Transform::angle_disp)
    Vector2 axis[2];       // Local axes for SAT
};


// Converts rotation to radians whether it's already an angle (float)
// or a 2D direction vector (Vector2) in your Transform.
inline float ToAngleRadians(float angleRadians) {
    return angleRadians; // already radians
}

inline float ToAngleRadians(const Vector2& dir) {
    // If Rotation is a direction vector, derive angle with atan2(y, x)
    // Assumes dir is normalized or at least non-zero.
    return atan2f(dir.y, dir.x);
}

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
    AABB BuildAABBFromTransform(const Framework::Transform& transform, const AABB& baseBox);

    // --- Collision Detection ---
    bool CheckAABBCollision(const AABB& a, const AABB& b);
    bool CheckEdgeContactCollision(const AABB& playerBox, const AABB& enemyBox, float threshold);
    bool CheckDistanceCollision(const Vector2& a, float aradius, const Vector2& b, float bradius);

    // --- Rotational (OBB) Collision Support ---
    OBB BuildOBBFromTransform(const Framework::Transform& transform, const AABB& baseBox);
    bool CheckOBBvsOBB(const OBB& obbA, const OBB& obbB);
    void ResolveOBBCollision(Framework::Transform& a, Framework::Transform& b, float elasticity = 0.25f);
};

#endif // COLLISION_H
