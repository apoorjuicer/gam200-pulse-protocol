/******************************************************************************/
/**
* @file        ScriptAPIClass.h
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
#pragma once
#include "TransformComponent.h"
#include "Health.h"
#include "DeltaTime.h"
#include "InputAPI.h"
#include "Collision.h"
#include "AudioAPI.h"

namespace ScriptAPI
{
    //forward delcaration
    value struct TransformComponent;
    value struct HealthComponent;
    value struct Factory;
    value struct Animation;
    // value struct DeltaTime 
    value struct CollisionComponent;
    value struct AudioComponent;
   // value struct DeltaTime 
    //value struct LogicComponent;

    public ref class Script abstract
    {
    public:
        virtual void Start() {}
        void virtual Update() {};

        TransformComponent GetTransform();
        HealthComponent GetHealth();
        Animation GetAnimation();
        InputComponent GetInput();
        CollisionComponent GetCollision();
        AudioComponent GetAudio();
        //DeltaTime GetDeltaTime();
        //LogicComponent GetScriptName();

    internal:
        // Store which entity this script is attached too
        void SetEntityID(int ID)
        {
            EntityID = ID;
        }
        
    protected:
        property int entityID
        {
            int get()
            {
                return EntityID;
            }
        }

    private:
        int EntityID;
    };
}