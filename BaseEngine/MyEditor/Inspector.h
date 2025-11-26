#pragma once

// Need ImGuiID for the dock target
#include "../../imgui/imgui.h"
class Coordinator;

namespace MyEditor
{
    // Draw the right-side Inspector window, docked into rightDockID
    void DrawInspectorWindow(ImGuiID rightDockID);

    void Inspector_NameComponent(Coordinator* g);
    void Inspector_TransformComponent(Coordinator* g);
    void Inspector_RenderableComponent(Coordinator* g);
    void Inspector_AnimationComponent(Coordinator* g);
    void Inspector_LogicComponent(Coordinator* g);
    void Inspector_AudioComponent(Coordinator* g);



}// end of namespace MyEditor
