/******************************************************************************/
/**
 * @file        Animation.h
 * @project     Pulse Protocol
 * @author      Goh Pin Kai - 100%
 * @brief       Declares the Animation component for the ECS framework.
 *              This component stores sprite sheet animation playback state,
 *              including frame indices, timing, and atlas layout information.
 *              Provides serialization and deserialization support for
 *              saving and loading animation settings in scene files.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#pragma once
// #include "../ComponentJson/Component_System.h"
// #include "../Math/Matrix_3x3.h"
// #include "../Math/Vector_2D.h"
#include "pch/pch.h" // FIRST
#include "CoreEngine/ECS/Coordinator.h"
#include "CoreEngine/Asset/AssetsManager.h"
#include <glm/gtc/type_ptr.hpp>
#include "glhelper.h"

#ifndef FRAMEWORK_ANIMATION_H
#define FRAMEWORK_ANIMATION_H

namespace Framework
{
	class Animation
	{
	public:
		int currentFrame = 0;
		float elapsedTime = 0.0f; // Time accumulator for animation
		int totalFrames = 1;	  // Total frames in the sprite sheet
		int columns = 1;		  // Columns in the sprite sheet
		int rows = 1;			  // Rows in the sprite sheet
		float animationSpeed = 2.0f;
		void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const;
		void Deserialize(const rapidjson::Value& in);
		static inline bool IsSingleImage(const Framework::Animation& a);
		void TickAnimation(Framework::Animation& anim, float dt);
	};
}
#endif // FRAMEWORK_ANIMATION_H