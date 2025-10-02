/*********************************************************************************************
 \file      Isystem.h
 \par		Cat Feeders
 \author	Michelle Han Jian Ping (m.han@digipen.edu) 100%


\copyright All content   2024 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/
#pragma once //Makes sure this header is only included once

#include "Message.h"
#include <string>

namespace Editor
{
	///System is a pure virtual base class (which is to say, an interface) that is
	///the base class for all systems used by the game. 
	class ISystem
	{
	public:
		///Systems can receive all message send to the Core. 
		///See Message.h for details.
		//virtual void SendMessage(Message* message) { UNREFERENCED_PARAMETER(message); };

		///All systems are updated every game frame.
		virtual void Update(float timeslice) = 0;

		///All systems provide a string name for debugging.
		virtual std::string GetName() = 0;

		///Initialize the system.
		virtual void Initialize() {};

		///All systems need a virtual destructor to have their destructor called 
		virtual ~ISystem() {}
	};
}
