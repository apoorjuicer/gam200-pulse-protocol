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

        //bool hasGuiButton;
        //GUIButton guiButton;

        SavedEntityState()
            : id(INVALID_ENTITY),
            hasTransform(false),
            hasRenderable(false),
            hasName(false)
            //hasGuiButton(false)
        {
        }
    };

    static bool s_isPlaying = false;               
    static bool s_hasEditorBaseline = false;       
    static std::vector<SavedEntityState> s_editorBaseline;

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
            std::string path = OpenJsonFileDialog();
            if (!path.empty())
            {
                if (auto sceneMgr = Coordinator::GetInstance()->GetSystem<SceneManager>())
                {
                    sceneMgr->LoadSceneFromJson(path);
                    std::cout << "[Editor] Loaded scene: " << path << "\n";
                    s_hasEditorBaseline = false;
                    selectedEntity = INVALID_ENTITY;
                }
            }
            else
            {
                std::cout << "[Editor] Load scene cancelled.\n";
            }
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
                std::cout << "[Editor] Play Started\n";
            }
            else
            {
                // STOP PLAYING
                Engine->StopGame();
                s_isPlaying = false;
                isPaused = false;
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

        // for the layer dropdown list
        static int s_layerFilter = 3; // 3 = Show All
        const char* layerOptions[] = { "Background", "World", "UI", "All" };

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
        if (ImGui::Combo("##LayerFilter", &s_layerFilter, layerOptions, 4)) {
            auto graphicsMgr = Coordinator::GetInstance()->GetSystem<GraphicsManager>();
            if (graphicsMgr) {
                LayerMask filterMask = LAYER_ALL;
                switch (s_layerFilter) {
                case 0: filterMask = LAYER_BACKGROUND; break;
                case 1: filterMask = LAYER_WORLD; break;
                case 2: filterMask = LAYER_UI; break;
                case 3: filterMask = LAYER_ALL; break;
                }
                graphicsMgr->SetEditorLayerFilter(filterMask);/*
                std::cout << "[TopBar] Layer filter: " << layerOptions[s_layerFilter] << std::endl;*/
            }
        }

        ImGui::PopStyleVar();
        ImGui::End();
    }

} // namespace MyEditor
