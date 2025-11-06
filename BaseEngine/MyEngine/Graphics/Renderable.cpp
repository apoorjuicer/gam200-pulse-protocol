/******************************************************************************/
/**
 * @file        Renderable.cpp
 * @project     Pulse Protocol
 * @author      Goh Pin Kai - 100%
 * @brief       Implements the Renderable component defined in Renderable.h.
 *              Provides texture assignment from asset paths and handles
 *              serialization and deserialization of Renderable data for
 *              scene saving and loading. This includes model reference,
 *              shader reference, texture ID, and sprite/texture lookup name.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#include "../MyEngine/Graphics/Renderable.h"
#include "../MyEngine/Math/math.h"

namespace Framework
{
	void AssignTextureFromPath(Framework::Renderable &t)
	{
		if (t.textureID == 0 && !t.spriteName.empty())
		{
			// t.textureID = Framework::GraphicsManager::GetOrLoad(t.spriteName);
			//  t.textureID = mAssets.getGraphics().GetOrLoad(t.spriteName);
			t.textureID = mAssets.GetOrLoadTexture(t.spriteName);
		}
	}

	/*Serialization && Deserialization */
	void Renderable::Serialize(rapidjson::Value &out, rapidjson::Document::AllocatorType &alloc) const
	{
		using namespace rapidjson;
		out.SetObject();

		out.AddMember("mdl_ref", mdl_ref, alloc);
		out.AddMember("shd_ref", shd_ref, alloc);
<<<<<<< Updated upstream
		//out.AddMember("textureID", static_cast<uint64_t>(textureID), alloc);
=======
		//t.AddMember("textureID", static_cast<uint64_t>(textureID), alloc);
>>>>>>> Stashed changes
		out.AddMember("useTexture", useTexture, alloc);

		Value spriteNameVal;
		spriteNameVal.SetString(spriteName.c_str(), static_cast<SizeType>(spriteName.length()), alloc);
		out.AddMember("spriteName", spriteNameVal, alloc);

		/* Color Serialization part */
		Value colorVal(kObjectType);
		colorVal.AddMember("r", tintColor.r, alloc);
		colorVal.AddMember("g", tintColor.g, alloc);
		colorVal.AddMember("b", tintColor.b, alloc);
		colorVal.AddMember("a", tintColor.a, alloc);
		out.AddMember("tintColor", colorVal, alloc);
	}

	void Renderable::Deserialize(const rapidjson::Value &in)
	{
		if (!in.IsObject())
		{
			throw std::runtime_error("Invalid JSON for Transform");
		}

		if (in.HasMember("mdl_ref"))
			mdl_ref = in["mdl_ref"].GetFloat();
		if (in.HasMember("shd_ref"))
			shd_ref = in["shd_ref"].GetFloat();
<<<<<<< Updated upstream
		//if (in.HasMember("textureID"))
			//textureID = static_cast<GLuint>(in["textureID"].GetUint64());
=======
		/*if (in.HasMember("textureID"))
			textureID = static_cast<GLuint>(in["textureID"].GetUint64());*/
>>>>>>> Stashed changes

		if (in.HasMember("spriteName"))
			spriteName = in["spriteName"].GetString();

		if (in.HasMember("useTexture"))
			useTexture = in["useTexture"].GetBool();

		/* color deserialization part */
		if (in.HasMember("tintColor") && in["tintColor"].IsObject())
		{
			auto &col = in["tintColor"];
			if (col.HasMember("r"))
				tintColor.r = col["r"].GetFloat();
			if (col.HasMember("g"))
				tintColor.g = col["g"].GetFloat();
			if (col.HasMember("b"))
				tintColor.b = col["b"].GetFloat();
			if (col.HasMember("a"))
				tintColor.a = col["a"].GetFloat();
		}
	}

} // end of namespace
