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