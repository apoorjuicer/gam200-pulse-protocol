// -----------------------------------------------------------------------------
// TopBarPanel.cpp  -  Editor top toolbar (Play, Pause, Save/Load scene)
// -----------------------------------------------------------------------------

#include "pch/pch_temp.h"   // must stay first (PCH)

#define NOMINMAX
#include <Windows.h>
#include <commdlg.h>

// STL
#include <filesystem>
#include <string>

// Editor / engine
#include "TopBarPanel.h"
#include "Editor.h"
#include "LayerManagerPanel.h"  // For syncing layer visibility
#include <imgui.h>
#include "../CoreEngine/ECS/Coordinator.h"


namespace MyEditor
{

    struct SavedEntityState
    {
        Entity id;

        bool hasTransform;
        Framework::Transform transform;

        bool hasRenderable;
        Framework::Renderable renderable;

        bool hasName;
        Name name;

        SavedEntityState()
            : id(INVALID_ENTITY),
            hasTransform(false),
            hasRenderable(false),
            hasName(false)
        {
        }
    };

    static bool s_isPlaying = false;
    static bool s_hasEditorBaseline = false;
    static std::vector<SavedEntityState> s_editorBaseline;

    bool IsPlaying()
    {
        return s_isPlaying;
    }

    void CaptureEditorBaseline()
    {
        s_editorBaseline.clear();

        auto* coord = Coordinator::GetInstance();
        if (!coord)
            return;

        for (auto e : coord->GetAllEntities())
        {
            SavedEntityState st;
            st.id = e;

            if (coord->HasComponent<Framework::Transform>(e))
            {
                st.hasTransform = true;
                st.transform = coord->GetComponent<Framework::Transform>(e);
            }

            if (coord->HasComponent<Framework::Renderable>(e))
            {
                st.hasRenderable = true;
                st.renderable = coord->GetComponent<Framework::Renderable>(e);
            }

            if (coord->HasComponent<Name>(e))
            {
                st.hasName = true;
                st.name = coord->GetComponent<Name>(e);
            }

            //if (coord->HasComponent<GUIButton>(e))
            //{
            //    st.hasGuiButton = true;
            //    st.guiButton = coord->GetComponent<GUIButton>(e);
            //}

            s_editorBaseline.push_back(std::move(st));
        }

        s_hasEditorBaseline = true;
        std::cout << "[Editor] Captured baseline with "
            << s_editorBaseline.size() << " entities.\n";
    }

    void RestoreEditorBaseline()
    {
        if (!s_hasEditorBaseline)
            return;

        auto* coord = Coordinator::GetInstance();
        if (!coord)
            return;

        for (auto const& st : s_editorBaseline)
        {
            if (coord->HasComponent<Framework::Transform>(st.id) && st.hasTransform)
                coord->GetComponent<Framework::Transform>(st.id) = st.transform;

            if (coord->HasComponent<Framework::Renderable>(st.id) && st.hasRenderable)
                coord->GetComponent<Framework::Renderable>(st.id) = st.renderable;

            if (coord->HasComponent<Name>(st.id) && st.hasName)
                coord->GetComponent<Name>(st.id) = st.name;

            //if (coord->HasComponent<GUIButton>(st.id) && st.hasGuiButton)
            //    coord->GetComponent<GUIButton>(st.id) = st.guiButton;
        }

        std::cout << "[Editor] Restored baseline.\n";
    }

    std::string MyEditor::OpenJsonFileDialog()
    {
        char fileBuf[MAX_PATH] = { 0 };

        OPENFILENAMEA ofn{};
        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
        ofn.lpstrFile = fileBuf;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        ofn.lpstrTitle = "Select scene JSON";

        namespace fs = std::filesystem;
        std::string dir = (fs::current_path() / "../../MyEngine/JSON").lexically_normal().string();
        ofn.lpstrInitialDir = dir.c_str();


        if (GetOpenFileNameA(&ofn)) {
            return std::string(fileBuf);
        }
        return {};
    }

    void DrawTopBarPanel(float W, float top_y, float top_h)
    {
        // Static variable to defer scene loading after dialog closes
        static std::string s_pendingScenePath;

        ImGuiWindowFlags top_flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings;

        ImGui::SetNextWindowPos(ImVec2(0.0f, top_y));
        ImGui::SetNextWindowSize(ImVec2(W, top_h));

        if (!ImGui::Begin("TopBar", nullptr, top_flags))
        {
            ImGui::End();
            return;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));

        ImVec2 framePadding = ImGui::GetStyle().FramePadding;
        float itemSpacing = ImGui::GetStyle().ItemSpacing.x;

        // ------------------------------------------------------------
        // Left side: Save Scene / Load Scene
        // ------------------------------------------------------------
        ImVec2 saveTextSize = ImGui::CalcTextSize("Save Scene");
        ImVec2 loadTextSize = ImGui::CalcTextSize("Load Scene");
        ImVec2 saveButtonSize = ImVec2(saveTextSize.x + framePadding.x * 2, 0);
        ImVec2 loadButtonSize = ImVec2(loadTextSize.x + framePadding.x * 2, 0);

