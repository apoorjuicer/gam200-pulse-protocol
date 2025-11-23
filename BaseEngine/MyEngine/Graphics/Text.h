/******************************************************************************/
/**
 * @file        Text.h
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin
 * @brief       Text Component
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#pragma once

#include <string>
#include <glm/vec4.hpp> 
#include <rapidjson/document.h>
#include <iostream>

struct Text
{
    std::string text{};        // Text to display
    std::string font_id{};     // key to font registry
    float x{}, y{}; //might change to offset
    float font_size{};      // pixel height
    glm::vec4 color = {1,1,1,1};  // RGBA
    bool visible = true;     // toggle on/off
    //To be added, probably typed class enum
    //std::string font_style;
    //std::string aligment;

    // ---------------------------------------------------
        // Serialize
        // ---------------------------------------------------
    void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const {
        using namespace rapidjson;
        out.SetObject();

        //string
        Value TextVal;
        TextVal.SetString(text.c_str(),
            static_cast<SizeType>(text.length()),
            alloc);
        out.AddMember("text", TextVal, alloc);

        Value fond_idVal;
        fond_idVal.SetString(font_id.c_str(),
            static_cast<SizeType>(font_id.length()),
            alloc);
        out.AddMember("font_id", fond_idVal, alloc);
        //float
        out.AddMember("x", x, alloc);
        out.AddMember("y", y, alloc);
        out.AddMember("font_size", font_size, alloc);
        //vec4
        
            rapidjson::Value colorArr(rapidjson::kArrayType);

            colorArr.PushBack(color.r, alloc);
            colorArr.PushBack(color.g, alloc);
            colorArr.PushBack(color.b, alloc);
            colorArr.PushBack(color.a, alloc);

            out.AddMember("color", colorArr, alloc);
       
        //bool        
        out.AddMember("visible", visible, alloc);
    }
    // ---------------------------------------------------
    // Deserialize
    // ---------------------------------------------------
    void Deserialize(const rapidjson::Value& in) {

        if (!in.IsObject())
        {
            std::cerr << "[TextComponent] Invalid JSON format for TextComponent\n";
            return;
        }
        //string
        if (in.HasMember("text") && in["text"].IsString())
        {
            text = (in["text"].GetString());
        }
        if (in.HasMember("font_id") && in["font_id"].IsString())
        {
            font_id = (in["font_id"].GetString());
        }
        //float
        if (in.HasMember("x") && in["x"].IsNumber())
        {
            x = (in["x"].GetFloat());
        }
        if (in.HasMember("y") && in["y"].IsNumber())
        {
            y = (in["y"].GetFloat());
        }
        if (in.HasMember("font_size") && in["font_size"].IsNumber())
        {
            font_size = (in["font_size"].GetFloat());
        }

        // glm::vec4 color
        //check if contain color member and if its an array
        if (in.HasMember("color") && in["color"].IsArray()) 
        {
            auto const& arr = in["color"].GetArray();
            if (arr.Size() == 4) //fixed size 4
            {
                color.r = arr[0].GetFloat();
                color.g = arr[1].GetFloat();
                color.b = arr[2].GetFloat();
                color.a = arr[3].GetFloat();
            }
            else
            {
                std::cerr << "[TextComponent] 'color' array must have 4 elements (r,g,b,a)\n";
            }
        }

        //bool
        if (in.HasMember("visible") && in["visible"].IsBool())
        {
            visible = (in["visible"].GetBool());
        }

    }

};