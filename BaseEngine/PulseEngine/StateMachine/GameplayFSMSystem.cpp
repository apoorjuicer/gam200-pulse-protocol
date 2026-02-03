/******************************************************************************/
/**
 * @file        GameplayFSMSystem.cpp
 * @project     Pulse Protocol
 * @author      Reginald Lew Yee Ren (primary)
 * @brief
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without
 *              the prior written consent of DigiPen Institute of Technology is
 *              prohibited.
 */
/******************************************************************************/

#include "GameplayFSMSystem.h"
#include "../CoreEngine/ECS/Coordinator.h"
#include "../Components/Health.h"
#include "Graphics/Transform.h"

void GameplayFSMSystem::Update(float dt)
{
    auto g = Coordinator::GetInstance();

    // IMPORTANT: in your ECS, Systems stores entities in EntityMember
    for (Entity e : EntityMember)
    {
        if (!g->HasComponent<FSMComponent>(e) ||
            !g->HasComponent<Health>(e))
            continue;

        auto& fsm = g->GetComponent<FSMComponent>(e);
        auto& hp = g->GetComponent<Health>(e);

		FSMContext* fsmcontext = nullptr;
        // Update ctx signals
        fsmcontext->isDead = (hp.hp <= 0) || !hp.isAlive; // adjust to your Health fields

        // NOTE: these should be set by your input/gameplay pipeline:
        /*fsm.ctx.wantsMove = true;
        fsm.ctx.wantsAttack = false;
        fsm.ctx.tookDamage = false;*/

        // Run FSM
        FSMContext ctx;
		StateMachine* statemachine = nullptr;
        statemachine->Update(ctx, dt);

        // Optionally: you can react engine-side to state changes here
        // e.g. lock movement during Attack/Damage, etc.
        /*static FSMState lastState = FSMState::Idle;
        std::cout << "[FSM] Updating entity " << e << "\n";
        FSMState current = fsm.machine.GetState();
        if (current != lastState)
        {
            std::cout << "[FSM] Updating entity " << e << "\n";
            std::cout << "[FSM] Entity " << e
                << " State changed to " << static_cast<int>(current)
                << std::endl;
            lastState = current;
        }*/
    }
}
