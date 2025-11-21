///******************************************************************************/
///**
// * @file        UIElement.h
// * @project     Pulse Protocol
// * @author      Chloe Lau Rey En
// * @brief       Defines the UIElement component structure for UI elements.
// *              CURRENTLY FOCUSED ON: Health bar support
// *              Other UI types (buttons, text) are commented out for future use.
// *
// * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
// *              Reproduction or disclosure of this file or its contents without the
// *              prior written consent of DigiPen Institute of Technology is prohibited.
// */
// /******************************************************************************/
//
//#pragma once
//#include "pch/pch_temp.h"
//
//// Type of UI element
//enum class UIType {
//    // BUTTON,           // Future: Uncomment when needed
//    HEALTH_BAR,          // Active: For health bars
//    // TEXT_LABEL,       // Future: Uncomment when needed
//    // FEEDBACK_BAR,     // Future: Uncomment when needed
//    // PANEL,            // Future: Uncomment when needed
//    // IMAGE             // Future: Uncomment when needed
//};
//
// // Interactive elements (buttons, etc.)
//     //enum class ButtonState {
//     //    Normal,
//     //    Hover
//     //};
//
//struct UIElement {
//    // Common properties for all UI elements
//    UIType type;
//    std::string elementID;      // Unique identifier for this UI element
//    bool isEnabled;
//    // bool isInteractable;        // Future: Uncomment when adding buttons
//
//    // Visual properties (used by most UI types)
//    // glm::vec4 normalColor;      // Future: For buttons
//    // glm::vec4 hoverColor;       // Future: For buttons
//
//    // ========== BUTTON PROPERTIES (COMMENTED OUT FOR NOW) ==========
//    // std::string label;          // Text displayed on button
//    // std::string actionType;     // "CreateEntity", "DestroyEntity", "LoadScene", etc.
//    // std::string entityJsonPath; // Path to entity JSON for creation
//    // ButtonState currentState;
//    // bool wasClickedThisFrame;
//
//    // ========== HEALTH BAR PROPERTIES (ACTIVE) ==========
//    float currentValue;         // Current HP value (e.g., 3.0)
//    float maxValue;             // Maximum HP value (e.g., 5.0)
//    glm::vec4 fillColor;        // Color of the filled portion (green bar)
//    glm::vec4 backgroundColor;  // Color of the empty portion (background container)
//
//    // ========== TEXT LABEL PROPERTIES (COMMENTED OUT FOR NOW) ==========
//    // std::string displayText;    // Text to display (for text labels)
//    // float fontSize;             // Font size for text rendering
//
//    UIElement()
//        : type(UIType::HEALTH_BAR),
//        elementID(""),
//        isEnabled(true),
//        // isInteractable(false),
//         //normalColor(0.3f, 0.3f, 0.8f, 1.0f),
//         //hoverColor(0.5f, 0.5f, 1.0f, 1.0f),
//        // label(""),
//        // actionType(""),
//        // entityJsonPath(""),
//        // currentState(ButtonState::Normal),
//        // wasClickedThisFrame(false),
//        currentValue(1.0f),
//        maxValue(1.0f),
//        fillColor(0.0f, 1.0f, 0.0f, 1.0f),     // Green for health
//        backgroundColor(0.3f, 0.3f, 0.3f, 1.0f) // Dark gray
//        // displayText(""),
//        // fontSize(24.0f)
//    {
//    }
//
//    void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const;
//    void Deserialize(const rapidjson::Value& in);
//};