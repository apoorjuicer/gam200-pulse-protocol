/******************************************************************************/
/**
 * @file        Renderable.h
 * @project     Pulse Protocol
 * @author      Goh Pin Kai (primary) - 90%
 * @author 		Ban Kai Wei Benjamin (secondary) - 10%
 * @brief       Declares the Transform component for the ECS framework,
 *              storing position, scale, rotation, animation state,
 *              and sprite data. Provides interfaces for updating,
 *              serializing, and deserializing transform properties.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#pragma once
#include "pch/pch.h" // FIRST
#include "CoreEngine/ECS/Coordinator.h"
#include "CoreEngine/Asset/AssetsManager.h"
#include <glm/gtc/type_ptr.hpp>
#include "glhelper.h"

#ifndef FRAMEWORK_RENDERABLE_H
#define FRAMEWORK_RENDERABLE_H

namespace Framework
{
	class Renderable 
	{
	public:
		float mdl_ref = 0.f;		// Model no
		float shd_ref = 0.f;		// Shader no
		GLuint textureID = 0;		// Texture ID
		std::string spriteName;
		void SetSprite(std::string ref);
		std::string GetSprite() const;
		void AssignTextureFromPath(Framework::Renderable& t);
		void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const;
		void Deserialize(const rapidjson::Value& in);
	};
}
#endif // FRAMEWORK_RENDERABLE_H