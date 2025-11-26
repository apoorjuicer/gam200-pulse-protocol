/******************************************************************************/
/**
 * @file        Hierarchy.h
 * @project     Pulse Protocol
 * @author      Leu Jun Yong (primary) - 60%
 * @author      Chloe Lau Rey En (secondary) - 40%
 * @brief       TBF
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once

#include "../../imgui/imgui.h"     // for ImGuiID
#include "CoreEngine/ECS/System.h" // for Entity (same type you use in Editor)

namespace MyEditor
{
    // Draw the full Hierarchy ImGui window (toolbar + list).
    // dockID is the dockspace ID from Editor.cpp (LeftDockSpace).
    void DrawHierarchyWindow(ImGuiID dockID);
}
