#pragma once

#ifndef PCH_H
#define PCH_H

// pch.h: This is a precompiled header file.
// I'm doing this as we only need to compile it once and make  the files neater, we will put all our lib that we are using here. 
// Just put #include "pch/pch.h" and you will have acces to it

// Standard Library Headers
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <memory>		 // for std::shared_ptr
#include <any>
#include <set>			
#include <cassert>		 // for assertion
#include <cstdint>		 // for uint32
#include <bitset>		 // for System ID/signature
#include <array>		 // for std::array
#include <vector>		 // for std::vectors
#include <deque>		 // for std::deque
#include <unordered_map> // for std::unordered_map



// Graphics Library Headers
#include <GL/glew.h>
#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>

// System Library Headers
#include "../MyEngine/GSM/gsm.h"
#include "../MyEngine/ECS/ecs.h"
#include "../MyEngine/Math/math.h"
#include "../MyEngine/Systems/System.h"
#include "../MyEngine/Input/InputManager.h"
#include "../MyEngine/Physics/collision.h"
#include "../MyEngine/Time/Time_Manager.h"
#include "../MyEngine/CoreEngine/CoreEngine.h"

// ECS
#include "../MyEngine/Core/Types.h"
#include "../MyEngine/Core/Window.h"
#include "../MyEngine/Core/System.h"
#include "../MyEngine/Core/Assets.h"
#include "../MyEngine/Core/Factory.h"
#include "../MyEngine/Core/Coordinator.h"
#include "../MyEngine/Core/SystemManager.h"
#include "../MyEngine/Core/EntityManager.h"
#include "../MyEngine/Core/ComponentArray.h"
#include "../MyEngine/Core/ComponentManager.h"




#endif //PCH_H