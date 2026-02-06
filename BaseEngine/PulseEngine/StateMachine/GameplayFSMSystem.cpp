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

std::unordered_map<Entity, FSMContext> GameplayFSMSystem::s_ctx;

void GameplayFSMSystem::RequestMove(Entity e, bool moving)
{
    s_ctx[e].wantsMove = moving;
}

void GameplayFSMSystem::RequestAttack(Entity e)
{
    // one-frame request (cleared after consuming in Update)
    s_ctx[e].wantsAttack = true;
}

void GameplayFSMSystem::NotifyDamage(Entity e)
{
    // one-frame request (cleared after consuming in Update)
    s_ctx[e].tookDamage = true;
}

void GameplayFSMSystem::Update(float dt)
{
    auto g = Coordinator::GetInstance();

    for (Entity e : EntityMember)
    {
        if (!g->HasComponent<FSM>(e) ||
            !g->HasComponent<Health>(e) ||
            !g->HasComponent<Framework::Transform>(e))
        {
            continue;
        }

        auto& fsm = g->GetComponent<FSM>(e);
        auto& hp = g->GetComponent<Health>(e);

        FSMContext& ctx = s_ctx[e];

        // IMPORTANT: no null pointers
        ctx.isDead = (hp.hp <= 0) || !hp.isAlive;

        // Update serialized ECS state
        StateMachine::Update(fsm.CurrentState, ctx, dt);

        // Clear one-frame requests
        ctx.wantsAttack = false;
        ctx.tookDamage = false;
    }
}
