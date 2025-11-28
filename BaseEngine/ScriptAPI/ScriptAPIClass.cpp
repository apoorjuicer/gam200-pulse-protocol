/******************************************************************************/
/**
* @file        ScriptAPIClass.cpp
* @project     Pulse Protocol
* @author      Chia Wei Xuan Rachael - 100%
* @brief       Provides the C++/CLI bridge with Script class that all C# game scripts inherit from, 
               managing entity-script associations and exposing component access methods to managed code.
*
* @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/
#include "ScriptAPIClass.h"
#include "TransformComponent.h"

namespace ScriptAPI
{
    TransformComponent Script::GetTransform()
    {
        // Gib the Transform Component our entity ID so ECS and script are using same Entiy ID
        return TransformComponent(EntityID);
    }
}