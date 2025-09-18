#pragma once
#include "pch/pch.h"


class EntityManager
{
public:
	EntityManager()
	{
		for (Entity entity = 0; entity < MaxEntity; ++entity)
		{
			EntityList.push_back(entity);
		}
	}

	Entity CreateEntity()
	{
		// IN CASE AMOUNT OF ENTITY IS ABOVE WHAT IS SET(iirc its 32, in types.h)
		assert(LiveEntityCount < MaxEntity && "Too many entities in existence.");

		// Take an ID from the front of the queue
		Entity ID = EntityList.front();

		// remove it from queue
		EntityList.pop_back();
		++LiveEntityCount;

		return ID;
	}

	void RemoveEntity(Entity entity)
	{
		// make sure Entity ID is withint range
		assert(entity < MaxEntity && "Entity out of range.");

		// clear it 
		SignatureID[entity].reset();

		// make it so the Entity ID is now free for use 
		EntityList.push_back(entity);
		--LiveEntityCount;
	}

	void SetSignature(Entity entity, Signature signature)
	{
		assert(entity < MaxEntity && "Entity out of range.");

		// set entity to the signature
		SignatureID[entity] = signature;
	}

	Signature GetSignature(Entity entity)
	{
		assert(entity < MaxEntity && "Entity out of range.");

		// give entity systems bit signature
		return SignatureID[entity];
	}

	

private:
	std::deque<Entity> EntityList{}; //EntityList[i]
	std::array<Signature, MaxEntity> SignatureID{};
	unsigned int LiveEntityCount{};

};