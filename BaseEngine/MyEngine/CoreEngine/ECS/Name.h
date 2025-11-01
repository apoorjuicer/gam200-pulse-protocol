/******************************************************************************/
/**
 * @file        Name.h
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Simple Name component for ECS entities to store entity names
                (Simpler words; display name each game object in the hierarchy panel)
 *              for identification and debugging purposes in the hierarchy.
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