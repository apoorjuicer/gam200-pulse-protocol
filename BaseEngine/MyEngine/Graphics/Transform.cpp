/*********************************************************************************************
 \file      Transform.cpp
 \par       Bloop
 \author    Goh Pin Kai (pinkai.g@digipen.edu)


\copyright All content   2025 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/
#include "Transform.h"
#include <../MyEngine/Math/math.h>

namespace Framework
{
    Transform::Transform() {
        Rotation = Vector2(angle_disp, angle_speed);
        //Renderable = Vector2(mdl_ref, shd_ref);
        //Matrix3x3::Mtx33Identity(mdl_to_ndc_xform);
        mdl_to_ndc_xform.SetToIdentity();
    }

    Transform::~Transform() {}

    void Transform::Initialize() {}

    void Transform::update(GLdouble delta_time,std::set<Entity> entity) {
        auto* g_coordinator = Coordinator::GetInstance();
        for (auto& it : entity)
        {
            auto x = g_coordinator->GetComponent<Framework::Transform>(it);
            float PnormX = x.Pos.x / 1600.0f;
            float PnormY = x.Pos.y / 900.0f;

            float SnormX = x.Scale.x / 1600.0f;
            float SnormY = x.Scale.y / 900.0f;

            Matrix3x3 Scaling, Rotating, Translating/*, scalingMatrix, translationMatrix*/;

            // Scaling matrix
            Scaling = Matrix3x3(
                SnormX, 0.0f, 0.0f,
                0.0f, SnormY, 0.0f,
                0.0f, 0.0f, 1.0f
            );

            // Rotating matrix
            constexpr float DEG_TO_RAD = 3.14159f / 180.0f;
            float cosA = std::cos(angle_disp * DEG_TO_RAD);
            float sinA = std::sin(angle_disp * DEG_TO_RAD);
            Rotating = Matrix3x3(
                cosA, sinA, 0.0f,
                -sinA, cosA, 0.0f,
                0.0f, 0.0f, 1.0f
            );
            Translating = Matrix3x3(
                1.0f, 0.0f, 0.0f,   // first column
                0.0f, 1.0f, 0.0f,   // second column
                PnormX, PnormY, 1.0f  // third column
            );
            //// Aspect ratio scaling
            //float objectAspect = SnormX / SnormY;
            //float viewportAspect = static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight);
            //float scale;
            //if (objectAspect > viewportAspect) {
            //    scale = static_cast<float>(WindowWidth) / static_cast<float>(WindowWidth);
            //}
            //else {
            //    scale = static_cast<float>(WindowHeight) / static_cast<float>(WindowHeight);
            //}
            //scalingMatrix = Matrix3x3(
            //    scale, 0.0f, 0.0f,
            //    0.0f, scale, 0.0f,
            //    0.0f, 0.0f, 1.0f
            //);

            //// Translation matrix for offsets
            //float offsetX = (WindowWidth - (SnormX * scale)) / 2;
            //float offsetY = (WindowHeight - (SnormY * scale)) / 2;
            //translationMatrix = Matrix3x3(
            //    1.0f, 0.0f, offsetX,
            //    0.0f, 1.0f, offsetY,
            //    0.0f, 0.0f, 1.0f
            //);

            // Calculate the final transformation matrix
            //Matrix3x3 temp;
            //Matrix3x3::Mtx33Transpose(temp, Translating);
            x.mdl_to_ndc_xform = /*translationMatrix * scalingMatrix **/ Translating * Rotating * Scaling;
            x.angle_disp += x.angle_speed * static_cast<GLfloat>(delta_time);
        }
    }

    void Transform::draw() const {}

    void Transform::SetSprite(std::string ref) {
        spriteName = ref;
    }

    std::string Transform::GetSprite() const {
        return spriteName;
    }

    //void Transform::Serialize(Value& data, Document::AllocatorType& alloc) {
    //    data.SetObject();
    //    Value value;
    //    Pos.Serialize(value, alloc);
    //    data.AddMember("PosVec", value, alloc);

    //    Scale.Serialize(value, alloc);
    //    data.AddMember("ScaleVec", value, alloc);

    //    Rotation.Serialize(value, alloc);
    //    data.AddMember("RotaVec", value, alloc);

    //    Renderable.Serialize(value, alloc);
    //    data.AddMember("RendVec", value, alloc);

    //    data.AddMember("spriteName", StringRef(spriteName.c_str()), alloc);
    //    data.AddMember("angle_disp", angle_disp, alloc);
    //    data.AddMember("angle_speed", angle_speed, alloc);
    //    data.AddMember("mdl_ref", mdl_ref, alloc);
    //    data.AddMember("shd_ref", shd_ref, alloc);
    //}

    //void Transform::Deserialize(Value& data) {
    //    Pos.Deserialize(data["PosVec"]);
    //    Scale.Deserialize(data["ScaleVec"]);
    //    Rotation.Deserialize(data["RotaVec"]);
    //    Renderable.Deserialize(data["RendVec"]);

    //    spriteName = data["spriteName"].GetString();
    //    angle_disp = data["angle_disp"].GetFloat();
    //    angle_speed = data["angle_speed"].GetFloat();
    //    mdl_ref = data["mdl_ref"].GetFloat();
    //    shd_ref = data["shd_ref"].GetFloat();
    //}
}


