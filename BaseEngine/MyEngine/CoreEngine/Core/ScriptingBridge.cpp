#include "CoreEngine.h"
#include "ScriptingBridge.h"
#include "../MyEngine/Graphics/Transform.h"
#include "../MyEngine/CoreEngine/ECS/Coordinator.h"

// ONLY FOR GRAPHICS SINCE HEADER FILE HAS FUNCTIONS AND COMP. 
Framework::Transform* ScriptBridge::GetTransformComponent(int ID)
{
	auto g_coordinator = Coordinator::GetInstance();
	return &g_coordinator->GetComponent <Framework::Transform>(ID);
}
