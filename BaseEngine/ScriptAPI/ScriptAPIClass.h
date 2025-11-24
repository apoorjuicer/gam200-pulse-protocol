#pragma once
#include "TransformComponent.h"

namespace ScriptAPI
{
    //forward delcaration
    value struct TransformComponent;

    public ref class Script abstract
    {
    public:
        void virtual Update() {};

        TransformComponent GetTransform();

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