/******************************************************************************/
/**
 * @file        Editor.h
 * @project     Pulse Protocol
 * @author      Leu Jun Yong
 * @brief       Initializes, updates, renders, and manages the ImGui
                editor interface with toggle support.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#pragma once
struct GLFWwindow;

namespace MyEditor {
    void Init(GLFWwindow* window);   // call once after GL context is created
    void NewFrame();                 // per-frame (no-op when disabled)
    void Draw();                     // per-frame (no-op when disabled)
    void Render();                   // per-frame (no-op when disabled)
    void Shutdown();                 // call once on exit

    // --- Toggle API ---
    void SetEnabled(bool enabled);
    bool IsEnabled();
}
