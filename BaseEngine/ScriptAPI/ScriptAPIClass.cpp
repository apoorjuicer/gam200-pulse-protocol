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