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

enum class FSMState : uint8_t
{
    Idle = 0,
    Moving,
    Attack,
    Damage,
    Death
};

// “Context” is the information the FSM needs to decide transitions.
// You can expand this later as your gameplay grows.
struct FSMContext
{
    bool wantsMove = false;      // e.g. input indicates movement
    bool wantsAttack = false;    // e.g. rhythm input success
    bool tookDamage = false;     // set by collision/damage pipeline
    bool isDead = false;         // hp <= 0

    // Timers (owned by FSM)
    float attackTimer = 0.0f;
    float damageTimer = 0.0f;
};

class StateMachine
{
public:
    void SetState(FSMState s);
    FSMState GetState() const { return m_state; }

    // Call once per frame
    void Update(FSMContext& ctx, float dt);

private:
    FSMState m_state = FSMState::Idle;

    // helpers for transitions
    void EnterState(FSMState next, FSMContext& ctx);
    void UpdateIdle(FSMContext& ctx, float dt);
    void UpdateMoving(FSMContext& ctx, float dt);
    void UpdateAttack(FSMContext& ctx, float dt);
    void UpdateDamage(FSMContext& ctx, float dt);
    void UpdateDeath(FSMContext& ctx, float dt);
};



struct DLL_API FSM
{
    FSMState CurrentState;

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
    }
};
