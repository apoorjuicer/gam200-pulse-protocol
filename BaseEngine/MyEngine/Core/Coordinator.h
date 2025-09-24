#pragma once
#include "pch/pch.h"
#include "SystemManager.h"
#include "EntityManager.h"
#include "ComponentManager.h"
#include "ComponentArray.h"

//  Acts as a mediator that allows us to have a single instance of the coordinator to use for all 3 manager :D
class Coordinator
{
public:	
	void init()
	{
		// Create pointers to each manager
		CoordEnitiyManager = std::make_unique<EntityManager>();
		CoordComponentManager = std::make_unique<ComponentManager>();
		CoordSystemManager = std::make_unique<SystemManager>();
	}

	// Shortcuts for create/kill entity 
	Entity CreateEntity()
	{
		return CoordEnitiyManager -> CreateEntity();	
	}

	void DestroyEntity(Entity entity)
	{
		CoordEnitiyManager->RemoveEntity(entity);

		CoordComponentManager->RemoveEntity(entity);

		CoordSystemManager->RemoveEntity(entity);
	}

	// Shortcuts for Component methods
	template<typename T>
	void RegisterComponent()
	{
		CoordComponentManager->RegisterComponents<T>();
	}

	template<typename T>
	void AddComponent(Entity entity, T component)
	{
		CoordComponentManager->AddComponent<T>(entity, component);

		auto signature = CoordEnitiyManager->GetSignature(entity);
		signature.set(CoordComponentManager->GetComponentType<T>(), true);

		CoordEnitiyManager->SetSignature(entity, signature);
		CoordSystemManager->EntitySignatureChanged(entity, signature);
	}

	template<typename T>
	void RemoveComponent(Entity entity)
	{
		CoordComponentManager->RemoveComponent<T>(entity);

		auto signature = CoordEnitiyManager->GetSignature(entity);
		signature.set(CoordComponentManager->GetComponentType<T>(), false);

		CoordEnitiyManager->SetSignature(entity, signature);
		CoordSystemManager->EntitySignatureChanged(entity, signature);
	}

	template<typename T>
	T& GetComponent(Entity entity)
	{
		return CoordComponentManager->GetComponent<T>(entity);
	}

	template<typename T>
	ComponentType GetComponentType()
	{
		return CoordComponentManager->GetComponentType<T>();
	}

	template<typename T>
	std::shared_ptr<T> GetSystem()
	{
		return CoordSystemManager->GetSystem<T>();
	}

	template<typename T>
	std::shared_ptr<T> RegisterSystem()
	{
		return CoordSystemManager->RegisteringSystem<T>();
	}

	template<typename T>
	void SetSystemSignature(Signature signature)
	{
		CoordSystemManager->SetSignature<T>(signature);
	}

	void UpdateAllSystems(float dt)
	{
		CoordSystemManager->UpdateAllSystems(dt);
	}

	void InitializeAllSystems()
	{
		CoordSystemManager->InitializeAllSystems();
	}

	template<typename T>
	bool HasComponent(Entity entity) const {
		auto sig = CoordEnitiyManager->GetSignature(entity);
		auto type = CoordComponentManager->GetComponentType<T>();
		return sig.test(type);
	}

	template<typename T>
	T* TryGetComponent(Entity entity) {
		if (!HasComponent<T>(entity)) return nullptr;
		return &CoordComponentManager->GetComponent<T>(entity);
	}
private:
	std::unique_ptr<EntityManager> CoordEnitiyManager;
	std::unique_ptr<ComponentManager> CoordComponentManager;
	std::unique_ptr<SystemManager> CoordSystemManager;
};

