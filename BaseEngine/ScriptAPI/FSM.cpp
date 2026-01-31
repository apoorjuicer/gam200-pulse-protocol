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
#include "../StateMachine/FSMComponent.h"

#undef generic

namespace ScriptAPI
{
    FSMComponentAPI::FSMComponentAPI(unsigned int ID) : entityID(ID) {}

    // Helper: safely get native FSMComponent
    static bool TryGetFSM(unsigned int id, FSMComponent*& outFSM)
    {
        auto g = Coordinator::GetInstance();
        if (!g->HasComponent<FSMComponent>(id))
            return false;

        outFSM = &g->GetComponent<FSMComponent>(id);
        return true;
    }

    int FSMComponentAPI::GetState()
    {
        FSMComponent* fsm = nullptr;
        if (!TryGetFSM(entityID, fsm))
            return -1; // entity has no FSMComponent

        return static_cast<int>(fsm->machine.GetState());
    }

    void FSMComponentAPI::RequestMove(bool moving)
    {
        FSMComponent* fsm = nullptr;
        if (!TryGetFSM(entityID, fsm))
            return;

        fsm->ctx.wantsMove = moving;
    }

    void FSMComponentAPI::RequestAttack()
    {
        FSMComponent* fsm = nullptr;
        if (!TryGetFSM(entityID, fsm))
            return;

        // one-frame request; your FSM Update() should clear it after consuming
        fsm->ctx.wantsAttack = true;
    }

    void FSMComponentAPI::NotifyDamage()
    {
        FSMComponent* fsm = nullptr;
        if (!TryGetFSM(entityID, fsm))
            return;

        // one-frame flag; FSM Update() should clear it after consuming
        fsm->ctx.tookDamage = true;
    }

    void FSMComponentAPI::ClearRequests()
    {
        FSMComponent* fsm = nullptr;
        if (!TryGetFSM(entityID, fsm))
            return;

        fsm->ctx.wantsMove = false;
        fsm->ctx.wantsAttack = false;
        fsm->ctx.tookDamage = false;
    }
}