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

namespace MyEditor {
    static GLFWwindow* s_Window = nullptr;
    static bool s_enabled = true; // default ON

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
        if (!s_enabled) return; // skip all ImGui work when disabled
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Draw() {
        if (!s_enabled) return;
        ImGui::Begin("Editor");
        ImGui::TextUnformatted("Hello from MyEditor!");
        ImGui::Separator();

        ImGui::TextUnformatted("Press O to toggle ImGui.");
        ImGui::Separator();

        // ===== PROFILER TAB =====
        if (ImGui::BeginTabBar("##BottomTabs", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("Profiler")) {
                // Get profiling data
                auto* profiler = ProfilingManager::GetInstance();
                const auto& profiles = profiler->GetDisplayProfiles();

                ImGui::TextUnformatted("Performance Metrics:");
                ImGui::Separator();
                ImGui::Text("FPS: %.1f", GLHelper::fps);
                ImGui::Text("Frame Time: %.2f ms", profiler->GetCachedFrameTime() * 1000.0);
                //ImGui::Text("Memory Usage: TBD");
                ImGui::Separator();
                ImGui::TextUnformatted("System Stats:");

                // Display profiling data for each system
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

        ImGui::End();
    }

    void Render() {
        if (!s_enabled) return;
        ImGui::Render();                                      // produce draw data
        if (ImDrawData* dd = ImGui::GetDrawData()) {          // guard against null
            ImGui_ImplOpenGL3_RenderDrawData(dd);
        }
    }

    void Shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        s_Window = nullptr;
    }
}
