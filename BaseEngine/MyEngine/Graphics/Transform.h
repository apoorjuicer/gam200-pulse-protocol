/*********************************************************************************************
 \file      Transform.h
 \par       Bloop
 \author    Goh Pin Kai (pinkai.g@digipen.edu) 100%


\copyright All content   2024 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/
#pragma once
//#include "../ComponentJson/Component_System.h"
//#include "../Math/Matrix_3x3.h"
//#include "../Math/Vector_2D.h"
#include "pch/pch.h"        // FIRST
#include "Core/Coordinator.h"
#include <vector>
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
        Vector2 Renderable;
        float angle_disp = 0.f;
        float angle_speed = 0.f;
        float mdl_ref = 0.f;    // Model no    
        float shd_ref = 0.f;    // Shader no
        Matrix3x3 mdl_to_ndc_xform;  // Initialize as identity matrix
        GLuint textureID = 0;         // Texture ID
        std::string spriteName;
        int currentFrame = 5;
        float elapsedTime = 0.0f;     // Time accumulator for animation
        int totalFrames = 6;          // Total frames in the sprite sheet
        int columns = 1;              // Columns in the sprite sheet
        int rows = 1;                 // Rows in the sprite sheet
        float animationSpeed = 7.f;

        Transform();
        ~Transform();

        virtual void Initialize();
        void update(GLdouble delta_time);
        void draw() const;

        void SetSprite(std::string ref);
        std::string GetSprite() const;
        // Register these components in your initialization code:
        //void Serialize(Value& data, Document::AllocatorType& alloc) override;
        //void Deserialize(Value& data);
    };
}
#endif // FRAMEWORK_TRANSFORM_H