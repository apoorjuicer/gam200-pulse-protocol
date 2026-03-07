/******************************************************************************/
/**
 * @file        TopBarPanel.cpp
 * @project     Pulse Protocol
 * @author      Leu Jun Yong
 * @brief       Implements the Top Panel for the editor.
 *
 * @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "pch/pch_temp.h"   // must stay first (PCH)

#define NOMINMAX
#include <Windows.h>
#include <commdlg.h>

// editor headers
#include "TopBarPanel.h"
#include "Editor.h"
#include "LayerManagerPanel.h"  // For syncing layer visibility
#include <imgui.h>

// engine headers
#include "CoreEngine/ECS/Coordinator.h"


namespace PulseEditor
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
            
    static bool s_hasEditorBaseline = false;     
    static Entity s_selectedBeforePlay = INVALID_ENTITY;
    static std::vector<SavedEntityState> s_editorBaseline;
    static std::string s_sceneBeforePlay;

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

            s_editorBaseline.push_back(std::move(st));
        }

        s_hasEditorBaseline = true;
        //std::cout << "[Editor] Captured baseline with "
        //    << s_editorBaseline.size() << " entities.\n";
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
        }

        //std::cout << "[Editor] Restored baseline.\n";
    }

    std::string PulseEditor::OpenJsonFileDialog()
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
        std::string dir = "../../PulseProtocol/JSON";
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

        // New Game popup state
        static bool s_showNewGamePopup = false;
        static char s_newGameNameBuf[128] = {};
        static std::string s_newGameResult;

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

        if (IsPlaying())
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        }

        if (ImGui::Button("Save Scene", saveButtonSize))
        {
            if (!IsPlaying()) // block the action manually
            {
                if (auto sceneMgr = Coordinator::GetInstance()->GetSystem<SceneManager>())
                    sceneMgr->SaveCurrentScene();
            }
        }

        if (IsPlaying())
        {
            ImGui::PopStyleColor(3);
            if (ImGui::IsItemHovered())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); // red-ish warning
                ImGui::SetTooltip("Stop the game first before saving!!!");
                ImGui::PopStyleColor();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Load Scene", loadButtonSize))
        {
            s_pendingScenePath = OpenJsonFileDialog();
        }

        ImGui::SameLine();

        if (ImGui::Button("New Game"))
        {
            s_showNewGamePopup = true;
            s_newGameNameBuf[0] = '\0';
            s_newGameResult.clear();
            ImGui::OpenPopup("New Game Project");
        }

        // ── New Game popup ─────────────────────────────────────────
        if (ImGui::BeginPopupModal("New Game Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Enter a name for the new game project:");
            ImGui::InputText("##gamename", s_newGameNameBuf, sizeof(s_newGameNameBuf));

            if (!s_newGameResult.empty())
            {
                ImGui::TextWrapped("%s", s_newGameResult.c_str());
            }

            if (ImGui::Button("Create") && s_newGameNameBuf[0] != '\0')
            {
                namespace fs = std::filesystem;

                // Find BaseEngine folder by walking up from current path
                fs::path cur = fs::current_path();
                while (cur.filename() != "BaseEngine" && cur.has_parent_path())
                    cur = cur.parent_path();

                std::string gameName(s_newGameNameBuf);
                fs::path gameRoot = cur / gameName;

                // Create folder structure
                std::error_code ec;
                fs::create_directories(gameRoot / "Assets" / "Textures", ec);
                fs::create_directories(gameRoot / "Assets" / "Sound",    ec);
                fs::create_directories(gameRoot / "Assets" / "Font",     ec);
                fs::create_directories(gameRoot / "Assets" / "Prefabs",  ec);
                fs::create_directories(gameRoot / "JSON",                 ec);

                // Write starter JSON files
                auto writeFile = [](const fs::path& p, const char* content) {
                    std::ofstream f(p);
                    if (f.is_open()) f << content;
                };

                writeFile(gameRoot / "JSON" / "config.json",
                    "{\n    \"config\": {\n        \"WindowWidth\": 1600,\n        \"WindowHeight\": 900,\n        \"ConfigPath\": \"\"\n    }\n}\n");
                writeFile(gameRoot / "JSON" / "texture.json",
                    "{\n    \"textureFilePaths\": []\n}\n");
                writeFile(gameRoot / "JSON" / "audio.json",
                    "{\n    \"audioFilePaths\": []\n}\n");
                writeFile(gameRoot / "JSON" / "animation.json",
                    "{\n    \"animationFilePaths\": []\n}\n");
                writeFile(gameRoot / "JSON" / "font.json",
                    "{\n    \"fontFilePaths\": []\n}\n");
                writeFile(gameRoot / "JSON" / "scene.json",
                    "{\n    \"sceneFilePaths\": []\n}\n");

                if (fs::exists(gameRoot / "JSON"))
                {
                    s_newGameResult = "Game project '" + gameName + "' created at:\n" + gameRoot.string()
                        + "\n\nTo use it: update the game project path in CoreEngine, rebuild, and restart.";
                }
                else
                {
                    s_newGameResult = "ERROR: Could not create project folder. Check permissions.";
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Close"))
            {
                s_showNewGamePopup = false;
                s_newGameResult.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // ------------------------------------------------------------
        // Center: Play / Pause buttons
        // ------------------------------------------------------------
        const char* playStopLabel = IsPlaying() ? "Stop" : "Play";
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

        // ═══════════════════════════════════════════════════════════════════════
        // Play / Stop Button
        // ═══════════════════════════════════════════════════════════════════════
        if (ImGui::Button(playStopLabel, playStopButtonSize))
        {
            if (!IsPlaying())
            {
                // PLAY PRESSED

                // Save selected entity
                s_selectedBeforePlay = selectedEntity;

                // Deselect to avoid issues
                selectedEntity = INVALID_ENTITY;

                // Save scene state
                if (auto sceneMgr = Coordinator::GetInstance()->GetSystem<SceneManager>())
                {
                    s_sceneBeforePlay = sceneMgr->SerializeToString();
                }

                Engine->PlayGame();
                SetPlaying(true);
                isPaused = false;
            }
            else
            {
                // STOP PRESSED

                Engine->StopGame();

                // Restore scene
                if (auto sceneMgr = Coordinator::GetInstance()->GetSystem<SceneManager>())
                {
                    if (!s_sceneBeforePlay.empty())
                    {
                        sceneMgr->LoadFromString(s_sceneBeforePlay);
                    }
                }

                // DON'T restore selected - just clear it
                selectedEntity = INVALID_ENTITY;

                s_sceneBeforePlay.clear();
                SetPlaying(false);
                isPaused = false;
            }
        }

        ImGui::SameLine();

        // --- Pause / Resume ---
        if (!IsPlaying())
        {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button(pauseLabel, pauseButtonSize))
        {
            if (IsPlaying())
            {
                isPaused = !isPaused;

                if (isPaused)
                {
                    Engine->PauseGame();
                    //std::cout << "[Editor] Game Paused\n";
                }
                else
                {
                    Engine->ResumeGame();
                    //std::cout << "[Editor] Game Resumed\n";
                }
            }
        }

        if (!IsPlaying())
        {
            ImGui::EndDisabled();
        }

        if (!IsPlaying() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("Start playing to enable Pause");
        }

        // ============================================================
        // Layer Visibility Dropdown (Synced with Layer Manager)
        // This dropdown controls the same visibility as Layer Manager
        // Both systems stay in sync
        // ============================================================

        auto& registry = LayerRegistry::Get();
        const auto& allLayers = registry.GetAllLayers();

        // Count how many layers are visible
        int visibleCount = 0;
        int singleVisibleIndex = -1;
        for (size_t i = 0; i < allLayers.size(); ++i)
        {
            if (allLayers[i].visible)
            {
                visibleCount++;
                singleVisibleIndex = static_cast<int>(i);
            }
        }

        // Determine current selection for dropdown
        // Options: [Layer0, Layer1, Layer2, ..., LayerN, "Custom", "All"]
        static int s_layerFilter = -1; // -1 means "All"

        if (visibleCount == static_cast<int>(allLayers.size()))
        {
            s_layerFilter = static_cast<int>(allLayers.size()) + 1; // "All" index
        }
        else if (visibleCount == 1)
        {
            s_layerFilter = singleVisibleIndex; // Single layer visible
        }
        else
        {
            s_layerFilter = static_cast<int>(allLayers.size()); // "Custom" index
        }

        // Build options: [Layer names..., "Custom", "All"]
        static std::vector<std::string> s_layerNames;
        static std::vector<const char*> s_layerOptions;

        s_layerNames.clear();
        s_layerOptions.clear();

        for (const auto& layer : allLayers)
        {
            s_layerNames.push_back(layer.name);
        }
        s_layerNames.push_back("Custom");
        s_layerNames.push_back("All");

        for (const auto& name : s_layerNames)
        {
            s_layerOptions.push_back(name.c_str());
        }

        // Calculate positions for right-aligned placement
        ImVec2 layerTextSize = ImGui::CalcTextSize("Layers:");
        float comboWidth = 130.0f;
        float totalLayerWidth = layerTextSize.x + itemSpacing + comboWidth;

        float rightPadding = 20.0f;
        float layerX = windowWidth - totalLayerWidth - rightPadding;

        ImGui::SameLine(layerX);

        ImGui::Text("Layers:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(comboWidth);

        int previousFilter = s_layerFilter;
        int customIndex = static_cast<int>(allLayers.size());
        int allIndex = static_cast<int>(allLayers.size()) + 1;

        if (ImGui::Combo("##LayerFilter", &s_layerFilter, s_layerOptions.data(), static_cast<int>(s_layerOptions.size())))
        {
            if (s_layerFilter == customIndex)
            {
                // Can't select "Custom" directly - revert
                s_layerFilter = previousFilter;
            }
            else if (s_layerFilter == allIndex)
            {
                // Show all layers
                registry.ShowAllLayers();
                UpdateGraphicsLayerFilter();
            }
            else if (s_layerFilter >= 0 && s_layerFilter < static_cast<int>(allLayers.size()))
            {
                // Show only selected layer
                for (size_t i = 0; i < allLayers.size(); ++i)
                {
                    registry.SetLayerVisibility(allLayers[i].mask, (i == static_cast<size_t>(s_layerFilter)));
                }
                UpdateGraphicsLayerFilter();
            }
        }

        if (s_layerFilter == customIndex && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Multiple layers visible - use Layer Manager to adjust");
        }
        // ============================================================

        ImGui::PopStyleVar();
        ImGui::End();

        if (!s_pendingScenePath.empty())
        {
            // Editor "Load Scene" button should stop the game
            // (This is an editor action, not in-game navigation)
            if (IsPlaying())
            {
                //std::cout << "[Editor] Stopping game before loading scene (editor action)\n";
                Engine->StopGame();
                SetPlaying(false);
                isPaused = false;
            }

            if (auto sceneMgr = Coordinator::GetInstance()->GetSystem<SceneManager>())
            {
                sceneMgr->LoadSceneFromJson(s_pendingScenePath);
                //std::cout << "[Editor] Loaded scene: " << s_pendingScenePath << "\n";
                s_hasEditorBaseline = false;
                selectedEntity = INVALID_ENTITY;
            }

            s_pendingScenePath.clear();
        }
    }

} // namespace PulseEditor