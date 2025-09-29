// ==================================================================================
// File: Assets.h
// Author: Reginald
// Brief: Core asset data structures used by the ECS system
// ==================================================================================

#ifndef ASSETS_H
#define ASSETS_H

// -----------------------------------------------------------------------------
// NPC Structure
// -----------------------------------------------------------------------------
// Represents a non-playable character (hero or monster) in the ECS system.
// Includes positional data, dimensions, health, and sword attack state.
// -----------------------------------------------------------------------------
struct NPC
{
    Vector2 pos;          // World position of the NPC (center point)
    float width;          // Width of NPC hitbox
    float height;         // Height of NPC hitbox

    // Sword parameters
    float swordWidth;     // Width of the sword hitbox
    float swordHeight;    // Height of the sword hitbox
    float swingProgress;  // Current swing progress (0.0 - 1.0 normalized)
    bool isSwinging;      // True if NPC is currently swinging its sword

    // Health and status
    int hp;               // Health points (e.g., Hero = 10 HP, Monster = 1 HP)
    bool isAlive;         // True if NPC is alive, false if defeated
};

// -----------------------------------------------------------------------------
// AABB Structure
// -----------------------------------------------------------------------------
// Axis-Aligned Bounding Box used for rectangular collision detection.
// -----------------------------------------------------------------------------
struct AABB
{
    Vector2 min;          // Minimum corner (bottom-left)
    Vector2 max;          // Maximum corner (top-right)
};

#endif // ASSETS_H
