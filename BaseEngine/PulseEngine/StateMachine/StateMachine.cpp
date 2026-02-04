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

static constexpr float ATTACK_DURATION = 0.25f; // tune later
static constexpr float DAMAGE_STUN = 0.35f; // tune later

void StateMachine::SetState(FSMState s)
{
    m_state = s;
}

void StateMachine::EnterState(FSMState next, FSMContext& ctx)
{
    m_state = next;

    // Initialize timers on enter
    if (next == FSMState::Attack)
        ctx.attackTimer = ATTACK_DURATION;

    if (next == FSMState::Damage)
        ctx.damageTimer = DAMAGE_STUN;
}

void StateMachine::Update(FSMContext& ctx, float dt)
{
    // Global priority transitions (these override everything)
    if (ctx.isDead && m_state != FSMState::Death)
    {
        EnterState(FSMState::Death, ctx);
        return;
    }

    if (ctx.tookDamage && m_state != FSMState::Damage && m_state != FSMState::Death)
    {
        EnterState(FSMState::Damage, ctx);
        return;
    }

    // State-specific updates
    switch (m_state)
    {
    case FSMState::Idle:   UpdateIdle(ctx, dt);   break;
    case FSMState::Moving: UpdateMoving(ctx, dt); break;
    case FSMState::Attack: UpdateAttack(ctx, dt); break;
    case FSMState::Damage: UpdateDamage(ctx, dt); break;
    case FSMState::Death:  UpdateDeath(ctx, dt);  break;
    default: break;
    }

    // Important: one-frame flags should be cleared AFTER update,
    // so they don’t keep triggering forever.
    ctx.wantsAttack = false;
    ctx.tookDamage = false;
}

void StateMachine::UpdateIdle(FSMContext& ctx, float /*dt*/)
{
    if (ctx.wantsAttack)
    {
        EnterState(FSMState::Attack, ctx);
        return;
    }

    if (ctx.wantsMove)
    {
        EnterState(FSMState::Moving, ctx);
        return;
    }

    // Otherwise stay Idle
}

void StateMachine::UpdateMoving(FSMContext& ctx, float /*dt*/)
{
    if (ctx.wantsAttack)
    {
        EnterState(FSMState::Attack, ctx);
        return;
    }

    if (!ctx.wantsMove)
    {
        EnterState(FSMState::Idle, ctx);
        return;
    }

    // Otherwise stay Moving
}

void StateMachine::UpdateAttack(FSMContext& ctx, float dt)
{
    ctx.attackTimer -= dt;
    if (ctx.attackTimer <= 0.0f)
    {
        // After attacking, return based on whether movement is held
        EnterState(ctx.wantsMove ? FSMState::Moving : FSMState::Idle, ctx);
        return;
    }

    // Otherwise stay Attack until timer ends
}

void StateMachine::UpdateDamage(FSMContext& ctx, float dt)
{
    ctx.damageTimer -= dt;
    if (ctx.damageTimer <= 0.0f)
    {
        // Recover based on whether movement is held
        EnterState(ctx.wantsMove ? FSMState::Moving : FSMState::Idle, ctx);
        return;
    }

    // Otherwise stay Damage until stun ends
}

void StateMachine::UpdateDeath(FSMContext& /*ctx*/, float /*dt*/)
{
    // No outgoing transitions
}
