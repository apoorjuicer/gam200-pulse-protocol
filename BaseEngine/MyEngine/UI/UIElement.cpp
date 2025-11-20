/******************************************************************************/
/**
 * @file        UIElement.cpp
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Implements serialization and deserialization for UIElement.
 *              CURRENTLY FOCUSED ON: Health bar support only
 *              Other UI types are commented out for future use.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "UIElement.h"

void UIElement::Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const {
    using namespace rapidjson;
    out.SetObject();

    // Serialize UI type
    const char* typeStr = "HEALTH_BAR";
    // switch (type) {
    //     case UIType::BUTTON: typeStr = "BUTTON"; break;
    //     case UIType::HEALTH_BAR: typeStr = "HEALTH_BAR"; break;
    //     case UIType::TEXT_LABEL: typeStr = "TEXT_LABEL"; break;
    //     case UIType::FEEDBACK_BAR: typeStr = "FEEDBACK_BAR"; break;
    //     case UIType::PANEL: typeStr = "PANEL"; break;
    //     case UIType::IMAGE: typeStr = "IMAGE"; break;
    // }
    out.AddMember("type", Value(typeStr, alloc), alloc);

    // Common properties
    out.AddMember("elementID", Value(elementID.c_str(), alloc), alloc);
    out.AddMember("isEnabled", isEnabled, alloc);
    // out.AddMember("isInteractable", isInteractable, alloc);  // Future: Uncomment for buttons

    // ========== BUTTON/UI COLORS (COMMENTED OUT) ==========
    // Value normalCol(kObjectType);
    // normalCol.AddMember("r", normalColor.r, alloc);
    // normalCol.AddMember("g", normalColor.g, alloc);
    // normalCol.AddMember("b", normalColor.b, alloc);
    // normalCol.AddMember("a", normalColor.a, alloc);
    // out.AddMember("normalColor", normalCol, alloc);

    // Value hoverCol(kObjectType);
    // hoverCol.AddMember("r", hoverColor.r, alloc);
    // hoverCol.AddMember("g", hoverColor.g, alloc);
    // hoverCol.AddMember("b", hoverColor.b, alloc);
    // hoverCol.AddMember("a", hoverColor.a, alloc);
    // out.AddMember("hoverColor", hoverCol, alloc);

    // ========== BUTTON PROPERTIES (COMMENTED OUT) ==========
    // out.AddMember("label", Value(label.c_str(), alloc), alloc);
    // out.AddMember("actionType", Value(actionType.c_str(), alloc), alloc);
    // out.AddMember("entityJsonPath", Value(entityJsonPath.c_str(), alloc), alloc);

    // ========== HEALTH BAR PROPERTIES ==========
    out.AddMember("currentValue", currentValue, alloc);
    out.AddMember("maxValue", maxValue, alloc);

    Value fillCol(kObjectType);
    fillCol.AddMember("r", fillColor.r, alloc);
    fillCol.AddMember("g", fillColor.g, alloc);
    fillCol.AddMember("b", fillColor.b, alloc);
    fillCol.AddMember("a", fillColor.a, alloc);
    out.AddMember("fillColor", fillCol, alloc);

    Value bgCol(kObjectType);
    bgCol.AddMember("r", backgroundColor.r, alloc);
    bgCol.AddMember("g", backgroundColor.g, alloc);
    bgCol.AddMember("b", backgroundColor.b, alloc);
    bgCol.AddMember("a", backgroundColor.a, alloc);
    out.AddMember("backgroundColor", bgCol, alloc);

    // ========== TEXT PROPERTIES (COMMENTED OUT) ==========
    // out.AddMember("displayText", Value(displayText.c_str(), alloc), alloc);
    // out.AddMember("fontSize", fontSize, alloc);
}

void UIElement::Deserialize(const rapidjson::Value& in) {
    if (!in.IsObject()) {
        throw std::runtime_error("Invalid JSON for UIElement");
    }

    // Deserialize UI type
    if (in.HasMember("type") && in["type"].IsString()) {
        std::string typeStr = in["type"].GetString();
        // if (typeStr == "BUTTON") type = UIType::BUTTON;
        if (typeStr == "HEALTH_BAR") type = UIType::HEALTH_BAR;
        // else if (typeStr == "TEXT_LABEL") type = UIType::TEXT_LABEL;
        // else if (typeStr == "FEEDBACK_BAR") type = UIType::FEEDBACK_BAR;
        // else if (typeStr == "PANEL") type = UIType::PANEL;
        // else if (typeStr == "IMAGE") type = UIType::IMAGE;
    }

    // Common properties
    if (in.HasMember("elementID") && in["elementID"].IsString()) {
        elementID = in["elementID"].GetString();
    }

    if (in.HasMember("isEnabled") && in["isEnabled"].IsBool()) {
        isEnabled = in["isEnabled"].GetBool();
    }
    // if (in.HasMember("isInteractable") && in["isInteractable"].IsBool()) {
    //     isInteractable = in["isInteractable"].GetBool();
    // }

    // ========== BUTTON/UI COLORS (COMMENTED OUT) ==========
    // if (in.HasMember("normalColor") && in["normalColor"].IsObject()) {
    //     auto& col = in["normalColor"];
    //     if (col.HasMember("r")) normalColor.r = col["r"].GetFloat();
    //     if (col.HasMember("g")) normalColor.g = col["g"].GetFloat();
    //     if (col.HasMember("b")) normalColor.b = col["b"].GetFloat();
    //     if (col.HasMember("a")) normalColor.a = col["a"].GetFloat();
    // }

    // if (in.HasMember("hoverColor") && in["hoverColor"].IsObject()) {
    //     auto& col = in["hoverColor"];
    //     if (col.HasMember("r")) hoverColor.r = col["r"].GetFloat();
    //     if (col.HasMember("g")) hoverColor.g = col["g"].GetFloat();
    //     if (col.HasMember("b")) hoverColor.b = col["b"].GetFloat();
    //     if (col.HasMember("a")) hoverColor.a = col["a"].GetFloat();
    // }

    // ========== BUTTON PROPERTIES (COMMENTED OUT) ==========
    // if (in.HasMember("label") && in["label"].IsString()) {
    //     label = in["label"].GetString();
    // }
    // if (in.HasMember("actionType") && in["actionType"].IsString()) {
    //     actionType = in["actionType"].GetString();
    // }
    // if (in.HasMember("entityJsonPath") && in["entityJsonPath"].IsString()) {
    //     entityJsonPath = in["entityJsonPath"].GetString();
    // }

    // ========== HEALTH BAR PROPERTIES (ACTIVE) ==========
    if (in.HasMember("currentValue") && in["currentValue"].IsNumber()) {
        currentValue = in["currentValue"].GetFloat();
    }
    if (in.HasMember("maxValue") && in["maxValue"].IsNumber()) {
        maxValue = in["maxValue"].GetFloat();
    }

    if (in.HasMember("fillColor") && in["fillColor"].IsObject()) {
        auto& col = in["fillColor"];
        if (col.HasMember("r")) fillColor.r = col["r"].GetFloat();
        if (col.HasMember("g")) fillColor.g = col["g"].GetFloat();
        if (col.HasMember("b")) fillColor.b = col["b"].GetFloat();
        if (col.HasMember("a")) fillColor.a = col["a"].GetFloat();
    }

    if (in.HasMember("backgroundColor") && in["backgroundColor"].IsObject()) {
        auto& col = in["backgroundColor"];
        if (col.HasMember("r")) backgroundColor.r = col["r"].GetFloat();
        if (col.HasMember("g")) backgroundColor.g = col["g"].GetFloat();
        if (col.HasMember("b")) backgroundColor.b = col["b"].GetFloat();
        if (col.HasMember("a")) backgroundColor.a = col["a"].GetFloat();
    }

    // ========== TEXT PROPERTIES (COMMENTED OUT) ==========
    // if (in.HasMember("displayText") && in["displayText"].IsString()) {
    //     displayText = in["displayText"].GetString();
    // }
    // if (in.HasMember("fontSize") && in["fontSize"].IsNumber()) {
    //     fontSize = in["fontSize"].GetFloat();
    // }
}