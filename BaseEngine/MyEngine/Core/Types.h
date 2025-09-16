#pragma once
#include "pch/pch.h"



// ECS
// JUST KNOW THAT ENTITY IS JUST A NUMBER. (1,2,3,4,5)
using Entity = std::uint32_t;

// Used to define the size of our container later on
const Entity MaxEntity = 5000;

// 8 type if component tyoe for now
using ComponentType = std::uint8_t;

// max amount of component per type
const ComponentType MAX_COMPONENTS = 32;

// for System ID
using Signature = std::bitset<MAX_COMPONENTS>;



