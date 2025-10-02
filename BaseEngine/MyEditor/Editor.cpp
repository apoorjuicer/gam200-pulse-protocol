/******************************************************************************/
/**
 * @file            Editor.cpp
 * @project			Pulse Protocol
 * @author          Leu Jun Yong
 * @brief			Update the brief of this file
 * @copyright		Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "pch/pch.h"

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
