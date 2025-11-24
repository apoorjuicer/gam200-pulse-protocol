#pragma once

// Need ImGuiID for the dock target
#include "../../imgui/imgui.h"

class Coordinator;

namespace MyEditor
{
    // Draw the right-side Inspector window, docked into rightDockID
    void DrawInspectorWindow(ImGuiID rightDockID);

    void AudioComponent(Coordinator* g);
}// end of namespace MyEditor
