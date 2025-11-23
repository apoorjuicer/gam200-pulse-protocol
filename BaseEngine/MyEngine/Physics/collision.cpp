/******************************************************************************/
/**
 * @file        collision.cpp
 * @project     Pulse Protocol
 * @author      Reginald Lew Yee Ren
 * @brief       Implements all collision mechanics such as AABB overlap, edge detection,
 *              and SAT-based OBB collision for rotated entities. Contains helper methods
 *              for building bounding volumes and detecting mouse/GUI interactions.
 * 
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 * 
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
    box.width = baseBox.width * transform.Scale.x / 50.0f;
    box.height = baseBox.height * transform.Scale.y / 50.0f;

    box.min.x = transform.Pos.x - (box.width * 0.5f);
    box.max.x = transform.Pos.x + (box.width * 0.5f);
    box.min.y = transform.Pos.y - (box.height * 0.5f);
    box.max.y = transform.Pos.y + (box.height * 0.5f);

    std::cout << std::fixed << std::setprecision(2)
        << "[DEBUG] BuildAABBFromTransform -> Pos(" << transform.Pos.x << "," << transform.Pos.y << ") "
        << "Width=" << box.width << " Height=" << box.height << "\n"
        << "AABB Min(" << box.min.x << "," << box.min.y << ") Max(" << box.max.x << "," << box.max.y << ")\n";

    return box;
}


// =============================================================================
// Collision Checks
// =============================================================================
bool CollisionManager::CheckAABBCollision(const AABB& a, const AABB& b)
{
    return (
        a.min.x <= b.max.x &&
        a.max.x >= b.min.x &&
        a.min.y <= b.max.y &&
        a.max.y >= b.min.y
        );
}

bool CollisionManager::CheckEdgeContactCollision(const AABB& a, const AABB& b, float threshold)
{
    // Horizontal edge contact (X direction)
    bool xEdgeContact =
        (std::abs(a.max.x - b.min.x) <= threshold) || // A right vs B left
        (std::abs(b.max.x - a.min.x) <= threshold);   // B right vs A left

    // Vertical edge contact (Y direction)
    bool yEdgeContact =
        (std::abs(a.max.y - b.min.y) <= threshold) || // A top vs B bottom
        (std::abs(b.max.y - a.min.y) <= threshold);   // B top vs A bottom

    // Allow slight Y and X proximity so height difference doesn�t block edge collision
    bool yCloseEnough = std::abs(((a.min.y + a.max.y) / 2) - ((b.min.y + b.max.y) / 2)) <= ((a.height + b.height) * 0.5f);
    bool xCloseEnough = std::abs(((a.min.x + a.max.x) / 2) - ((b.min.x + b.max.x) / 2)) <= ((a.width + b.width) * 0.5f);

    // Trigger collision when edges meet in either direction, with small tolerance
    return (xEdgeContact && yCloseEnough) || (yEdgeContact && xCloseEnough);
}


bool CollisionManager::CheckDistanceCollision(const Vector2& a, float aradius,
    const Vector2& b, float bradius)
{
    float dist = distance(a, b);
    return dist < (aradius + bradius);
}

// =============================================================================
// Rotational Oriented Bounding Box (OBB) Collision
// =============================================================================
OBB CollisionManager::BuildOBBFromTransform(const Framework::Transform& transform, const AABB& baseBox)
{
    OBB obb;

    // === 1. Center remains the transform position ===
    obb.center = transform.Pos;

    // === 2. Build an aligned AABB first ===
    AABB aabb = BuildAABBFromTransform(transform, baseBox);

    // Compute half extents directly from AABB size
    obb.halfExtents = Vector2(
        (aabb.max.x - aabb.min.x) * 0.5f,
        (aabb.max.y - aabb.min.y) * 0.5f
    );

    // === 3. Assign rotation from Transform ===
    obb.rotation = transform.Rotation.x;

    // === 4. Compute orientation axes ===
    float rad = obb.rotation * (3.14159265359f / 180.0f);

    obb.axis[0] = Vector2(std::cos(rad), std::sin(rad));     // X axis
    obb.axis[1] = Vector2(-std::sin(rad), std::cos(rad));    // Y axis

    // === 5. Debug (optional, keep yours) ===
    std::cout << std::fixed << std::setprecision(2)
        << "[DEBUG] BuildOBBFromTransform -> Pos("
        << transform.Pos.x << "," << transform.Pos.y << ") "
        << "AABB Extents(" << obb.halfExtents.x << "," << obb.halfExtents.y << ") "
        << "Rotation=" << obb.rotation << " deg\n";

    return obb;
}


// =============================================================================
// SAT-based OBB vs OBB collision detection (with detailed debug)
// =============================================================================
bool CollisionManager::CheckOBBvsOBB(const OBB& obbA, const OBB& obbB)
{
    std::cout << std::fixed << std::setprecision(2)
        << "[DEBUG] OBB A halfExtents=(" << obbA.halfExtents.x << "," << obbA.halfExtents.y << ")"
        << " | OBB B halfExtents=(" << obbB.halfExtents.x << "," << obbB.halfExtents.y << ")\n";

    // Compute local axes
    float cosA = cosf(obbA.rotation), sinA = sinf(obbA.rotation);
    float cosB = cosf(obbB.rotation), sinB = sinf(obbB.rotation);

    Vector2 axes[4] = {
        { cosA, sinA },  // A's X axis
        { -sinA, cosA }, // A's Y axis
        { cosB, sinB },  // B's X axis
        { -sinB, cosB }  // B's Y axis
    };

    auto project = [&](const OBB& box, const Vector2& axis)
        {
            float c = cosf(box.rotation), s = sinf(box.rotation);
            Vector2 he = box.halfExtents;

            // Local corners (rotated)
            Vector2 corners[4] = {
                box.center + Vector2(c * he.x - s * he.y, s * he.x + c * he.y),
                box.center + Vector2(-c * he.x - s * he.y, -s * he.x + c * he.y),
                box.center + Vector2(c * he.x + s * he.y, s * he.x - c * he.y),
                box.center + Vector2(-c * he.x + s * he.y, -s * he.x - c * he.y)
            };

            float minProj = corners[0].x * axis.x + corners[0].y * axis.y;
            float maxProj = minProj;

            for (int i = 1; i < 4; ++i)
            {
                float proj = corners[i].x * axis.x + corners[i].y * axis.y;
                minProj = std::min(minProj, proj);
                maxProj = std::max(maxProj, proj);
            }

            // --- Debug each projection ---
            std::cout << "Axis (" << axis.x << ", " << axis.y << ") -> "
                << "MinAABB: " << minProj << " MaxAABB: " << maxProj << "\n";

            return std::make_pair(minProj, maxProj);
        };

    // --- Print out all corners for both OBBs ---
    auto printCorners = [&](const char* name, const OBB& box)
        {
            float c = cosf(box.rotation), s = sinf(box.rotation);
            Vector2 he = box.halfExtents;
            Vector2 corners[4] = {
                box.center + Vector2(c * he.x - s * he.y, s * he.x + c * he.y),
                box.center + Vector2(-c * he.x - s * he.y, -s * he.x + c * he.y),
                box.center + Vector2(c * he.x + s * he.y, s * he.x - c * he.y),
                box.center + Vector2(-c * he.x + s * he.y, -s * he.x - c * he.y)
            };
            std::cout << "=== " << name << " OBB corners ===\n";
            for (int i = 0; i < 4; ++i)
                std::cout << "(" << corners[i].x << ", " << corners[i].y << ")\n";
        };

    printCorners("OBB A", obbA);
    printCorners("OBB B", obbB);

    // --- Perform SAT overlap checks ---
    bool overlap = true;
    for (int i = 0; i < 4; ++i)
    {
        Vector2 axis = axes[i];
        float len = sqrtf(axis.x * axis.x + axis.y * axis.y);
        if (len != 0.0f)
        {
            axis.x /= len;
            axis.y /= len;
        }

        auto [minA, maxA] = project(obbA, axis);
        auto [minB, maxB] = project(obbB, axis);

        std::cout << "[SAT] Axis " << i
            << " | A(" << minA << "," << maxA << ") "
            << "B(" << minB << "," << maxB << ")\n";

        // --- Apply small tolerance for near-edge overlap ---
        const float SAT_EPS = 2.0f; // small buffer for edge contacts (~0.4% of your 500-scale)

        if (maxA < (minB - SAT_EPS) || maxB < (minA - SAT_EPS))
        {
            std::cout << "[SAT] Separation found on axis " << i << " (within epsilon)\n";
            overlap = false;
            break;
        }

    }

    std::cout << (overlap ? ">>> COLLISION: TRUE\n" : ">>> COLLISION: FALSE\n");
    return overlap;
}

// =============================================================================
// OBB collision response (unchanged)
// =============================================================================
void CollisionManager::ResolveOBBCollision(Framework::Transform& a, Framework::Transform& b,
    float elasticity)
{
    Vector2 diff = { b.Pos.x - a.Pos.x, b.Pos.y - a.Pos.y };
    float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);
    if (dist == 0.0f)
        return;

    Vector2 normal = { diff.x / dist, diff.y / dist };
    Vector2 push = { 0.5f * (1.0f - elasticity) * normal.x,
                     0.5f * (1.0f - elasticity) * normal.y };

    a.Pos.x -= push.x;
    a.Pos.y -= push.y;
    b.Pos.x += push.x;
    b.Pos.y += push.y;
}
