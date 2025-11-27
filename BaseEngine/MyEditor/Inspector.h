/******************************************************************************/
/**
 * @file        Inspector.h
 * @project     Pulse Protocol
 * @author      Leu Jun Yong
 * @brief       TBF
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once

// Need ImGuiID for the dock target
#include "../../imgui/imgui.h"
class Coordinator;

namespace MyEditor
{
    // Draw the right-side Inspector window, docked into rightDockID
    void DrawInspectorWindow(ImGuiID rightDockID);
    //inspector components
    void Inspector_NameComponent(Coordinator* g);
    void Inspector_TransformComponent(Coordinator* g);
    void Inspector_RenderableComponent(Coordinator* g);
    void Inspector_AnimationComponent(Coordinator* g);
    void Inspector_LogicComponent(Coordinator* g);
    void Inspector_AudioComponent(Coordinator* g);
    void Inspector_TextComponentUI(Coordinator* g);
}// end of namespace MyEditor
