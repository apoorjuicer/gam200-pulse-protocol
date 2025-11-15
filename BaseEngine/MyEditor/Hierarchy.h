#pragma once

#include "../../imgui/imgui.h"     // for ImGuiID
#include "CoreEngine/ECS/System.h" // for Entity (same type you use in Editor)

namespace MyEditor
{
    // Draw the full Hierarchy ImGui window (toolbar + list).
    // dockID is the dockspace ID from Editor.cpp (LeftDockSpace).
    void DrawHierarchyWindow(ImGuiID dockID);
}
