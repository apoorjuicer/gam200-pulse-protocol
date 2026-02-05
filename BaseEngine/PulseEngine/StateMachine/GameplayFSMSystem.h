/******************************************************************************/
/**
 * @file        GameplayFSMSystem.h
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

#include <unordered_map>
#include "../CoreEngine/Core/ImportExport.h"
#include "../CoreEngine/ECS/System.h"
#include "../CoreEngine/ECS/Types.h"
#include "StateMachine.h" // FSMState, FSMContext, FSM component struct, StateMachine

class GameplayFSMSystem : public Systems
{
public:
    void Init() override {}
    void Update(float dt) override;

    // Export these so ScriptAPI.dll can link against PulseEngine symbols.
    // NOTE: Signature must match EXACTLY between .h and .cpp
    static DLL_API void RequestMove(Entity e, bool moving);
    static DLL_API void RequestAttack(Entity e);
    static DLL_API void NotifyDamage(Entity e);

private:
    static std::unordered_map<Entity, FSMContext> s_ctx;
};