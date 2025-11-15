#pragma once

// Need ImGuiID for the dock target
#include "../../imgui/imgui.h"

namespace MyEditor
{
    // Draw the right-side Inspector window, docked into rightDockID
    void DrawInspectorWindow(ImGuiID rightDockID);
}
