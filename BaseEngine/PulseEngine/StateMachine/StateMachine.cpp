/******************************************************************************/
/**
 * @file        StateMachine.cpp
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

#include "StateMachine.h"

// Tune these later to match your rhythm windows / hit-stun feel
static constexpr float ATTACK_DURATION = 0.25f;
static constexpr float DAMAGE_STUN = 0.35f;

void StateMachine::Update(FSMState& state, FSMContext& ctx, float dt)
{
    // Highest priority: Death overrides everything
    if (ctx.isDead)
    {
        if (state != FSMState::Death)
            EnterState(FSMState::Death, state, ctx);

        UpdateDeath(state, ctx, dt);
        return;
    }

    // Damage can override most states (except Death)
    if (ctx.tookDamage && state != FSMState::Damage)
    {
        EnterState(FSMState::Damage, state, ctx);
        // Continue into damage update this frame
    }

    switch (state)
    {
    case FSMState::Idle:   UpdateIdle(state, ctx, dt);   break;
    case FSMState::Moving: UpdateMoving(state, ctx, dt); break;
    case FSMState::Attack: UpdateAttack(state, ctx, dt); break;
    case FSMState::Damage: UpdateDamage(state, ctx, dt); break;
    case FSMState::Death:  UpdateDeath(state, ctx, dt);  break;
    default:
        // Safety fallback
        EnterState(FSMState::Idle, state, ctx);
        break;
    }
}

void StateMachine::EnterState(FSMState next, FSMState& state, FSMContext& ctx)
{
    state = next;

    // Reset timers on entering timed states
    if (next == FSMState::Attack)
        ctx.attackTimer = ATTACK_DURATION;

    if (next == FSMState::Damage)
        ctx.damageTimer = DAMAGE_STUN;
}

void StateMachine::UpdateIdle(FSMState& state, FSMContext& ctx, float /*dt*/)
{
    if (ctx.tookDamage)
    {
        EnterState(FSMState::Damage, state, ctx);
        return;
    }

    if (ctx.wantsAttack)
    {
        EnterState(FSMState::Attack, state, ctx);
        return;
    }

    if (ctx.wantsMove)
    {
        EnterState(FSMState::Moving, state, ctx);
        return;
    }
}

void StateMachine::UpdateMoving(FSMState& state, FSMContext& ctx, float /*dt*/)
{
    if (ctx.tookDamage)
    {
        EnterState(FSMState::Damage, state, ctx);
        return;
    }

    if (ctx.wantsAttack)
    {
        EnterState(FSMState::Attack, state, ctx);
        return;
    }

    if (!ctx.wantsMove)
    {
        EnterState(FSMState::Idle, state, ctx);
        return;
    }
}

void StateMachine::UpdateAttack(FSMState& state, FSMContext& ctx, float dt)
{
    // Optional: allow damage to interrupt attack
    if (ctx.tookDamage)
    {
        EnterState(FSMState::Damage, state, ctx);
        return;
    }

    ctx.attackTimer -= dt;
    if (ctx.attackTimer <= 0.0f)
    {
        // Return to Moving if still moving, else Idle
        EnterState(ctx.wantsMove ? FSMState::Moving : FSMState::Idle, state, ctx);
        return;
    }
}

void StateMachine::UpdateDamage(FSMState& state, FSMContext& ctx, float dt)
{
    ctx.damageTimer -= dt;
    if (ctx.damageTimer <= 0.0f)
    {
        // Return to Moving if still moving, else Idle
        EnterState(ctx.wantsMove ? FSMState::Moving : FSMState::Idle, state, ctx);
        return;
    }
}

void StateMachine::UpdateDeath(FSMState& /*state*/, FSMContext& /*ctx*/, float /*dt*/)
{
    // Remain in Death until gameplay respawn logic changes it.
}