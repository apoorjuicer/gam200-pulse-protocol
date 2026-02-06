/******************************************************************************/
/**
* @file        FSM.cpp
* @project     Pulse Protocol
* @author      Reginald - 100%
* @brief       C++/CLI ScriptAPI wrapper for engine-side FSM.
*
* @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/
#pragma once
#define generic generic_workaround

#include "FSM.h"
#include "../CoreEngine/ECS/Coordinator.h"
//#include "../StateMachine/FSMComponent.h"
#include "../StateMachine/StateMachine.h"
#include "../StateMachine/GameplayFSMSystem.h"

#undef generic

namespace ScriptAPI
{
    FSMComponentAPI::FSMComponentAPI(unsigned int ID) : entityID(ID) {}

    int FSMComponentAPI::GetState()
    {
        auto g = Coordinator::GetInstance();

        if (!g->HasComponent<FSM>(entityID))
            return -1;

        auto& fsm = g->GetComponent<FSM>(entityID);
        return static_cast<int>(fsm.CurrentState);
    }

    void FSMComponentAPI::RequestMove(bool moving)
    {
        GameplayFSMSystem::RequestMove(static_cast<Entity>(entityID), moving);
    }

    void FSMComponentAPI::RequestAttack()
    {
        GameplayFSMSystem::RequestAttack(static_cast<Entity>(entityID));
    }

    void FSMComponentAPI::NotifyDamage()
    {
        GameplayFSMSystem::NotifyDamage(static_cast<Entity>(entityID));
    }

    void FSMComponentAPI::ClearRequests()
    {
        // optional: stop movement request
        GameplayFSMSystem::RequestMove(static_cast<Entity>(entityID), false);
    }
}