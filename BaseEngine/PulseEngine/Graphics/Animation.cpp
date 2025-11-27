/******************************************************************************/
/**
 * @file        Animation.cpp
 * @project     Pulse Protocol
 * @author      Goh Pin Kai
 * @brief       Implements the Animation component defined in Animation.h.
 *              Handles serialization and deserialization of animation state,
 *              including frame index, animation timing, and sprite sheet
 *              layout parameters for scene saving and loading.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#include "Animation.h"
#include <../MyEngine/Math/math.h>

namespace Framework
{
	static inline bool IsSingleImage(const Framework::Animation& a) {
		// treat anything that's logically 1 frame as "not a spritesheet"
		return (a.rows <= 1 && a.columns <= 1) || a.totalFrames <= 1;
	}

	// Pseudocode in your per-frame update for an entity:
	void TickAnimation(Framework::Animation& anim, float dt)
	{
		if (IsSingleImage(anim)) {
			anim.currentFrame = 0;
			anim.elapsedTime = 0.0f; // keep it frozen
			return;
		}

		anim.elapsedTime += dt;
		const float secondsPerFrame = (anim.animationSpeed > 0.0f) ? (1.0f / anim.animationSpeed) : 1.0f;
		while (anim.elapsedTime >= secondsPerFrame) {
			anim.elapsedTime -= secondsPerFrame;
			anim.currentFrame = (anim.currentFrame + 1) % std::max(1, anim.totalFrames);
		}
	}


	/*Serialization && Deserialization */
	void Animation::Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const
	{
		using namespace rapidjson;
		out.SetObject();

		out.AddMember("currentFrame", currentFrame, alloc);
		out.AddMember("elapsedTime", elapsedTime, alloc);
		out.AddMember("totalFrames", totalFrames, alloc);
		out.AddMember("columns", columns, alloc);
		out.AddMember("rows", rows, alloc);
		out.AddMember("animationSpeed", animationSpeed, alloc);

	}

	void Animation::Deserialize(const rapidjson::Value& in)
	{
		if (!in.IsObject())
		{
			throw std::runtime_error("Invalid JSON for Transform");
		}

		if (in.HasMember("currentFrame"))
			currentFrame = in["currentFrame"].GetInt();
		if (in.HasMember("elapsedTime"))
			elapsedTime = in["elapsedTime"].GetFloat();
		if (in.HasMember("totalFrames"))
			totalFrames = in["totalFrames"].GetInt();
		if (in.HasMember("columns"))
			columns = in["columns"].GetInt();
		if (in.HasMember("rows"))
			rows = in["rows"].GetInt();
		if (in.HasMember("animationSpeed"))
			animationSpeed = in["animationSpeed"].GetFloat();
	}
} // end of namespace
