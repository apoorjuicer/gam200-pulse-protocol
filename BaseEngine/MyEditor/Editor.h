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
    extern const Entity INVALID_ENTITY;
    extern Entity selectedEntity;
    static std::vector<std::string> s_pendingOSDrops;

    void EnqueueExternalDrop(const std::vector<std::string>& paths);
    void PushOSDropPath(const char* path);
    static bool ReplaceTextureFileOnDisk(const std::string& dstPath, const std::string& srcPath);
    static void RefreshCurrentDir();
    static bool ReplaceTextureFileOnDisk_AndRefreshAll(const std::string& dstPath,
        const std::string& srcPath);

    // Returns the middle "game" rectangle in FRAMEBUFFER PIXELS, computed from current panel sizes
    GameViewport ComputeMiddleGameViewportPixels(GLFWwindow* window, bool keep_16x9 = true);

    // Simple editor-gizmo state
    struct GizmoState {
        bool  dragging = false;
        Entity active = INVALID_ENTITY;
        glm::vec2 dragStartMouseScreen{ 0,0 }; // ImGui (screen) px
        Vector2  dragStartWorld{ 0,0 };        // world coords at press
        Vector2  entityStartPos{ 0,0 };        // starting entity world pos
    };

    // world<->screen helpers
    Vector2 ScreenToWorld(glm::vec2 screen, const GameViewport& vp);
    glm::vec2 WorldToScreen(const Vector2& world, const GameViewport& vp);

    void Gizmo_DrawAndHandle(const GameViewport& vp);

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
