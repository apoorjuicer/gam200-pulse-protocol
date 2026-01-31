/******************************************************************************/
/**
* @file        FSM.h
* @project     Pulse Protocol
* @author      Reginald - 100%
* @brief       C++/CLI ScriptAPI wrapper for engine-side FSM (StateMachine).
*
* @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/
#pragma once

namespace ScriptAPI
{
    // Scripts can do: GetFSM().RequestAttack();
    public value struct FSMComponentAPI
    {
    public:
        // Read current state (Idle/Moving/Attack/Damage/Death)
        int GetState();

        // Requests/events sent to the engine-side FSM (flags only)
        void RequestMove(bool moving);
        void RequestAttack();
        void NotifyDamage();

        // Optional convenience: force-clear one-frame flags (not required)
        void ClearRequests();

    internal:
        FSMComponentAPI(unsigned int ID);

    private:
        unsigned int entityID;
    };
}