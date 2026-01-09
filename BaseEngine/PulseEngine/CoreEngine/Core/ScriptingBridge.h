/******************************************************************************/
/**
 * @file        ScriptingBridge.h
 * @project     Pulse Protocol
 * @author		Chia Wei Xuan Rachael - 100%
 * @brief		Bridge between C++ and C# scripting.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#pragma once
#include "CoreEngine/Core/ImportExport.h"

class CoreEngine;
class Health;
class InputManager;
class LogicComponent;
namespace Framework { class Transform; }

class DLL_API ScriptBridge
{
public:
	static Health* GetHealthComponent(int ID);
	static InputManager* GetInputKeys(int ID);
	static LogicComponent* GetLogicComponent(int ID);
	static Framework::Transform* GetTransformComponent(int ID);
	
	void CallHelloWorld(CoreEngine* engine);
};