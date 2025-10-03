/*********************************************************************************************
 \file      Transform.h
 \par       Bloop
 \author    Goh Pin Kai (pinkai.g@digipen.edu) 100%


\copyright All content   2025 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/
#pragma once
//#include "../ComponentJson/Component_System.h"
//#include "../Math/Matrix_3x3.h"
//#include "../Math/Vector_2D.h"
#include "pch/pch.h"        // FIRST
#include "Core/Coordinator.h"
#include "../Core/AssetsManager.h"
#include <glm/gtc/type_ptr.hpp>
#include "glhelper.h" 

#ifndef FRAMEWORK_TRANSFORM_H
#define FRAMEWORK_TRANSFORM_H

namespace Framework
{
	struct TestComponent
	{
		Vector2 Pos;
		Vector2 Scale;

	};

	class Transform  /* : public GameCom*/ {
	public:
		//ComponentTypeID typeID = Com_Trans;
		Vector2 Pos;
		Vector2 Scale;
		Vector2 Rotation;
		float angle_disp = 0.f;
		float angle_speed = 0.f;
		float mdl_ref = 0.f;    // Model no    
		float shd_ref = 0.f;    // Shader no
		Matrix3x3 mdl_to_ndc_xform;  // Initialize as identity matrix
		GLuint textureID = 0;         // Texture ID
		std::string spriteName;
		int currentFrame = 0;
		float elapsedTime = 0.0f;     // Time accumulator for animation
		int totalFrames = 1;          // Total frames in the sprite sheet
		int columns = 1;              // Columns in the sprite sheet
		int rows = 1;                 // Rows in the sprite sheet
		float animationSpeed = 2.f;
		void AssignTextureFromPath(Framework::Transform& t);

		Transform();
		~Transform();

		virtual void Initialize();
		void update(GLdouble delta_time, const std::set<Entity> entity);
		void draw() const;

		void SetSprite(std::string ref);
		std::string GetSprite() const;

		bool isVisible = true; // Used for input detect purpose
		float visibilityTimer = 0.0f; // Used for input detect purpose
		bool autoHide = false; // Used for input detect purpose

		// Register these components in your initialization code:
		//void Serialize(Value& data, Document::AllocatorType& alloc) override;
		//void Deserialize(Value& data);
		void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const;
		void Deserialize(const rapidjson::Value& in);
	};

	bool SaveTransform(Transform const& t, std::string const& path);
	bool LoadTransform(Transform& t, std::string const& path);
}
#endif // FRAMEWORK_TRANSFORM_H