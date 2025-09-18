#pragma once

#ifndef PCH_H
#define PCH_H

// pch.h: This is a precompiled header file.
// I'm doing this as we only need to compile it once and make  the files neater, we will put all our lib that we are using here. 
// Just put #include "pch/pch.h" and you will have acces to it

// Standard Library Headers
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include <cassert> //debug throw assertion
#include <iomanip> //debug
#include <algorithm>
#include <random>
#include <array>

// Graphics Library Headers
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Suppress APIENTRY macro redefinition warning
#pragma warning(push)
#pragma warning(disable: 4005)

// System Library Headers
#include <../MyEngine/GameState/gsm.h>
#include <../MyEngine/ECS/ecs.h>
#include <../MyEngine/Math/math.h>
#include <../MyEngine/Input/InputManager.h>
#include <../MyEngine/Physics/collision.h>
#include <../MyEngine/Time/Time_Manager.h>
#include <../MyEngine/CoreEngine/CoreEngine.h>
#include <../MyEngine/Resources/ResourceManager.h>

// Restore warning settings
#pragma warning(pop)

#endif //PCH_H