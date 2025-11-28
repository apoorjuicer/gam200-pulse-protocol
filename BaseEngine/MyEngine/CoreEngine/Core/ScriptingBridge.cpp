/******************************************************************************/
/**
 * @file        ScriptingBridge.cpp
 * @project     Pulse Protocol
 * @author		Chia Wei Xuan Rachael - 100%
 * @brief		Testing bridge between C++ and C# scripting.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
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
