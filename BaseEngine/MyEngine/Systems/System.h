#include "pch/pch.h"




// System is a pure virtual base class (which is to say, an interface) that is the base class for all systems used by the game. 
class System
{
public:
		
	// Initialize the system.
	virtual void Init() {};

	// All systems need a virtual destructor to have their destructor called 
	virtual ~System() {}

	// All systems are updated every game frame.
	virtual void Update(float timeslice) = 0;

	// All systems provide a string name for debugging.
	virtual std::string GetName() = 0;
		
};






