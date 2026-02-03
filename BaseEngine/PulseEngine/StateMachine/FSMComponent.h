/******************************************************************************/
/**
 * @file        FSMComponent.h
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
#include "StateMachine.h"

struct FSMComponent
{
    /*StateMachine machine;
    FSMContext   ctx;*/
    FSMState  States;

    // Requests (set by input/scripts; consumed by FSM each frame)
    // You can set these directly from your input system or ScriptAPI.
};
