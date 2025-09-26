// ==================================================================================
// collision.h  
// Author: Reginald
// Brief: AABB
//        
//        
// ==================================================================================
#ifndef COLLISION_H
#define COLLISION_H

#include "../Core/Assets.h"

class CollisionManager : public Systems
{
    
public:
    void Init() override;
    void Update(float dt) override;

    // Utility functions
    AABB BuildSwordHitbox(const Hero& hero);
    AABB BuildHeroHitbox(const Hero& hero);
    AABB BuildMonsterHitbox(const Monster& monster);
    bool CheckAABBCollision(const AABB& a, const AABB& b);
    bool CheckDistanceCollision(const Vector2& a, float aradius, const Vector2& b, float bradius);

    // Box-to-Line Collision System
    enum class LineHitResult { None, Bad, Good, Excellent };

    // Check if a box (AABB) collides with a horizontal line
    // lineY: y-coordinate of the line
    // lineStartX, lineEndX: x-range of the line
    LineHitResult CheckBoxLineCollision(const AABB& box, float lineY, float lineStartX, float lineEndX);
};



// Hero struct for collision system
//struct Hero {
//    Vector2 pos;
//    float width, height;
//    float swordWidth, swordHeight;
//    float swingProgress;
//    bool isSwinging;
//    int hp;
//};
//
//struct Monster {
//    Vector2 pos;
//    float width, height;
//    bool isAlive;
//};
//
//struct AABB {
//    Vector2 min;
//    Vector2 max;
//};
//
//void collision_init();
//void collision_update(Hero& hero, std::vector<Monster>& monsters);
//void collision_exit();
//
//// Utility functions
//AABB BuildSwordHitbox(const Hero& hero);
//AABB BuildHeroHitbox(const Hero& hero);
//AABB BuildMonsterHitbox(const Monster& monster);
//bool CheckAABBCollision(const AABB& a, const AABB& b);
//bool CheckDistanceCollision(const Vector2& a, float aradius, const Vector2& b, float bradius);
//
//// Box-to-Line Collision System
//enum class LineHitResult { None, Bad, Good, Excellent };
//
//// Check if a box (AABB) collides with a horizontal line
//// lineY: y-coordinate of the line
//// lineStartX, lineEndX: x-range of the line
//LineHitResult CheckBoxLineCollision(const AABB& box, float lineY, float lineStartX, float lineEndX);

#endif