///*Serialization && Deserialization */
//void Transform::Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const {
//    using namespace rapidjson;
//    out.SetObject();
//
//    //Vector2s
//    Value posVal, scaleVal, rotVal, renderVal;
//    Pos.Serialize(posVal, alloc);
//    Scale.Serialize(scaleVal, alloc);
//    Rotation.Serialize(rotVal, alloc);
//    Renderable.Serialize(renderVal, alloc);
//
//    out.AddMember("position", posVal, alloc);
//    out.AddMember("scale", scaleVal, alloc);
//    out.AddMember("rotation", rotVal, alloc);
//    out.AddMember("renderable", renderVal, alloc);
//
//    //simple types
//    out.AddMember("angle_disp", angle_disp, alloc);
//    out.AddMember("angle_speed", angle_speed, alloc);
//    out.AddMember("mdl_ref", mdl_ref, alloc);
//    out.AddMember("shd_ref", shd_ref, alloc);
//    out.AddMember("textureID", static_cast<uint64_t>(textureID), alloc);
//
//    Value spriteNameVal;
//    spriteNameVal.SetString(spriteName.c_str(), static_cast<SizeType>(spriteName.length()), alloc);
//    out.AddMember("spriteName", spriteNameVal, alloc);
//
//    out.AddMember("currentFrame", currentFrame, alloc);
//    out.AddMember("elapsedTime", elapsedTime, alloc);
//    out.AddMember("totalFrames", totalFrames, alloc);
//    out.AddMember("columns", columns, alloc);
//    out.AddMember("rows", rows, alloc);
//    out.AddMember("animationSpeed", animationSpeed, alloc);
//
//    //mat3
//    // Matrix3x3
//    Value matVal;
//    mdl_to_ndc_xform.Serialize(matVal, alloc);
//    out.AddMember("mdl_to_ndc_xform", matVal, alloc);
//
//}
//
//void Transform::Deserialize(const rapidjson::Value& in) {
//    if (!in.IsObject()) {
//        throw std::runtime_error("Invalid JSON for Transform");
//    }
//
//    if (in.HasMember("position"))    Pos.Deserialize(in["position"]);
//    if (in.HasMember("scale"))       Scale.Deserialize(in["scale"]);
//    if (in.HasMember("rotation"))    Rotation.Deserialize(in["rotation"]);
//    if (in.HasMember("renderable"))  Renderable.Deserialize(in["renderable"]);
//
//    if (in.HasMember("angle_disp"))  angle_disp = in["angle_disp"].GetFloat();
//    if (in.HasMember("angle_speed")) angle_speed = in["angle_speed"].GetFloat();
//    if (in.HasMember("mdl_ref"))     mdl_ref = in["mdl_ref"].GetFloat();
//    if (in.HasMember("shd_ref"))     shd_ref = in["shd_ref"].GetFloat();
//    if (in.HasMember("textureID"))   textureID = static_cast<GLuint>(in["textureID"].GetUint64());
//
//    if (in.HasMember("spriteName"))  spriteName = in["spriteName"].GetString();
//
//    if (in.HasMember("currentFrame")) currentFrame = in["currentFrame"].GetInt();
//    if (in.HasMember("elapsedTime"))  elapsedTime = in["elapsedTime"].GetFloat();
//    if (in.HasMember("totalFrames"))  totalFrames = in["totalFrames"].GetInt();
//    if (in.HasMember("columns"))      columns = in["columns"].GetInt();
//    if (in.HasMember("rows"))         rows = in["rows"].GetInt();
//    if (in.HasMember("animationSpeed")) animationSpeed = in["animationSpeed"].GetFloat();
//
//    if (in.HasMember("mdl_to_ndc_xform"))
//        mdl_to_ndc_xform.Deserialize(in["mdl_to_ndc_xform"]);
//}
//
//bool SaveTransform(Transform const& t, std::string const& path) {
//    rapidjson::Document d;
//    d.SetObject();
//    auto& alloc = d.GetAllocator();
//
//    rapidjson::Value tVal;
//    t.Serialize(tVal, alloc);
//    d.AddMember("transform", tVal, alloc);
//
//    rapidjson::StringBuffer buffer;
//    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
//    d.Accept(writer);
//
//    // Ensure directory exists
//    namespace fs = std::filesystem;
//    //fs::create_directories("../../MyEngine/JSON");
//    fs::create_directories("../JSON");
//
//    std::ofstream ofs(path);
//    if (!ofs.is_open()) {
//        std::cerr << "Error: Cannot open file for writing: " << path << "\n";
//        return false;
//    }
//    ofs << buffer.GetString();
//    if (!ofs.good()) {
//        std::cerr << "Error: Failed to write JSON to file: " << path << "\n";
//        return false;
//    }
//    return true;
//}
//
//
//bool LoadTransform(Transform& t, std::string const& path) {
//    std::ifstream ifs(path);
//    if (!ifs.is_open()) {
//        std::cerr << "Error: Cannot open file for reading: " << path << "\n";
//        return false;
//    }
//    rapidjson::IStreamWrapper isw(ifs);
//    rapidjson::Document d;
//    d.ParseStream(isw);
//
//    if (d.HasParseError()) {
//        std::cerr << "Error: Failed to parse JSON in file: " << path << "\n";
//        return false;
//    }
//
//    if (!d.HasMember("transform")) {
//        std::cerr << "Error: No 'transform' field in file: " << path << "\n";
//        return false;
//    }
//
//    t.Deserialize(d["transform"]);
//    return true;
//}
//}