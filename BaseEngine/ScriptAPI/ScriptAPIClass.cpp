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
//#include "..//ScriptAPI/ScriptAPIClass.h"

#include "ScriptAPIClass.h"
#include "TransformComponent.h"
#include "Health.h"
#include "Animation.h"
#include "ScriptName.h"
#include "Collision.h"
#include "InputAPI.h"
#include "Sprite.h"

namespace ScriptAPI
{
    TransformComponent Script::GetTransform()
    {
        // Gib the Transform Component our entity ID so ECS and script are using same Entiy ID
        return TransformComponent(EntityID);
    }

    HealthComponent Script::GetHealth()
    {
        return HealthComponent(EntityID);
    }

    Animation Script::GetAnimation()
    {
        return Animation(EntityID);
    }

    CollisionComponent Script::GetCollision()
    {
        return CollisionComponent(EntityID);
    }

    InputComponent Script::GetInput()
    {
        return InputComponent(EntityID);
    }

    AudioComponent Script::GetAudio()
    {
        return AudioComponent(EntityID);
    }

    SpriteComponent Script::GetSprite()
    {
        return SpriteComponent(EntityID);
    }

    /*LogicComponent Script::GetScriptName()
    {
        return LogicComponent(EntityID);
    }*/

   /* DeltaTime Script::GetDeltaTime()
    {
        return DeltaTime(dt);
    }*/
}