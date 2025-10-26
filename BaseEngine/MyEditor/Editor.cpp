/******************************************************************************/
/**
 * @file        Editor.cpp
 * @project     Pulse Protocol
 * @author      Leu Jun Yong
 * @brief       Initializes, updates, renders, and manages the ImGui
                editor interface with toggle support.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "../../imgui/imgui.h"
#include "../../imgui/backends/imgui_impl_glfw.h"
#include "../../imgui/backends/imgui_impl_opengl3.h"
#include "Editor.h"
#include <Graphics/glhelper.h>
#include <GLFW/glfw3.h> 

namespace MyEditor {

    static GLFWwindow* s_Window = nullptr;
    static bool s_enabled = true;

    static float s_left_ratio = 0.20f;
    static float s_right_ratio = 0.15f;
    static float s_bottom_ratio = 0.35f;

    static constexpr float SPLIT_THICK = 6.0f;
    static constexpr float MIN_RATIO_W = 0.12f;
    static constexpr float MIN_RATIO_H = 0.15f;

    template<typename T>
    static T clamp_val(T v, T lo, T hi) { return (v < lo) ? lo : (v > hi) ? hi : v; }

    // Left splitter: drags the RIGHT edge of the Left panel => modifies left_ratio
    static void SplitterV_Left(const char* id, float x_px, float top_h_px, float full_w_px, float& left_ratio,
        float right_ratio_limit)
    {
        ImGui::SetCursorScreenPos(ImVec2(x_px - SPLIT_THICK * 0.5f, 0.0f));
        ImGui::InvisibleButton(id, ImVec2(SPLIT_THICK, top_h_px));
        if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        if (ImGui::IsItemActive()) {
            left_ratio += ImGui::GetIO().MouseDelta.x / full_w_px;
            left_ratio = clamp_val(left_ratio, MIN_RATIO_W, 1.0f - right_ratio_limit - MIN_RATIO_W);
        }
        ImGui::GetForegroundDrawList()->AddLine(ImVec2(x_px, 0.0f), ImVec2(x_px, top_h_px),
            IM_COL32(180, 180, 180, 160), 1.0f);
    }

    // Right splitter: drags the LEFT edge of the Right panel => modifies right_ratio
    static void SplitterV_Right(const char* id, float x_px, float top_h_px, float full_w_px, float& right_ratio,
        float left_ratio_limit)
    {
        ImGui::SetCursorScreenPos(ImVec2(x_px - SPLIT_THICK * 0.5f, 0.0f));
        ImGui::InvisibleButton(id, ImVec2(SPLIT_THICK, top_h_px));
        if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        if (ImGui::IsItemActive()) {
            float edge = x_px + ImGui::GetIO().MouseDelta.x;        // new x of right panel’s left edge
            right_ratio = (full_w_px - edge) / full_w_px;           // convert to ratio
            right_ratio = clamp_val(right_ratio, MIN_RATIO_W, 1.0f - left_ratio_limit - MIN_RATIO_W);
        }
        ImGui::GetForegroundDrawList()->AddLine(ImVec2(x_px, 0.0f), ImVec2(x_px, top_h_px),
            IM_COL32(180, 180, 180, 160), 1.0f);
    }

    // Bottom splitter: drags the TOP edge of the bottom area => modifies bottom_ratio
    static void SplitterH_Bottom(const char* id, float y_px, float full_w_px, float full_h_px, float& bottom_ratio)
    {
        ImGui::SetCursorScreenPos(ImVec2(0.0f, y_px - SPLIT_THICK * 0.5f));
        ImGui::InvisibleButton(id, ImVec2(full_w_px, SPLIT_THICK));
        if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        if (ImGui::IsItemActive()) {
            float edge = y_px + ImGui::GetIO().MouseDelta.y; // new y of top of bottom area
            bottom_ratio = (full_h_px - edge) / full_h_px;
            bottom_ratio = clamp_val(bottom_ratio, MIN_RATIO_H, 0.7f);
        }
        ImGui::GetForegroundDrawList()->AddLine(ImVec2(0.0f, y_px), ImVec2(full_w_px, y_px),
            IM_COL32(180, 180, 180, 160), 1.0f);
    }


    void Init(GLFWwindow* w) {
        s_Window = w;
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(s_Window, true);
        ImGui_ImplOpenGL3_Init("#version 130");
        s_enabled = true;
    }


    void SetEnabled(bool enabled) { s_enabled = enabled; }
    bool IsEnabled() { return s_enabled; }

    void NewFrame() {
        if (!s_enabled) return;
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Draw() {
        if (!s_enabled) return;
        // ImGui::Begin("Editor");
        // ImGui::TextUnformatted("Hello from MyEditor!");
        // ImGui::Separator();

        // ImGui::TextUnformatted("Press O to toggle ImGui.");
        // ImGui::Separator();

        // // ===== PROFILER TAB =====
        // if (ImGui::BeginTabBar("##BottomTabs", ImGuiTabBarFlags_None)) {
        //     if (ImGui::BeginTabItem("Profiler")) {
        //         // Get profiling data
        //         auto* profiler = ProfilingManager::GetInstance();
        //         const auto& profiles = profiler->GetDisplayProfiles();

        //         ImGui::TextUnformatted("Performance Metrics:");
        //         ImGui::Separator();
        //         ImGui::Text("FPS: %.1f", GLHelper::fps);
        //         ImGui::Text("Frame Time: %.2f ms", profiler->GetCachedFrameTime() * 1000.0);
        //         //ImGui::Text("Memory Usage: TBD");
        //         ImGui::Separator();
        //         ImGui::TextUnformatted("System Stats:");

        //         // Display profiling data for each system
        //         if (!profiles.empty()) {
        //             ImGui::BeginChild("ProfilesChild", ImVec2(0, 200), true);
        //             for (const auto& [systemName, profile] : profiles) {
        //                 ImGui::Text("%s: %.2f%% (%.3f ms)",
        //                     systemName.c_str(),
        //                     profile.percentage,
        //                     profile.totalTime * 1000.0);
        //             }
        //             ImGui::EndChild();
        //         }
        //         else {
        //             ImGui::TextColored(ImVec4(1, 1, 0, 1), "No profiling data yet...");
        //         }

        //         ImGui::EndTabItem();
        //     }

        //     ImGui::EndTabBar();
        // }


        ImGuiIO& io = ImGui::GetIO();
        const float W = io.DisplaySize.x;
        const float H = io.DisplaySize.y;

        // current pixel sizes
        const float assets_h = H * s_bottom_ratio;
        const float top_h = H - assets_h;
        const float left_w = W * s_left_ratio;
        const float right_w = W * s_right_ratio;
        const float center_w = (std::max)(0.0f, W - left_w - right_w);


        ImGuiWindowFlags panel_flags =
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

        // ---- LEFT (fixed to left edge) ----
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(left_w, top_h));
        if (ImGui::Begin("LeftPanel", nullptr, panel_flags)) {
            const float scene_h = top_h * 0.5f;
            const float entity_h = top_h - scene_h;

            ImGui::BeginChild("Scene", ImVec2(-FLT_MIN, scene_h), true);
            ImGui::TextUnformatted("Scene hierarchy goes here...");
            ImGui::EndChild();

            ImGui::Separator();

            ImGui::BeginChild("Entity", ImVec2(-FLT_MIN, entity_h), true);
            ImGui::TextUnformatted("Selected entity list/details...");
            ImGui::EndChild();
        }
        ImGui::End();

        // ---- RIGHT (fixed to right edge) ----
        ImGui::SetNextWindowPos(ImVec2(W - right_w, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(right_w, top_h));
        if (ImGui::Begin("Component", nullptr, panel_flags)) {
            ImGui::TextUnformatted("Inspector: Transform, Sprite, etc.");
        }
        ImGui::End();

        // ---- BOTTOM (full width), with TABS: Assets | Debug ----
        ImGui::SetNextWindowPos(ImVec2(0.0f, top_h));
        ImGui::SetNextWindowSize(ImVec2(W, assets_h));
        if (ImGui::Begin("Bottom", nullptr, panel_flags)) {
            if (ImGui::BeginTabBar("BottomTabs")) {
                if (ImGui::BeginTabItem("Assets")) {
                    ImGui::TextUnformatted("Asset browser / thumbnails / filters.");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Debug")) {
                    ImGui::TextUnformatted("Debug info / logs / toggles...");
                    // put your debug UI here
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();

        // ---- SPLITTERS (overlay so they don't create any windows/tabs) ----
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(W, H));
        ImGuiWindowFlags overlay_flags =
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("LayoutOverlay", nullptr, overlay_flags);
        // between left and center
        SplitterV_Left("##splitV_left", left_w, top_h, W, s_left_ratio, s_right_ratio);
        // between center and right
        SplitterV_Right("##splitV_right", W - right_w, top_h, W, s_right_ratio, s_left_ratio);
        // between top area and bottom tabs
        SplitterH_Bottom("##splitH_bottom", top_h, W, H, s_bottom_ratio);
        ImGui::End();
    }



    void Render() {
        if (!s_enabled) return;
        ImGui::Render();
        if (ImDrawData* dd = ImGui::GetDrawData())
            ImGui_ImplOpenGL3_RenderDrawData(dd);
    }

    void Shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        s_Window = nullptr;
    
    }

    extern float s_left_ratio;
    extern float s_right_ratio;
    extern float s_bottom_ratio;

    GameViewport ComputeGameViewportPixels(GLFWwindow* window, bool keep_16x9)
    {
        ImGuiIO& io = ImGui::GetIO();

        // ImGui logical size
        const float w = io.DisplaySize.x;
        const float h = io.DisplaySize.y;

        // Current panel sizes from live ratios
        const float assets_h = h * s_bottom_ratio;
        const float top_h = h - assets_h;

        const float left_w = w * s_left_ratio;
        const float right_w = w * s_right_ratio;
        const float center_w = std::max(0.0f, w - left_w - right_w);

        // Convert to framebuffer pixels (HiDPI aware)
        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        const float sx = io.DisplayFramebufferScale.x;
        const float sy = io.DisplayFramebufferScale.y;

        int vx = (int)std::round(left_w * sx);
        int vy = (int)std::round(assets_h * sy);
        int vw = (int)std::round(center_w * sx);
        int vh = (int)std::round(top_h * sy);

        // Optional: keep 16:9 and center inside the middle area
        if (keep_16x9 && vw > 0 && vh > 0) {
            const float target = 16.0f / 9.0f;
            if ((float)vh > (float)vw / target) {
                vh = (int)std::round(vw / target);
            }
            else {
                vw = (int)std::round(vh * target);
            }
            const int midW = (int)std::round(center_w * sx);
            const int midH = (int)std::round(top_h * sy);
            vx += (midW - vw) / 2;
            vy += (midH - vh) / 2;
        }

        return { vx, vy, vw, vh };
    }



}
