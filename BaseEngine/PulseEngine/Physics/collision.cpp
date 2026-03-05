/******************************************************************************/
/**
 * @file        collision.cpp
 * @project     Pulse Protocol
 * @author      Reginald Lew Yee Ren
 * @brief       Implements all collision mechanics such as AABB overlap, edge detection,
 *              and SAT-based OBB collision for rotated entities. Contains helper methods
 *              for building bounding volumes and detecting mouse/GUI interactions.
 *
 * @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 *
 ******************************************************************************/

#include "pch/pch.h"

void CollisionManager::Init() {}

void CollisionManager::Update(float) {
    RebuildUniformGrid();
}

// =============================================================================
// BuildAABBFromTransform
// =============================================================================
//AABB CollisionManager::BuildAABBFromTransform(const Framework::Transform& transform,
//    const AABB& baseBox)
//{
//    AABB box{};
//
//    // ------------------------------------------------------------
//    // 1) Get local box extents from min/max first (JSON style)
//    // ------------------------------------------------------------
//    float localMinX = baseBox.min.x;
//    float localMinY = baseBox.min.y;
//    float localMaxX = baseBox.max.x;
//    float localMaxY = baseBox.max.y;
//
//    float localW = localMaxX - localMinX;
//    float localH = localMaxY - localMinY;
//
//    // If min/max is not usable, fall back to width/height
//    if (localW <= 0.0f || localH <= 0.0f)
//    {
//        float w = baseBox.width;
//        float h = baseBox.height;
//
//        // last fallback: treat max as size if width/height missing
//        if (w <= 0.0f) w = baseBox.max.x;
//        if (h <= 0.0f) h = baseBox.max.y;
//
//        if (w <= 0.0f) w = 50.0f;
//        if (h <= 0.0f) h = 50.0f;
//
//        localMinX = 0.0f; localMinY = 0.0f;
//        localMaxX = w;    localMaxY = h;
//        localW = w;       localH = h;
//    }
//
//    // ------------------------------------------------------------
//    // 2) Handle 2 possible "units":
//    //    A) Small (<= ~60)  => assume 50-based units, scale by Transform.Scale/50
//    //    B) Large (> ~60)   => assume pixels already, no scaling
//    // ------------------------------------------------------------
//    float scaleX = 1.0f;
//    float scaleY = 1.0f;
//
//    if (localW <= 60.0f && transform.Scale.x > 60.0f)
//        scaleX = transform.Scale.x / 50.0f;
//
//    if (localH <= 60.0f && transform.Scale.y > 60.0f)
//        scaleY = transform.Scale.y / 50.0f;
//
//    // ------------------------------------------------------------
//    // 3) Convert from "local box coordinates" to world:
//    //    We anchor local (0..50) or (0..pixels) around Transform.Pos (CENTER).
//    //    This matches your gameplay world where Pos is centered.
//    // ------------------------------------------------------------
//    const float halfX = transform.Scale.x * 0.5f;
//    const float halfY = transform.Scale.y * 0.5f;
//
//    box.min.x = transform.Pos.x + (localMinX * scaleX) - halfX;
//    box.max.x = transform.Pos.x + (localMaxX * scaleX) - halfX;
//
//    box.min.y = transform.Pos.y + (localMinY * scaleY) - halfY;
//    box.max.y = transform.Pos.y + (localMaxY * scaleY) - halfY;
//
//    // Optional width/height set (not required for overlap test)
//    box.width = box.max.x - box.min.x;
//    box.height = box.max.y - box.min.y;
//
//    return box;
//}

