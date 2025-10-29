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
#include <algorithm>
#include <cmath> 


namespace MyEditor {
    void Init(GLFWwindow* window);
    void NewFrame();
    void Draw();
    void Render();
    void Shutdown();
    void SetEnabled(bool enabled);
    bool IsEnabled();

    struct GameViewport { int x, y, w, h; };
    // Returns the middle "game" rectangle in FRAMEBUFFER PIXELS, computed from current panel sizes
    GameViewport ComputeGameViewportPixels(GLFWwindow* window, bool keep_16x9 = true);
}
