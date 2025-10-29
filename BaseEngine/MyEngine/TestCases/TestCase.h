/*****************************************************************************/
/**
 * @file        TestCase.h
 * @project     Pulse Protocol
 * @author      Reginald Lew Yee Ren
 * @brief       Handles gameplay test logic, including spawning entities,
 *              performing test cases, scaling, rotation, movement,
 *              and calling collision utilities from CollisionManager.
 */
 /*****************************************************************************/
#ifndef TESTCASE_H
#define TESTCASE_H

#include "../CoreEngine/Assets.h"
#include "pch/pch.h"

class TestCase : public Systems
{
public:
    void Init() override;
    void Update(float dt) override;

private:
    CollisionManager collision;              // for hitbox and collision utility
    std::vector<Entity> EntityNPC;           // hero and monsters
    bool monsterCanMove = false;
};

#endif // TESTCASE_H
