/******************************************************************************/
/**
 * @file        GUIButton.h
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       GUI Button component for creation of clickable in-game buttons.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#pragma once
#include "pch/pch_temp.h"

enum class ButtonState {
    Normal,
    Hover
};

struct GUIButton {
    std::string buttonID;
    std::string label; // necessary for text on the button

    // colors for normal & hover states to differentiate whether button is being detected by mouse
    glm::vec4 normalColor;
    glm::vec4 hoverColor;

    // what actions each button can do
    std::string actionType;     // "CreateEntity" or "DestroyEntity"
    std::string entityJsonPath; // Path to entity JSON for creation

    ButtonState currentState;
    bool isEnabled;
    bool wasClickedThisFrame;

    GUIButton()
        : normalColor(0.3f, 0.3f, 0.8f, 1.0f),
        hoverColor(0.5f, 0.5f, 1.0f, 1.0f),
        currentState(ButtonState::Normal),
        isEnabled(true),
        wasClickedThisFrame(false) {
    }

    void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const;
    void Deserialize(const rapidjson::Value& in);
};
