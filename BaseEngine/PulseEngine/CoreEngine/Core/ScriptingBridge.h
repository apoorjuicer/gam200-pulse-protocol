/******************************************************************************/
/**
 * @file        ScriptingBridge.h
 * @project     Pulse Protocol
 * @author		Chia Wei Xuan Rachael - 100%
 * @brief		Testing bridge between C++ and C# scripting.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#pragma once
#include "CoreEngine/Core/ImportExport.h"

class CoreEngine;
namespace Framework { class Transform; }

class DLL_API ScriptBridge
{
public:
	static Framework::Transform* GetTransformComponent(int ID);
	void CallHelloWorld(CoreEngine* engine);
};