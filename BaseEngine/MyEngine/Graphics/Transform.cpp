///*********************************************************************************************
// \file      Transform.cpp
// \par       Bloop
// \author    Goh Pin Kai (pinkai.g@digipen.edu)
//
//
//\copyright All content   2024 DigiPen Institute of Technology Singapore. All rights reserved.
//**********************************************************************************************/
//#include "Transform.h"
//#include "../Math/Matrix_3x3.h"
//
//namespace Framework
//{
//    Transform::Transform() {
//        Rotation = Vector2D(angle_disp, angle_speed);
//        Renderable = Vector2D(mdl_ref, shd_ref);
//        Matrix_3x3::Mtx33Identity(mdl_to_ndc_xform);
//    }
//
//    Transform::~Transform() {}
//
//    void Transform::Initialize() {}
//
//    void Transform::update(GLdouble delta_time) {
//        float PnormX = Pos.getX() / 1600.0f;
//        float PnormY = Pos.getY() / 900.0f;
//
//        float SnormX = Scale.getX() / 1600.0f;
//        float SnormY = Scale.getY() / 900.0f;
//
//        Matrix_3x3 Scaling, Rotating, Translating, scalingMatrix, translationMatrix;
//
//        // Scaling matrix
//        Scaling = Matrix_3x3(
//            SnormX, 0.0f, 0.0f,
//            0.0f, SnormY, 0.0f,
//            0.0f, 0.0f, 1.0f
//        );
//
//        // Rotating matrix
//        constexpr float DEG_TO_RAD = 3.14159f / 180.0f;
//        float cosA = std::cos(angle_disp * DEG_TO_RAD);
//        float sinA = std::sin(angle_disp * DEG_TO_RAD);
//        Rotating = Matrix_3x3(
//            cosA, sinA, 0.0f,
//            -sinA, cosA, 0.0f,
//            0.0f, 0.0f, 1.0f
//        );
//
//        // Translating matrix
//        Translating = Matrix_3x3(
//            1.0f, 0.0f, PnormX,
//            0.0f, 1.0f, PnormY,
//            0.0f, 0.0f, 1.0f
//        );
//
//        // Aspect ratio scaling
//        float objectAspect = SnormX / SnormY;
//        float viewportAspect = static_cast<float>(GLHelper::GameWidth) / static_cast<float>(GLHelper::GameHeight);
//        float scale;
//        if (objectAspect > viewportAspect) {
//            scale = static_cast<float>(GLHelper::GameWidth) / static_cast<float>(GLHelper::GameWidth);
//        }
//        else {
//            scale = static_cast<float>(GLHelper::GameHeight) / static_cast<float>(GLHelper::GameHeight);
//        }
//        scalingMatrix = Matrix_3x3(
//            scale, 0.0f, 0.0f,
//            0.0f, scale, 0.0f,
//            0.0f, 0.0f, 1.0f
//        );
//
//        // Translation matrix for offsets
//        float offsetX = (GLHelper::GameWidth - (SnormX * scale)) / 2;
//        float offsetY = (GLHelper::GameHeight - (SnormY * scale)) / 2;
//        translationMatrix = Matrix_3x3(
//            1.0f, 0.0f, offsetX,
//            0.0f, 1.0f, offsetY,
//            0.0f, 0.0f, 1.0f
//        );
//
//        // Calculate the final transformation matrix
//        Matrix_3x3 temp;
//        Matrix_3x3::Mtx33Transpose(temp, Translating);
//        mdl_to_ndc_xform = translationMatrix * scalingMatrix * temp * Rotating * Scaling;
//        angle_disp += angle_speed * static_cast<GLfloat>(delta_time);
//    }
//
//    void Transform::draw() const {}
//
//    void Transform::SetSprite(std::string ref) {
//        spriteName = ref;
//    }
//
//    std::string Transform::GetSprite() const {
//        return spriteName;
//    }
//
//    //void Transform::Serialize(Value& data, Document::AllocatorType& alloc) {
//    //    data.SetObject();
//    //    Value value;
//    //    Pos.Serialize(value, alloc);
//    //    data.AddMember("PosVec", value, alloc);
//
//    //    Scale.Serialize(value, alloc);
//    //    data.AddMember("ScaleVec", value, alloc);
//
//    //    Rotation.Serialize(value, alloc);
//    //    data.AddMember("RotaVec", value, alloc);
//
//    //    Renderable.Serialize(value, alloc);
//    //    data.AddMember("RendVec", value, alloc);
//
//    //    data.AddMember("spriteName", StringRef(spriteName.c_str()), alloc);
//    //    data.AddMember("angle_disp", angle_disp, alloc);
//    //    data.AddMember("angle_speed", angle_speed, alloc);
//    //    data.AddMember("mdl_ref", mdl_ref, alloc);
//    //    data.AddMember("shd_ref", shd_ref, alloc);
//    //}
//
//    //void Transform::Deserialize(Value& data) {
//    //    Pos.Deserialize(data["PosVec"]);
//    //    Scale.Deserialize(data["ScaleVec"]);
//    //    Rotation.Deserialize(data["RotaVec"]);
//    //    Renderable.Deserialize(data["RendVec"]);
//
//    //    spriteName = data["spriteName"].GetString();
//    //    angle_disp = data["angle_disp"].GetFloat();
//    //    angle_speed = data["angle_speed"].GetFloat();
//    //    mdl_ref = data["mdl_ref"].GetFloat();
//    //    shd_ref = data["shd_ref"].GetFloat();
//    //}
//}
