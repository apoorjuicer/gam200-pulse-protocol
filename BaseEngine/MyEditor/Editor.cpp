/******************************************************************************/
/**
 * @file        Editor.cpp
 * @project     Pulse Protocol
 * @author      Leu Jun Yong
 * @author      Goh Pin Kai
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

    // --- PANEL RATIOS -------------------------------------------------
    static float s_top_ratio = 0.05f;  // ~5% of screen height for Play/Pause bar
    static float s_bottom_ratio = 0.30f;  // bottom assets/debug/profiler
    static float s_left_ratio = 0.20f;  // scene/entity panel
    static float s_right_ratio = 0.15f;  // inspector panel

    static constexpr float SPLIT_THICK = 6.0f;
    static constexpr float MIN_RATIO_W = 0.12f; // min 12% for left/right widths
    static constexpr float MIN_RATIO_H = 0.05f; // min 5% for top/bottom heights
    static constexpr float MAX_TOTAL_SIDE = 0.85f;
    static constexpr float MAX_TOTAL_VERT = 0.85f;

    template<typename T>
    static T clamp_val(T v, T lo, T hi) { return (v < lo) ? lo : (v > hi) ? hi : v; }

    // helper to draw a 1px line
    static void DrawGuideLine(ImVec2 a, ImVec2 b) {
        ImGui::GetForegroundDrawList()->AddLine(
            a, b,
            IM_COL32(180, 180, 180, 160),
            1.0f
        );
    }

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

        ImGuiIO& io = ImGui::GetIO();
        const float W = io.DisplaySize.x;
        const float H = io.DisplaySize.y;

        // ----------------------------
        // Resolve actual pixel sizes
        // ----------------------------
        // clamp ratios to prevent nonsense
        s_top_ratio = clamp_val(s_top_ratio, MIN_RATIO_H, MAX_TOTAL_VERT);
        s_bottom_ratio = clamp_val(s_bottom_ratio, MIN_RATIO_H, MAX_TOTAL_VERT);
        if (s_top_ratio + s_bottom_ratio > MAX_TOTAL_VERT)
        {
            // squish bottom if combined too large
            s_bottom_ratio = MAX_TOTAL_VERT - s_top_ratio;
        }

        s_left_ratio = clamp_val(s_left_ratio, MIN_RATIO_W, MAX_TOTAL_SIDE);
        s_right_ratio = clamp_val(s_right_ratio, MIN_RATIO_W, MAX_TOTAL_SIDE);
        if (s_left_ratio + s_right_ratio > MAX_TOTAL_SIDE)
        {
            // squish right if combined too large
            s_right_ratio = MAX_TOTAL_SIDE - s_left_ratio;
        }

        const float top_h = H * s_top_ratio;
        const float bottom_h = H * s_bottom_ratio;
        const float middle_h = H - top_h - bottom_h;
        // keep middle at least 1px high
        const float safe_middle_h = (middle_h < 1.0f ? 1.0f : middle_h);

        const float left_w = W * s_left_ratio;
        const float right_w = W * s_right_ratio;
        const float mid_w = W - left_w - right_w;
        const float safe_mid_w = (mid_w < 1.0f ? 1.0f : mid_w);

        // positions of the 4 bars
        const float top_y = 0.0f;
        const float middle_y = top_h;
        const float bottom_y = top_h + safe_middle_h;

        const float left_x = 0.0f;
        const float mid_x = left_w;
        const float right_x = left_w + safe_mid_w;

        // common flags
        ImGuiWindowFlags panel_flags =
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings;

        // ----------------------------
        // TOP BAR (Play/Pause)
        // ----------------------------
        {
            ImGuiWindowFlags top_flags =
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse |
                panel_flags;

            ImGui::SetNextWindowPos(ImVec2(0.0f, top_y));
            ImGui::SetNextWindowSize(ImVec2(W, top_h));
            if (ImGui::Begin("TopBar", nullptr, top_flags)) {
                // toolbar row
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));
                if (ImGui::Button("Play")) {
                    // TODO hook
                }
                ImGui::SameLine();
                if (ImGui::Button("Pause")) {
                    // TODO hook
                }
                ImGui::SameLine();
                ImGui::TextUnformatted("| Game View");
                ImGui::PopStyleVar();
            }
            ImGui::End();
        }

        // ----------------------------
        // LEFT PANEL (Scene/Entity)
        // spans the MIDDLE region vertically
        // ----------------------------
        ImGui::SetNextWindowPos(ImVec2(left_x, middle_y));
        ImGui::SetNextWindowSize(ImVec2(left_w, safe_middle_h));
        if (ImGui::Begin("LeftPanel", nullptr, panel_flags)) {
            float scene_h = safe_middle_h * 0.5f;
            float entity_h = safe_middle_h - scene_h;

            ImGui::BeginChild("Scene", ImVec2(-FLT_MIN, scene_h), true);
            ImGui::TextUnformatted("Scene hierarchy goes here...");
            ImGui::EndChild();

            ImGui::Separator();

            ImGui::BeginChild("Entity", ImVec2(-FLT_MIN, entity_h), true);
            ImGui::TextUnformatted("Selected entity list/details...");
            ImGui::EndChild();
        }
        ImGui::End();

        // ----------------------------
        // RIGHT PANEL (Inspector)
        // spans the MIDDLE region vertically
        // ----------------------------
        ImGui::SetNextWindowPos(ImVec2(right_x, middle_y));
        ImGui::SetNextWindowSize(ImVec2(right_w, safe_middle_h));
        if (ImGui::Begin("Component", nullptr, panel_flags)) {
            ImGui::TextUnformatted("Inspector: Transform, Sprite, etc.");
        }
        ImGui::End();

        // ----------------------------
        // BOTTOM PANEL (Assets/Debug/Profiler)
        // full width, bottom strip
        // ----------------------------
        ImGui::SetNextWindowPos(ImVec2(0.0f, bottom_y));
        ImGui::SetNextWindowSize(ImVec2(W, bottom_h));
        if (ImGui::Begin("Bottom", nullptr, panel_flags)) {
            if (ImGui::BeginTabBar("BottomTabs")) {

                if (ImGui::BeginTabItem("Assets")) {
                    ImGui::TextUnformatted("Asset browser / thumbnails / filters.");
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Debug")) {
                    ImGui::TextUnformatted("Debug info / logs / toggles...");
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Profiler")) {
                    auto* profiler = ProfilingManager::GetInstance();
                    const auto& profiles = profiler->GetDisplayProfiles();

                    ImGui::TextUnformatted("Performance Metrics:");
                    ImGui::Separator();
                    ImGui::Text("FPS: %.1f", GLHelper::fps);
                    ImGui::Text("Frame Time: %.2f ms", profiler->GetCachedFrameTime() * 1000.0);
                    ImGui::Separator();
                    ImGui::TextUnformatted("System Stats:");

                    if (!profiles.empty()) {
                        ImGui::BeginChild("ProfilesChild", ImVec2(0, 200), true);
                        for (const auto& [systemName, profile] : profiles) {
                            ImGui::Text("%s: %.2f%% (%.3f ms)",
                                systemName.c_str(),
                                profile.percentage,
                                profile.totalTime * 1000.0);
                        }
                        ImGui::EndChild();
                    }
                    else {
                        ImGui::TextColored(ImVec4(1, 1, 0, 1), "No profiling data yet...");
                    }

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();

        // ----------------------------
        // SPLITTER OVERLAY
        // ----------------------------
        ImGuiWindowFlags overlay_flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(W, H));
        ImGui::Begin("LayoutOverlay", nullptr, overlay_flags);

        //
        // 1. Horizontal splitter between TOP and MIDDLE
        //
        {
            float bar_y = top_h; // line between top and middle
            ImGui::SetCursorScreenPos(ImVec2(0.0f, bar_y - SPLIT_THICK * 0.5f));
            ImGui::InvisibleButton("##split_top_mid", ImVec2(W, SPLIT_THICK));
            if (ImGui::IsItemHovered())
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            if (ImGui::IsItemActive()) {
                float new_edge = bar_y + ImGui::GetIO().MouseDelta.y;
                // convert new_edge back to ratio of full height
                s_top_ratio = new_edge / H;
                // re-clamp, don't let top eat bottom/middle completely
                s_top_ratio = clamp_val(s_top_ratio, MIN_RATIO_H, MAX_TOTAL_VERT);
                // keep total with bottom in check
                if (s_top_ratio + s_bottom_ratio > MAX_TOTAL_VERT) {
                    s_top_ratio = MAX_TOTAL_VERT - s_bottom_ratio;
                }
            }

            DrawGuideLine(ImVec2(0.0f, bar_y), ImVec2(W, bar_y));
        }

        //
        // 2. Horizontal splitter between MIDDLE and BOTTOM
        //
        {
            float bar_y = bottom_y; // line between middle and bottom
            ImGui::SetCursorScreenPos(ImVec2(0.0f, bar_y - SPLIT_THICK * 0.5f));
            ImGui::InvisibleButton("##split_mid_bottom", ImVec2(W, SPLIT_THICK));
            if (ImGui::IsItemHovered())
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            if (ImGui::IsItemActive()) {
                float new_edge = bar_y + ImGui::GetIO().MouseDelta.y;
                // bottom_y = top_h + middle_h
                // => middle_h = new_edge - top_h
                float new_middle_h = new_edge - top_h;
                float new_bottom_h = H - new_edge;
                // convert new_bottom_h back to a ratio
                s_bottom_ratio = new_bottom_h / H;
                s_bottom_ratio = clamp_val(s_bottom_ratio, MIN_RATIO_H, MAX_TOTAL_VERT);
                if (s_top_ratio + s_bottom_ratio > MAX_TOTAL_VERT) {
                    s_bottom_ratio = MAX_TOTAL_VERT - s_top_ratio;
                }
            }

            DrawGuideLine(ImVec2(0.0f, bar_y), ImVec2(W, bar_y));
        }

        //
        // 3. Vertical splitter between LEFT and CENTER
        //
        {
            float bar_x = left_w; // line between left and middle region
            ImGui::SetCursorScreenPos(ImVec2(bar_x - SPLIT_THICK * 0.5f, middle_y));
            ImGui::InvisibleButton("##split_left_mid", ImVec2(SPLIT_THICK, safe_middle_h));
            if (ImGui::IsItemHovered())
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            if (ImGui::IsItemActive()) {
                float new_edge = bar_x + ImGui::GetIO().MouseDelta.x;
                s_left_ratio = new_edge / W;
                s_left_ratio = clamp_val(s_left_ratio, MIN_RATIO_W, MAX_TOTAL_SIDE);
                if (s_left_ratio + s_right_ratio > MAX_TOTAL_SIDE) {
                    s_left_ratio = MAX_TOTAL_SIDE - s_right_ratio;
                }
            }

            DrawGuideLine(ImVec2(bar_x, middle_y), ImVec2(bar_x, middle_y + safe_middle_h));
        }

        //
        // 4. Vertical splitter between CENTER and RIGHT
        //
        {
            float bar_x = right_x; // line between center and right
            ImGui::SetCursorScreenPos(ImVec2(bar_x - SPLIT_THICK * 0.5f, middle_y));
            ImGui::InvisibleButton("##split_mid_right", ImVec2(SPLIT_THICK, safe_middle_h));
            if (ImGui::IsItemHovered())
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            if (ImGui::IsItemActive()) {
                float new_edge = bar_x + ImGui::GetIO().MouseDelta.x;
                // right_ratio = width_of_right / W
                float new_right_w = (W - new_edge);
                s_right_ratio = new_right_w / W;
                s_right_ratio = clamp_val(s_right_ratio, MIN_RATIO_W, MAX_TOTAL_SIDE);
                if (s_left_ratio + s_right_ratio > MAX_TOTAL_SIDE) {
                    s_right_ratio = MAX_TOTAL_SIDE - s_left_ratio;
                }
            }

            DrawGuideLine(ImVec2(bar_x, middle_y), ImVec2(bar_x, middle_y + safe_middle_h));
        }

        ImGui::End(); // LayoutOverlay
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

    GameViewport ComputeMiddleGameViewportPixels(GLFWwindow* window, bool keep_16x9)
    {
        ImGuiIO& io = ImGui::GetIO();

        // 1. Get current ImGui logical size (in "UI pixels")
        float w = io.DisplaySize.x;
        float h = io.DisplaySize.y;

        if (w <= 0.f || h <= 0.f)
        {
            int winW = 0, winH = 0;
            glfwGetWindowSize(window, &winW, &winH);
            w = (float)std::max(1, winW);
            h = (float)std::max(1, winH);
        }

        // 2. Clamp ratios, but SWAP meaning:
        //    - s_bottom_ratio now drives the TOP bar height
        //    - s_top_ratio    now drives the BOTTOM panel height
        float clamped_top = clamp_val(s_bottom_ratio, MIN_RATIO_H, MAX_TOTAL_VERT);
        float clamped_bottom = clamp_val(s_top_ratio, MIN_RATIO_H, MAX_TOTAL_VERT);

        if (clamped_top + clamped_bottom > MAX_TOTAL_VERT)
        {
            // prevent overlap: shrink bottom if the combined is too big
            clamped_bottom = MAX_TOTAL_VERT - clamped_top;
        }

        float clamped_left = clamp_val(s_left_ratio, MIN_RATIO_W, MAX_TOTAL_SIDE);
        float clamped_right = clamp_val(s_right_ratio, MIN_RATIO_W, MAX_TOTAL_SIDE);
        if (clamped_left + clamped_right > MAX_TOTAL_SIDE)
        {
            // prevent overlap: shrink right if combined is too big
            clamped_right = MAX_TOTAL_SIDE - clamped_left;
        }

        // 3. Convert ratios -> absolute logical sizes
        const float topbar_h = h * clamped_top;        // NOW driven by s_bottom_ratio
        const float bottombar_h = h * clamped_bottom;     // NOW driven by s_top_ratio
        const float middle_h = h - topbar_h - bottombar_h;
        const float safe_mid_h = (middle_h < 1.0f ? 1.0f : middle_h);

        const float left_w = w * clamped_left;
        const float right_w = w * clamped_right;
        const float mid_w = w - left_w - right_w;
        const float safe_mid_w = (mid_w < 1.0f ? 1.0f : mid_w);

        // 4. Game area in logical ImGui coords
        //    It still starts after the "top bar" and ends before the "bottom bar".
        const float game_x = left_w;
        const float game_y = topbar_h;        // below (swapped) top bar
        const float game_w = safe_mid_w;
        const float game_h = safe_mid_h;      // above (swapped) bottom panel

        // 5. Convert to framebuffer coords
        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(window, &fbW, &fbH);

        const float sx = (w > 0.f) ? (float)fbW / w : 1.f;
        const float sy = (h > 0.f) ? (float)fbH / h : 1.f;

        int vx = (int)std::round(game_x * sx);
        int vy = (int)std::round(game_y * sy);
        int vw = (int)std::round(game_w * sx);
        int vh = (int)std::round(game_h * sy);

        // 6. Optional 16:9 fit inside the middle band
        if (keep_16x9 && vw > 0 && vh > 0)
        {
            const float target = 16.0f / 9.0f;
            if ((float)vh > (float)vw / target)
            {
                // area is too tall -> clamp height
                vh = (int)std::round(vw / target);
            }
            else
            {
                // area is too wide -> clamp width
                vw = (int)std::round(vh * target);
            }

            // recenter that (vw,vh) inside the mid band
            const int bandWpx = (int)std::round(game_w * sx);
            const int bandHpx = (int)std::round(game_h * sy);
            vx += (bandWpx - vw) / 2;
            vy += (bandHpx - vh) / 2;
        }

        // 7. Clamp to framebuffer bounds
        vw = std::clamp(vw, 1, fbW);
        vh = std::clamp(vh, 1, fbH);
        vx = std::clamp(vx, 0, std::max(0, fbW - vw));
        vy = std::clamp(vy, 0, std::max(0, fbH - vh));

        return { vx, vy, vw, vh };
    }





}
