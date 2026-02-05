/******************************************************************************/
/**
 * @file        StateMachine.h
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

#pragma once
#include <pch/pch_temp.h>
#include <cstdint>
#include <stdexcept>

//Serialize
#pragma warning(push, 0)
#include <rapidjson/document.h>     // for Document, Value, AllocatorType
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h> // for StringBuffer
#pragma warning(pop)


//for .NET
#include "CoreEngine/Core/ImportExport.h" // for DLL_API
#pragma warning(push)
#pragma warning(disable: 4251)

// -------------------- FSM State --------------------
enum class FSMState : uint8_t
{
    Idle = 0,
    Moving,
    Attack,
    Damage,
    Death
};

// -------------------- FSM Context (NOT a component) --------------------
// This is per-entity runtime context used by the FSM logic.
// We DO NOT store this inside the ECS component (FSM) because you want FSM
// to remain minimal for JSON (CurrentState only).
struct FSMContext
{
    bool wantsMove = false;  // set continuously while input indicates movement
    bool wantsAttack = false;  // one-frame request
    bool tookDamage = false;  // one-frame request
    bool isDead = false;  // derived from Health

    float attackTimer = 0.0f;  // owned by FSM
    float damageTimer = 0.0f;  // owned by FSM
};

// -------------------- ECS Component: FSM --------------------
// This IS the ECS component that you register in CoreEngine.cpp:
//   g_coordinator->RegisterComponent<FSM>();
// JSON stores CurrentState.
struct DLL_API FSM
{
    FSMState CurrentState = FSMState::Idle;

    void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const
    {
        out.SetObject();
        out.AddMember("CurrentState", static_cast<uint8_t>(CurrentState), alloc);
    }

    void Deserialize(const rapidjson::Value& in)
    {
        if (in.HasMember("CurrentState") && in["CurrentState"].IsUint())
        {
            CurrentState = static_cast<FSMState>(in["CurrentState"].GetUint());
        }
        else
        {
            // If JSON doesn't contain it, keep default Idle
            CurrentState = FSMState::Idle;
        }
    }
};

// -------------------- FSM Logic --------------------
// StateMachine is a stateless helper that updates (CurrentState + Context).
class StateMachine
{
public:
    // Updates the current state based on the context each frame.
    // - state lives in ECS component: FSM.CurrentState
    // - ctx lives in GameplayFSMSystem (per entity)
    static void Update(FSMState& state, FSMContext& ctx, float dt);

private:
    static void EnterState(FSMState next, FSMState& state, FSMContext& ctx);

    static void UpdateIdle(FSMState& state, FSMContext& ctx, float dt);
    static void UpdateMoving(FSMState& state, FSMContext& ctx, float dt);
    static void UpdateAttack(FSMState& state, FSMContext& ctx, float dt);
    static void UpdateDamage(FSMState& state, FSMContext& ctx, float dt);
    static void UpdateDeath(FSMState& state, FSMContext& ctx, float dt);
};

#pragma warning(pop)