        if (ImGui::Button("Save Scene", saveButtonSize))
        {
            if (auto sceneMgr = Coordinator::GetInstance()->GetSystem<SceneManager>())
            {
                sceneMgr->SaveCurrentScene();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Load Scene", loadButtonSize))
        {
            s_pendingScenePath = OpenJsonFileDialog();
        }

        // ------------------------------------------------------------
        // Center: Play / Pause buttons
        // ------------------------------------------------------------
        const char* playStopLabel = s_isPlaying ? "Stop" : "Play";
        ImVec2 playStopTextSize = ImGui::CalcTextSize(playStopLabel);

        static bool isPaused = false;
        const char* pauseLabel = isPaused ? "Resume" : "Pause";
        ImVec2 pauseTextSize = ImGui::CalcTextSize(pauseLabel);

        ImVec2 playStopButtonSize = ImVec2(playStopTextSize.x + framePadding.x * 2 + 20.0f, 0);
        ImVec2 pauseButtonSize = ImVec2(pauseTextSize.x + framePadding.x * 2 + 20.0f, 0);

        float windowWidth = ImGui::GetWindowWidth();
        float totalButtonWidth = playStopButtonSize.x + pauseButtonSize.x + itemSpacing;
        float offset = (windowWidth - totalButtonWidth) * 0.5f;

        if (offset > 0.0f)
            ImGui::SameLine(offset);
        else
            ImGui::SameLine();

        // --- Play / Stop ---
        if (ImGui::Button(playStopLabel, playStopButtonSize))
        {
            if (!s_isPlaying)
            {
                // START PLAYING
                Engine->PlayGame();
                s_isPlaying = true;
                isPaused = false;

                // to prevent creating a new empty entity when transitioning
                selectedEntity = INVALID_ENTITY;

                std::cout << "[Editor] Play Started\n";
            }
            else
            {
                // STOP PLAYING
                Engine->StopGame();
                s_isPlaying = false;
                isPaused = false;

                // to prevent creating a new empty entity when transitioning
                selectedEntity = INVALID_ENTITY;

                std::cout << "[Editor] Stopped - Scene Restored\n";
            }
        }

        ImGui::SameLine();

        // --- Pause / Resume ---
        if (!s_isPlaying)
        {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button(pauseLabel, pauseButtonSize))
        {
            if (s_isPlaying)
            {
                isPaused = !isPaused;

                if (isPaused)
                {
                    Engine->PauseGame();
                    std::cout << "[Editor] Game Paused\n";
                }
                else
                {
                    Engine->ResumeGame();
                    std::cout << "[Editor] Game Resumed\n";
                }
            }
        }

        if (!s_isPlaying)
        {
            ImGui::EndDisabled();
        }

        if (!s_isPlaying && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("Start playing to enable Pause");
        }

        // ============================================================
        // Layer Visibility Dropdown (Synced with Layer Manager)
        // This dropdown controls the same visibility as Layer Manager
        // Both systems stay in sync
        // ============================================================

        // Read current visibility state from Layer Manager
        bool bgVisible = IsLayerVisible(LAYER_BACKGROUND);
        bool worldVisible = IsLayerVisible(LAYER_WORLD);
        bool uiVisible = IsLayerVisible(LAYER_UI);

        // Determine current selection
        static int s_layerFilter = 4; // Start with "All"

        // Update display to match Layer Manager state
        if (bgVisible && worldVisible && uiVisible) {
            s_layerFilter = 4; // All
        }
        else if (bgVisible && !worldVisible && !uiVisible) {
            s_layerFilter = 0; // Background only
        }
        else if (!bgVisible && worldVisible && !uiVisible) {
            s_layerFilter = 1; // World only
        }
        else if (!bgVisible && !worldVisible && uiVisible) {
            s_layerFilter = 2; // UI only
        }
        else {
            s_layerFilter = 3; // Custom combination
        }

        const char* layerOptions[] = { "Background", "World", "UI", "Custom", "All" };

        // Calculate positions for right-aligned placement
        ImVec2 layerTextSize = ImGui::CalcTextSize("Layers:");
        float comboWidth = 100.0f;
        float totalLayerWidth = layerTextSize.x + itemSpacing + comboWidth;

        // Position at right side of window with some padding
        float rightPadding = 20.0f;
        float layerX = windowWidth - totalLayerWidth - rightPadding;

        // Move cursor to right side
        ImGui::SameLine(layerX);

        ImGui::Text("Layers:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);

        int previousFilter = s_layerFilter;
        if (ImGui::Combo("##LayerFilter", &s_layerFilter, layerOptions, 5)) {
            // User changed the dropdown - update Layer Manager's internal state
            // Layer Manager will automatically call UpdateGraphicsFilter()

            switch (s_layerFilter) {
            case 0: // Background only
                SetAllLayersVisibility(true, false, false);
                break;
            case 1: // World only
                SetAllLayersVisibility(false, true, false);
                break;
            case 2: // UI only
                SetAllLayersVisibility(false, false, true);
                break;
            case 3: // Custom - don't change anything
                s_layerFilter = previousFilter; // Revert
                break;
            case 4: // All
                SetAllLayersVisibility(true, true, true);
                break;
            }
        }

        if (s_layerFilter == 3 && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Multiple layers visible - use Layer Manager to adjust");
        }
        // ============================================================

        ImGui::PopStyleVar();
        ImGui::End();

        if (!s_pendingScenePath.empty())
        {
            // Editor "Load Scene" button should stop the game
            // (This is an editor action, not in-game navigation)
            if (s_isPlaying)
            {
                std::cout << "[Editor] Stopping game before loading scene (editor action)\n";
                Engine->StopGame();
                s_isPlaying = false;
                isPaused = false;
            }

            if (auto sceneMgr = Coordinator::GetInstance()->GetSystem<SceneManager>())
            {
                sceneMgr->LoadSceneFromJson(s_pendingScenePath);
                std::cout << "[Editor] Loaded scene: " << s_pendingScenePath << "\n";
                s_hasEditorBaseline = false;
                selectedEntity = INVALID_ENTITY;
            }
            s_pendingScenePath.clear();
        }
    }

} // namespace MyEditor