// =============================================================================
// BuildAABBFromTransform
// =============================================================================
AABB CollisionManager::BuildAABBFromTransform(const Framework::Transform& transform,
    const AABB& baseBox)
{
    AABB box{};

    const float sxAbs = std::abs(transform.Scale.x);
    const float syAbs = std::abs(transform.Scale.y);

    // Treat Transform.Pos as CENTER
    const float left = transform.Pos.x - sxAbs * 0.5f;
    const float right = transform.Pos.x + sxAbs * 0.5f;
    const float top = transform.Pos.y - syAbs * 0.5f;
    const float bottom = transform.Pos.y + syAbs * 0.5f;

    bool doNormalized = (baseBox.type == AABBType::Normalized);
    bool doPixel = (baseBox.type == AABBType::Pixel);
    bool doGrid = (baseBox.type == AABBType::Grid);

    // Helper: determine if values look normalized-ish
    auto LooksNormalized = [&](const AABB& b) -> bool
        {
            const float maxAbs =
                std::max({ std::abs(b.min.x), std::abs(b.min.y),
                           std::abs(b.max.x), std::abs(b.max.y) });

            // Normalized local coords usually in [-1..1] (or [-0.5..0.5]).
            return maxAbs <= 2.0f;
        };

    // --- AUTO mode: keep your existing logic ---
    if (baseBox.type == AABBType::Auto)
    {
        // Detect normalized local AABB [-0.5..0.5] or similar small range
        const bool looksNormalized =
            (std::abs(baseBox.min.x) <= 1.0f && std::abs(baseBox.min.y) <= 1.0f &&
                std::abs(baseBox.max.x) <= 1.0f && std::abs(baseBox.max.y) <= 1.0f);

        if (looksNormalized)
        {
            doNormalized = true;
        }
        else
        {
            // Legacy “grid units” AABB like min=-0.5 max=10 (old placeholder)
            const float derivedW = std::abs(baseBox.max.x - baseBox.min.x);
            const float derivedH = std::abs(baseBox.max.y - baseBox.min.y);

            const bool looksLikeOldPlaceholder =
                (derivedW > 0.0f && derivedW <= 12.0f) &&
                (derivedH > 0.0f && derivedH <= 12.0f);

            // If it looks like the old placeholder, use FULL sprite bounds.
            if (looksLikeOldPlaceholder)
            {
                box.min = { left,  top };
                box.max = { right, bottom };
                box.width = right - left;
                box.height = bottom - top;
                return box;
            }

            // Otherwise keep your existing behavior
            const bool looksLikeGridUnits =
                (std::abs(baseBox.max.x) <= 100.0f && std::abs(baseBox.max.y) <= 100.0f &&
                    std::abs(baseBox.min.x) <= 20.0f && std::abs(baseBox.min.y) <= 20.0f);

            const float gx = looksLikeGridUnits ? (sxAbs / 50.0f) : 1.0f;
            const float gy = looksLikeGridUnits ? (syAbs / 50.0f) : 1.0f;

            // Using sprite TOP-LEFT anchor
            box.min.x = left + baseBox.min.x * gx;
            box.min.y = top + baseBox.min.y * gy;
            box.max.x = left + baseBox.max.x * gx;
            box.max.y = top + baseBox.max.y * gy;

            if (box.min.x > box.max.x) std::swap(box.min.x, box.max.x);
            if (box.min.y > box.max.y) std::swap(box.min.y, box.max.y);

            box.width = box.max.x - box.min.x;
            box.height = box.max.y - box.min.y;
            return box;
        }
    }

    // --- Explicit modes ---
    if (doNormalized)
    {
        // SAFETY: if someone put pixel-ish numbers into a Normalized box,
        // treat it like Pixel offsets from CENTER to avoid giant AABBs.
        if (!LooksNormalized(baseBox))
        {
            // Fallback to Pixel behavior (offset from center, unscaled)
            box.min.x = transform.Pos.x + baseBox.min.x;
            box.max.x = transform.Pos.x + baseBox.max.x;
            box.min.y = transform.Pos.y + baseBox.min.y;
            box.max.y = transform.Pos.y + baseBox.max.y;
        }
        else
        {
            // Normalized local space => scale by Transform.Scale around center
            box.min.x = transform.Pos.x + baseBox.min.x * sxAbs;
            box.max.x = transform.Pos.x + baseBox.max.x * sxAbs;
            box.min.y = transform.Pos.y + baseBox.min.y * syAbs;
            box.max.y = transform.Pos.y + baseBox.max.y * syAbs;
        }
    }
    else if (doPixel)
    {
        // Pixel: absolute offset from center (unscaled)
        box.min.x = transform.Pos.x + baseBox.min.x;
        box.max.x = transform.Pos.x + baseBox.max.x;
        box.min.y = transform.Pos.y + baseBox.min.y;
        box.max.y = transform.Pos.y + baseBox.max.y;
    }
    else if (doGrid)
    {
        // Grid: scale by (Scale/50.0f), anchor top-left
        const float gx = sxAbs / 50.0f;
        const float gy = syAbs / 50.0f;

        box.min.x = left + baseBox.min.x * gx;
        box.min.y = top + baseBox.min.y * gy;
        box.max.x = left + baseBox.max.x * gx;
        box.max.y = top + baseBox.max.y * gy;
    }
    else
    {
        // If type is somehow invalid/unset, default to Normalized
        box.min.x = transform.Pos.x + baseBox.min.x * sxAbs;
        box.max.x = transform.Pos.x + baseBox.max.x * sxAbs;
        box.min.y = transform.Pos.y + baseBox.min.y * syAbs;
        box.max.y = transform.Pos.y + baseBox.max.y * syAbs;
    }

    if (box.min.x > box.max.x) std::swap(box.min.x, box.max.x);
    if (box.min.y > box.max.y) std::swap(box.min.y, box.max.y);

    box.width = box.max.x - box.min.x;
    box.height = box.max.y - box.min.y;
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

// =============================================================================
// Uniform Grid Broad Phase Methods
// =============================================================================
bool CollisionManager::BroadPhaseAllows(unsigned int a, unsigned int b) const
{
    // If grid is empty, we fail open (don’t block collisions)
    if (m_candidatePairs.empty())
        return true;

    PairKey key = MakePair(a, b);
    return (m_candidatePairs.find(key) != m_candidatePairs.end());
}

void CollisionManager::RebuildUniformGrid()
{
    m_cells.clear();
    m_candidatePairs.clear();
    m_broadPairsLastFrame = 0;

    auto g = Coordinator::GetInstance();

    // mEntities comes from System base class
    for (Entity id : EntityMember)
    {
        if (!g->HasComponent<Framework::Transform>(id) ||
            !g->HasComponent<AABB>(id))
            continue;

        auto& t = g->GetComponent<Framework::Transform>(id);
        auto& base = g->GetComponent<AABB>(id);

        AABB world = BuildAABBFromTransform(t, base);

        int minX = static_cast<int>(std::floor(world.min.x / m_cellSize));
        int maxX = static_cast<int>(std::floor(world.max.x / m_cellSize));
        int minY = static_cast<int>(std::floor(world.min.y / m_cellSize));
        int maxY = static_cast<int>(std::floor(world.max.y / m_cellSize));

        for (int cy = minY; cy <= maxY; ++cy)
        {
            for (int cx = minX; cx <= maxX; ++cx)
            {
                m_cells[{ cx, cy }].push_back(id);
            }
        }
    }

    // 2) Build candidate pairs within each cell
    for (auto& kv : m_cells)
    {
        auto& list = kv.second;
        const size_t n = list.size();
        if (n < 2) continue;

        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = i + 1; j < n; ++j)
            {
                m_candidatePairs.insert(MakePair(list[i], list[j]));
            }
        }
    }

    m_broadPairsLastFrame = m_candidatePairs.size();

    //Optional debug proof (uncomment in debug builds)
   //static int frameCounter = 0;
   //if (++frameCounter % 60 == 0) // print once per ~60 frames
   //{
   //    std::cout << "[BroadPhase] cells=" << m_cells.size()
   //        << " candidatePairs=" << m_candidatePairs.size()
   //        << " entities=" << EntityMember.size()
   //        << "\n";
   //}
}
