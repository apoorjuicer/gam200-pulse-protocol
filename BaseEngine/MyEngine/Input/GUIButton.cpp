/******************************************************************************/
/**
 * @file        GUIButton.h
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Implementation of GUIButton component serialization.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "pch/pch.h"

void GUIButton::Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const {
    using namespace rapidjson;
    out.SetObject();

    out.AddMember("buttonID", Value(buttonID.c_str(), alloc), alloc);
    out.AddMember("label", Value(label.c_str(), alloc), alloc);

    Value normalCol(kObjectType);
    normalCol.AddMember("r", normalColor.r, alloc);
    normalCol.AddMember("g", normalColor.g, alloc);
    normalCol.AddMember("b", normalColor.b, alloc);
    normalCol.AddMember("a", normalColor.a, alloc);
    out.AddMember("normalColor", normalCol, alloc);

    Value hoverCol(kObjectType);
    hoverCol.AddMember("r", hoverColor.r, alloc);
    hoverCol.AddMember("g", hoverColor.g, alloc);
    hoverCol.AddMember("b", hoverColor.b, alloc);
    hoverCol.AddMember("a", hoverColor.a, alloc);
    out.AddMember("hoverColor", hoverCol, alloc);

    out.AddMember("actionType", Value(actionType.c_str(), alloc), alloc);
    out.AddMember("entityJsonPath", Value(entityJsonPath.c_str(), alloc), alloc);
    out.AddMember("isEnabled", isEnabled, alloc);
}

void GUIButton::Deserialize(const rapidjson::Value& in) {
    if (!in.IsObject()) {
        throw std::runtime_error("Invalid JSON for GUIButton");
    }

    if (in.HasMember("buttonID") && in["buttonID"].IsString()) {
        buttonID = in["buttonID"].GetString();
    }
    if (in.HasMember("label") && in["label"].IsString()) {
        label = in["label"].GetString();
    }

    if (in.HasMember("normalColor") && in["normalColor"].IsObject()) {
        auto& col = in["normalColor"];
        if (col.HasMember("r")) normalColor.r = col["r"].GetFloat();
        if (col.HasMember("g")) normalColor.g = col["g"].GetFloat();
        if (col.HasMember("b")) normalColor.b = col["b"].GetFloat();
        if (col.HasMember("a")) normalColor.a = col["a"].GetFloat();
    }

    if (in.HasMember("hoverColor") && in["hoverColor"].IsObject()) {
        auto& col = in["hoverColor"];
        if (col.HasMember("r")) hoverColor.r = col["r"].GetFloat();
        if (col.HasMember("g")) hoverColor.g = col["g"].GetFloat();
        if (col.HasMember("b")) hoverColor.b = col["b"].GetFloat();
        if (col.HasMember("a")) hoverColor.a = col["a"].GetFloat();
    }

    if (in.HasMember("actionType") && in["actionType"].IsString()) {
        actionType = in["actionType"].GetString();
    }
    if (in.HasMember("entityJsonPath") && in["entityJsonPath"].IsString()) {
        entityJsonPath = in["entityJsonPath"].GetString();
    }
    if (in.HasMember("isEnabled") && in["isEnabled"].IsBool()) {
        isEnabled = in["isEnabled"].GetBool();
    }
}