#pragma once
#define generic generic_workaround
#include "TransformComponent.h"
#include "../MyEngine/Graphics/Transform.h"
#include "../MyEngine/CoreEngine/Core/ScriptingBridge.h"
//#include "../MyEngine/CoreEngine/Core/CoreEngine.h"
//#include "../MyEngine/CoreEngine/ECS/Coordinator.h"
#undef generic

namespace Framework { class Transform; } // forward delare

namespace ScriptAPI
{
	// constrcutro for each instance, Param: ECS entity ID. We make both ECS and script system use same ID.
	TransformComponent::TransformComponent(unsigned int ID): entityID(ID) {}

	//Scale accessor and modifer
	float TransformComponent::ScaleX::get()
	{
		/*auto g_coordinator = Coordinator::GetInstance();
		auto& transform = g_coordinator->GetComponent<Framework::Transform>(entityID);*/
		auto* transform = CoreEngine::GetTransformComponent(entityID);
		return transform->Scale.x;
	}

	float TransformComponent::ScaleY::get()
	{
		auto* transform = CoreEngine::GetTransformComponent(entityID);
		return transform->Scale.y;
	}

	void TransformComponent::ScaleX::set(float value)
	{
		auto* transform = CoreEngine::GetTransformComponent(entityID);
		transform->Scale.x = value;
	}

	void TransformComponent::ScaleY::set(float value)
	{
		auto* transform = CoreEngine::GetTransformComponent(entityID);
		transform->Scale.y = value;
	}



	// Rotation acessor and modifer
	float TransformComponent::RotX::get()
	{
		auto* transform = CoreEngine::GetTransformComponent(entityID);
		return transform->Rotation.x;
	}

	void TransformComponent::RotX::set(float value)
	{
		auto* transform = CoreEngine::GetTransformComponent(entityID);
		transform->Rotation.x = value;
	}

	float TransformComponent::RotY::get()
	{
		auto* transform = CoreEngine::GetTransformComponent(entityID);
		return transform->Rotation.y;
	}

	void TransformComponent::RotY::set(float value)
	{
		auto* transform = CoreEngine::GetTransformComponent(entityID);
		transform->Rotation.y = value;
	}

	// Translate accessor and modifer 
	float TransformComponent::X::get()
	{
		auto* transform = CoreEngine::GetTransformComponent(entityID);
		return transform->Pos.x;
	}

	float TransformComponent::Y::get()
	{
		auto* transform = CoreEngine::GetTransformComponent(entityID);
		return transform->Pos.y;
	}

	void TransformComponent::X::set(float value)
	{
		auto* transform = CoreEngine::GetTransformComponent(entityID);
		transform->Pos.x = value;
	}

	void TransformComponent::Y::set(float value)
	{
		auto* transform = CoreEngine::GetTransformComponent(entityID);
		transform->Pos.y = value;
	}


}