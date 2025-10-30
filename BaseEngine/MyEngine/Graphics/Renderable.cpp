/******************************************************************************/
/**
 * @file        Renderable.cpp
 * @project     Pulse Protocol
 * @author      Goh Pin Kai (primary) - 50%
 * @author 		Ban Kai Wei Benjamin (secondary) - 50%
 * @brief       Implements the Renderable component, providing position,
 *              scale, rotation, animation, and sprite management. Includes
 *              update logic for entity transforms, matrix composition,
 *              and JSON-based serialization/deserialization for saving
 *              and loading transform state.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#include "Renderable.h"
#include <../MyEngine/Math/math.h>

namespace Framework
{
    void AssignTextureFromPath(Framework::Renderable &t)
    {
        if (t.textureID == 0 && !t.spriteName.empty())
            t.textureID = Framework::AssetsManager::GetOrLoad(t.spriteName);
    }

    /*Serialization && Deserialization */
    void Renderable::Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const
    {
        using namespace rapidjson;
        out.SetObject();

        out.AddMember("mdl_ref", mdl_ref, alloc);
        out.AddMember("shd_ref", shd_ref, alloc);
        out.AddMember("textureID", static_cast<uint64_t>(textureID), alloc);

        Value spriteNameVal;
        spriteNameVal.SetString(spriteName.c_str(), static_cast<SizeType>(spriteName.length()), alloc);
        out.AddMember("spriteName", spriteNameVal, alloc);
    }

    void Renderable::Deserialize(const rapidjson::Value& in)
    {
        if (!in.IsObject())
        {
            throw std::runtime_error("Invalid JSON for Transform");
        }

        if (in.HasMember("mdl_ref"))
            mdl_ref = in["mdl_ref"].GetFloat();
        if (in.HasMember("shd_ref"))
            shd_ref = in["shd_ref"].GetFloat();
        if (in.HasMember("textureID"))
            textureID = static_cast<GLuint>(in["textureID"].GetUint64());

        if (in.HasMember("spriteName"))
            spriteName = in["spriteName"].GetString();
    }

} // end of namespace
