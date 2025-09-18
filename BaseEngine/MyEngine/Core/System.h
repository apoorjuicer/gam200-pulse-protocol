#pragma once

#include "pch/pch.h"
#include <set>
#include "Types.h"
// Each system can inherit from this class which allows the System Manager
// to keep a list of pointers to systems.
// exp: auto const& entity : EntityMember
// auto& rigidBody = GetComponent<RigidBody>(entity);

// Will be our base class. Exp: Graphics, Physics...
class Systems
{
public:
	// functions that all of the system will have.


	// Only contains entities that have ALL the components this system needs
	// We do this using the signatures 
	std::set<Entity> EntityMember;
};




