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
#include "../CoreEngine/ECS/System.h" 
#include "StateMachine.h"
#include "FSMComponent.h"

class GameplayFSMSystem : public Systems
{
public:
    void Init() override {}
    void Update(float dt) override;
};
