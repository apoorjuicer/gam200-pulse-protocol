#pragma once
#include "pch/pch.h"


class SystemManager
{
public:
	template<typename T>
	std::shared_ptr<T> RegisteringSystem()
	{
		// typeid operator returns a constant reference to template of type, exp: Graphics
		std::string SystemTypeName = typeid(T).name();

		// make sure we dont hv duplicated systems
		assert(MappedSystems.find(SystemTypeName) == MappedSystems.end() && "System has been registered more than once!");

		// make a new instance of the system:
		// std::make_shared<T>(): calls T constructor and wrapped into shared_ptr
		auto system = std::make_shared<T>();

		// Now that we have the name of the system and the pointer to the system, we insert it into our map
		// With the name of the system as key and access to the actual system as the "reward"
		MappedSystems.insert({ SystemTypeName, system });

		// Return the newly made system to be configurated/modified
		return system;
	}

	template<typename T>
	void SetSignature(Signature signature)
	{
		// grab system name first
		const char* SystemTypeName = typeid(T).name();

		// check if system has been made alr
		assert(MappedSystems.find(SystemTypeName) != MappedSystems.end() && "System has not been registered.");

		// map tha name as key and the bit signature as the "reward"
		MappedSignature.insert({ SystemTypeName, signature });
	}

	// Remove entity from all the system when the entity dies >:D
	void RemoveEntity(Entity entity)
	{
		// iterate thru all the systems we have
		for (auto const& pair : MappedSystems)
		{
			auto const& SystemInstance = pair.second;

			// remove entity from all systems it originally had. (one by one)
			SystemInstance->EntityMember.erase(entity);
		}
	}

	void EntitySignatureChanged(Entity entity, Signature entitySignature)
	{
		// iterate thru all the systems we have
		for (auto const& pair : MappedSystems)
		{
			auto const& type = pair.first;				// system name							
			auto const& SystemInstance = pair.second;	// system instance 

			// what its signature is(bit): 101001
			auto const& systemSignature = MappedSignature[type];

			// Using bit signature, if they match, then slot into the new system
			// EXP: Before[0011] Now[0010]. System1[0011] System2[0010]. Now entity is in system 2 instead
			if ((entitySignature & systemSignature) == systemSignature)
			{
				// add into the new system 
				SystemInstance->EntityMember.insert(entity);
			}
			else
			{
				// remove from current system its iterating thru
				SystemInstance->EntityMember.erase(entity);
			}
		}
	}

	// add one for all the system added already to console

private:
	// Map for system name and the bitwise(aka signatures)
	std::unordered_map<std::string, Signature> MappedSignature{};
	/*
	exp: MappedSignature = {{"Graphics", 0b00110},    // Needs Transform + render components
							{"Physics" , 0b01010},    // Needs Transform + position components
							{"Math"    , 0b10110}};   // Needs Transform + math components? idk what math there is to do XD
	*/

	// Same thing but now mapped for system name and pointer to system instance
	std::unordered_map<std::string, std::shared_ptr<Systems>> MappedSystems{};


};
