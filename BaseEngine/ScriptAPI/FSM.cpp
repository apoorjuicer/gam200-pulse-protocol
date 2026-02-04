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
#include "../StateMachine/StateMachine.h"

#undef generic

namespace ScriptAPI
{
    FSMComponentAPI::FSMComponentAPI(unsigned int ID) : entityID(ID) {}

    // Helper: safely get native FSMComponent
    /*static bool TryGetFSM(unsigned int id, FSMComponent*& outFSM)
    {
        auto g = Coordinator::GetInstance();
        if (!g->HasComponent<FSMComponent>(id))
            return false;

        outFSM = &g->GetComponent<FSMComponent>(id);
        return true;
    }*/

    int FSMComponentAPI::GetState()
    {
        ::StateMachine fsm ;
        //if (!TryGetFSM(entityID, fsm))
        //    return -1; // entity has no FSMComponent

        return static_cast<int>(fsm.GetState());
    }

    void FSMComponentAPI::RequestMove(bool moving)
    {
        ::FSMContext fsm ;
        /*if (!TryGetFSM(entityID, fsm))
            return;*/

        fsm.wantsMove = moving;
    }

    void FSMComponentAPI::RequestAttack()
    {
        ::FSMContext fsm;
        /*if (!TryGetFSM(entityID, fsm))
            return;*/

        // one-frame request; your FSM Update() should clear it after consuming
        fsm.wantsAttack = true;
    }

    void FSMComponentAPI::NotifyDamage()
    {
        FSMContext fsm;
        /*if (!TryGetFSM(entityID, fsm))
            return;*/

        // one-frame flag; FSM Update() should clear it after consuming
        fsm.tookDamage = true;
    }

    void FSMComponentAPI::ClearRequests()
    {
        FSMContext fsm;
        /*if (!TryGetFSM(entityID, fsm))
            return;*/

        fsm.wantsMove = false;
        fsm.wantsAttack = false;
        fsm.tookDamage = false;
    }
}