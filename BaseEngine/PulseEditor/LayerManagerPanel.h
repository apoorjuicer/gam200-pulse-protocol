/******************************************************************************/
/**
 * @file        LayerManagerPanel.h
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Layer management window for controlling layer visibility,
 *              locking, and filtering in the editor.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#pragma once

#include "../../imgui/imgui.h"

namespace PulseEditor
{
    // Draw the Layer Manager window
    void DrawLayerManagerWindow();

    // Check if a layer is currently visible
    bool IsLayerVisible(unsigned int layer);

    // Check if a layer is currently locked (for editor selection)
    bool IsLayerLocked(unsigned int layer);

    // Set visibility for a specific layer (updates filter automatically)
    void SetLayerVisibility(unsigned int layer, bool visible);

    // Set visibility for all layers at once (for quick filters like TopBar)
    void SetAllLayersVisibility(bool bg, bool world, bool ui);
}