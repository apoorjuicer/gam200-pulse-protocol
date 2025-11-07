/******************************************************************************/
/**
 * @file        ScritpName.h
 * @project     Pulse Protocol
 * @author		Chia Wei Xuan Rachael
 * @brief		Component for associating game logic scripts with entities via JSON serialization.
 *              Stores script identifier as a string to enable data-driven behavior attachment.
 *              Provides RapidJSON serialization/deserialization for script component persistence
 *              and standalone save/load utilities for script configuration management.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#pragma once
#include "pch/pch_temp.h"


//Serialize
#pragma warning(push, 0)
#include <rapidjson/document.h>     // for Document, Value, AllocatorType
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h> // for StringBuffer
#pragma warning(pop)

struct LogicComponent
{
    std::string ScriptName;

    void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const;

    void Deserialize(const rapidjson::Value& in);
};

bool SaveScriptName(LogicComponent const& t, std::string const& path);
bool LoadScriptName(LogicComponent& t, std::string const& path);
