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
#include <string>


namespace MyEditor {
    void Init(GLFWwindow* window);
    void NewFrame();
    void Draw();
    void Render();
    void Shutdown();
    void SetEnabled(bool enabled);
    bool IsEnabled();
    static std::string OpenJsonFileDialog(); 

    struct GameViewport { int x, y, w, h; };
    // Returns the middle "game" rectangle in FRAMEBUFFER PIXELS, computed from current panel sizes
    GameViewport ComputeMiddleGameViewportPixels(GLFWwindow* window, bool keep_16x9 = true);

    struct AssetItem
{
    std::string name;        // just the file/folder name
    std::string fullPath;    // absolute-ish path (../../MyEngine/Assets/...)
    bool        isDir = false;

    // thumbnail stuff (for files)
    unsigned int texID = 0;
    bool         triedLoad = false; // so we don't spam-load every frame
};


}
