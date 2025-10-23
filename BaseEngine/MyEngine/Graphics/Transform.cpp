/******************************************************************************/
/**
 * @file        Transform.cpp
 * @project     Pulse Protocol
 * @author      Goh Pin Kai (primary) - 50%
 * @author 		Ban Kai Wei Benjamin (secondary) - 50%
 * @brief       Implements the Transform component, providing position,
 *              scale, rotation, animation, and sprite management. Includes
 *              update logic for entity transforms, matrix composition,
 *              and JSON-based serialization/deserialization for saving
 *              and loading transform state.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#include "Transform.h"
#include <../MyEngine/Math/math.h>

namespace Framework
{
    Transform::Transform()
    {
        Rotation = Vector2(angle_disp, angle_speed);
        // Renderable = Vector2(mdl_ref, shd_ref);
        // Matrix3x3::Mtx33Identity(mdl_to_ndc_xform);
        mdl_to_ndc_xform.SetToIdentity();
    }

    Transform::~Transform() {}
    void AssignTextureFromPath(Framework::Transform &t)
    {
        if (t.textureID == 0 && !t.spriteName.empty())
            t.textureID = Framework::AssetsManager::GetOrLoad(t.spriteName);
    }
    void Transform::Initialize() {}

    void Transform::update(GLdouble delta_time, const std::set<Entity> entity)
    {
        auto *g_coordinator = Coordinator::GetInstance();

        // pull the current game area; fall back if not set
        const float gw = (GLHelper::GameWidth > 0) ? (float)GLHelper::GameWidth : 1600.0f;
        const float gh = (GLHelper::GameHeight > 0) ? (float)GLHelper::GameHeight : 900.0f;

        for (auto it : entity)
        {
            // IMPORTANT: reference, not copy
            auto &x = g_coordinator->GetComponent<Framework::Transform>(it);

            // normalize from pixels to your game space
            const float PnormX = x.Pos.x / gw;
            const float PnormY = x.Pos.y / gh;
            const float SnormX = x.Scale.x / gw;
            const float SnormY = x.Scale.y / gh;

            Matrix3x3 Scaling, Rotating, Translating;

            // S
            Scaling = Matrix3x3(
                SnormX, 0.0f, 0.0f,
                0.0f, SnormY, 0.0f,
                0.0f, 0.0f, 1.0f);

            // R (use the entity�s angle)
            constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;
            const float cosA = std::cos(x.angle_disp * DEG_TO_RAD);
            const float sinA = std::sin(x.angle_disp * DEG_TO_RAD);
            Rotating = Matrix3x3(
                cosA, sinA, 0.0f,
                -sinA, cosA, 0.0f,
                0.0f, 0.0f, 1.0f);

            // T (column-major layout for your Matrix3x3)
            Translating = Matrix3x3(
                1.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                PnormX, PnormY, 1.0f);

            // M = T * R * S
            x.mdl_to_ndc_xform = Translating * Rotating * Scaling;

            // advance entity�s angle
            x.angle_disp += x.angle_speed * static_cast<GLfloat>(delta_time);
            // std::cout << x.mdl_to_ndc_xform;
        }
    }

    void Transform::draw() const {}

    void Transform::SetSprite(std::string ref)
    {
        spriteName = ref;
    }

    std::string Transform::GetSprite() const
    {
        return spriteName;
    }

    /*Serialization && Deserialization */
    void Transform::Serialize(rapidjson::Value &out, rapidjson::Document::AllocatorType &alloc) const
    {
        using namespace rapidjson;
        out.SetObject();

        // Vector2s
        Value posVal, scaleVal, rotVal, renderVal;
        Pos.Serialize(posVal, alloc);
        Scale.Serialize(scaleVal, alloc);
        Rotation.Serialize(rotVal, alloc);

        out.AddMember("position", posVal, alloc);
        out.AddMember("scale", scaleVal, alloc);
        out.AddMember("rotation", rotVal, alloc);

        // simple types
        out.AddMember("angle_disp", angle_disp, alloc);
        out.AddMember("angle_speed", angle_speed, alloc);
        out.AddMember("mdl_ref", mdl_ref, alloc);
        out.AddMember("shd_ref", shd_ref, alloc);
        out.AddMember("textureID", static_cast<uint64_t>(textureID), alloc);

        Value spriteNameVal;
        spriteNameVal.SetString(spriteName.c_str(), static_cast<SizeType>(spriteName.length()), alloc);
        out.AddMember("spriteName", spriteNameVal, alloc);

        out.AddMember("currentFrame", currentFrame, alloc);
        out.AddMember("elapsedTime", elapsedTime, alloc);
        out.AddMember("totalFrames", totalFrames, alloc);
        out.AddMember("columns", columns, alloc);
        out.AddMember("rows", rows, alloc);
        out.AddMember("animationSpeed", animationSpeed, alloc);

        //
        out.AddMember("isVisible", isVisible, alloc);
        out.AddMember("visibilityTimer", visibilityTimer, alloc);
        out.AddMember("autoHide", autoHide, alloc);

        // mat3
        //  Matrix3x3
        Value matVal;
        mdl_to_ndc_xform.Serialize(matVal, alloc);
        out.AddMember("mdl_to_ndc_xform", matVal, alloc);
    }

    void Transform::Deserialize(const rapidjson::Value &in)
    {
        if (!in.IsObject())
        {
            throw std::runtime_error("Invalid JSON for Transform");
        }

        if (in.HasMember("position"))
            Pos.Deserialize(in["position"]);
        if (in.HasMember("scale"))
            Scale.Deserialize(in["scale"]);
        if (in.HasMember("rotation"))
            Rotation.Deserialize(in["rotation"]);

        if (in.HasMember("angle_disp"))
            angle_disp = in["angle_disp"].GetFloat();
        if (in.HasMember("angle_speed"))
            angle_speed = in["angle_speed"].GetFloat();
        if (in.HasMember("mdl_ref"))
            mdl_ref = in["mdl_ref"].GetFloat();
        if (in.HasMember("shd_ref"))
            shd_ref = in["shd_ref"].GetFloat();
        if (in.HasMember("textureID"))
            textureID = static_cast<GLuint>(in["textureID"].GetUint64());

        if (in.HasMember("spriteName"))
            spriteName = in["spriteName"].GetString();

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

        if (in.HasMember("isVisible"))
            isVisible = in["isVisible"].GetBool();
        if (in.HasMember("visibilityTimer"))
            visibilityTimer = in["visibilityTimer"].GetFloat();
        if (in.HasMember("autoHide"))
            autoHide = in["autoHide"].GetBool();

        if (in.HasMember("mdl_to_ndc_xform"))
            mdl_to_ndc_xform.Deserialize(in["mdl_to_ndc_xform"]);
    }

    bool SaveTransform(Transform const &t, std::string const &path)
    {
        rapidjson::Document d;
        d.SetObject();
        auto &alloc = d.GetAllocator();

        rapidjson::Value tVal;
        t.Serialize(tVal, alloc);
        d.AddMember("transform", tVal, alloc);

        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        d.Accept(writer);

        // Ensure directory exists
        namespace fs = std::filesystem;
        fs::create_directories("../../MyEngine/JSON");
        // fs::create_directories("../JSON");

        std::ofstream ofs(path);
        if (!ofs.is_open())
        {
            std::cerr << "Error: Cannot open file for writing: " << path << "\n";
            return false;
        }
        ofs << buffer.GetString();
        if (!ofs.good())
        {
            std::cerr << "Error: Failed to write JSON to file: " << path << "\n";
            return false;
        }
        return true;
    }

    bool LoadTransform(Transform &t, std::string const &path)
    {
        std::ifstream ifs(path);
        if (!ifs.is_open())
        {
            std::cerr << "Error: Cannot open file for reading: " << path << "\n";
            return false;
        }
        rapidjson::IStreamWrapper isw(ifs); //must have
        rapidjson::Document d;                  //must have
        d.ParseStream(isw);                       //must have

        if (d.HasParseError())
        {
            std::cerr << "Error: Failed to parse JSON in file: " << path << "\n";
            return false;
        }

        if (!d.HasMember("transform"))
        {
            std::cerr << "Error: No 'transform' field in file: " << path << "\n";
            return false;
        }

        t.Deserialize(d["transform"]);
        return true;
    }

} // end of namespace
