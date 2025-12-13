/******************************************************************************/
/**
 * @file        DisplayName.h
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Defines the Name component for ECS entities. Provides a simple
 *              string-based identifier for entities with JSON serialization
 *              support, used primarily for hierarchy display and debugging.
 * 
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#pragma once
#include "pch/pch_temp.h"

struct Name {
    std::string name;

    // Serialize to JSON
    void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const
    {
        out.SetObject();
        rapidjson::Value nameVal;
        nameVal.SetString(name.c_str(), static_cast<rapidjson::SizeType>(name.length()), alloc);
        out.AddMember("name", nameVal, alloc);
    }

    // Deserialize from JSON
    void Deserialize(const rapidjson::Value& in)
    {
        if (!in.IsObject())
            return;

        if (in.HasMember("name") && in["name"].IsString())
            name = in["name"].GetString();
    }
};