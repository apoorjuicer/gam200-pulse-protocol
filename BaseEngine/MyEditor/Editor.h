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
#include "../../imgui/imgui.h"
#include "../../imgui/backends/imgui_impl_glfw.h"
#include "../../imgui/backends/imgui_impl_opengl3.h"

#include "pch/pch_temp.h"
#include "DebugConsole.h"
#include "ProfilingTab.h"
#include "CoreEngine/Factory/Factory.h" // for creating/deleting entities
#include "CoreEngine/Prefab/PrefabManager.h"
#include "CoreEngine/ECS/System.h"   // For Entity type
#include "Components/DisplayName.h"     // For display name in hierarchy list
#include "Graphics/Transform.h"

struct GLFWwindow;

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
    bool isDir = false;

    // thumbnail stuff (for files)
    unsigned int texID = 0;
    bool triedLoad = false; // so we don't spam-load every frame
};


}
