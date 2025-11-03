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

#define NOMINMAX        
#include <windows.h>
#include <commdlg.h>

#include <GLFW/glfw3.h> 
#include "DebugConsole.h"
#include "ProfilingTab.h"
#include "pch/pch.h"
#include "CoreEngine/ECS/System.h"   // For Entity type
#include "CoreEngine/ECS/Name.h"     // For display name in hierarchy list
#include "Graphics/Transform.h"


namespace MyEditor
{
    // --------------------------------------------------------------------
    // CONSTANTS & BASIC STATE
    // --------------------------------------------------------------------
    constexpr Entity INVALID_ENTITY = static_cast<Entity>(-1);
    static Entity selectedEntity = INVALID_ENTITY;

    static GLFWwindow* s_Window = nullptr;
    static bool s_enabled = true;
    static bool s_restoreNextFrame = false;

    // --------------------------------------------------------------------
    // PANEL RATIOS
    // --------------------------------------------------------------------
    static float s_top_ratio = 0.05f;     // ~5% of screen height for Play/Pause bar
    static float s_bottom_ratio = 0.30f;  // bottom assets/debug/profiler
    static float s_left_ratio = 0.20f;    // scene/entity panel
    static float s_right_ratio = 0.15f;   // inspector panel

    static constexpr float SPLIT_THICK = 6.0f;
    static constexpr float MIN_RATIO_W = 0.12f;   // min 12% for left/right widths
    static constexpr float MIN_RATIO_H = 0.05f;   // min 5% for top/bottom heights
    static constexpr float MAX_TOTAL_SIDE = 0.85f;
    static constexpr float MAX_TOTAL_VERT = 0.85f;

    // --------------------------------------------------------------------
    // ASSET BROWSER STATE
    // --------------------------------------------------------------------
    static std::string s_assetRoot = "../../MyEngine/Assets";
    static std::string s_currentDir = s_assetRoot;
    static std::vector<AssetItem> s_dirEntries;

    static float s_thumbSize = 72.0f;
    static float s_thumbPadding = 10.0f;

    // --------------------------------------------------------------------
    // PLAY MODE SNAPSHOT STATE
    // --------------------------------------------------------------------
    struct SavedEntityState
    {
        Entity id;

        bool hasTransform;
        Framework::Transform transform;

        bool hasRenderable;
        Framework::Renderable renderable;

        bool hasName;
        Name name;

        bool hasGuiButton;
        GUIButton guiButton;

        SavedEntityState()
            : id(INVALID_ENTITY),
              hasTransform(false),
              hasRenderable(false),
              hasName(false),
              hasGuiButton(false)
        {}
    };

    static bool s_isPlaying = false;                // current play mode or not
    static bool s_hasEditorBaseline = false;        // whether we have captured a snapshot
    static std::vector<SavedEntityState> s_editorBaseline;

    #ifdef IMGUI_HAS_DOCKING
        static ImGuiID   g_DockspaceID = 0;
        static ImGuiID   g_NodeLeft = 0, g_NodeRight = 0, g_NodeBottom = 0, g_NodeCenter = 0;
        static bool      g_DockLayoutBuilt = false;

        enum class DockTarget { Left, Right, Bottom, Center };
    #endif


    // --------------------------------------------------------------------
    // UTILITY
    // --------------------------------------------------------------------
    template<typename T>
    static T clamp_val(T v, T lo, T hi)
    {
        return (v < lo) ? lo : (v > hi) ? hi : v;
    }


    // helper to draw a 1px line
    static void DrawGuideLine(ImVec2 a, ImVec2 b) {
        ImGui::GetForegroundDrawList()->AddLine(
            a, b,
            IM_COL32(180, 180, 180, 160),
            1.0f
        );
    }

    static bool IsImageExt(const std::string& path)
    {
        auto dot = path.find_last_of('.');
        if (dot == std::string::npos) return false;
        std::string ext = path.substr(dot + 1);
        for (auto& c : ext) c = (char)tolower(c);

        // add more if your AssetsManager supports them
        return (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "tga" || ext == "dds");
    }

    static void RefreshCurrentDir()
    {
        s_dirEntries.clear();

        namespace fs = std::filesystem;
        std::error_code ec;

        if (!fs::exists(s_currentDir, ec))
            return;

        // 1) first, add ".." if not root
        if (fs::weakly_canonical(s_currentDir, ec) != fs::weakly_canonical(s_assetRoot, ec))
        {
            AssetItem up;
            up.name = "..";
            up.fullPath = fs::weakly_canonical(fs::path(s_currentDir).parent_path(), ec).string();
            up.isDir = true;
            s_dirEntries.push_back(up);
        }

        // 2) list current dir (non-recursive)
        for (auto const& entry : fs::directory_iterator(s_currentDir, ec))
        {
            AssetItem it;
            it.fullPath = entry.path().lexically_normal().string();
            it.name = entry.path().filename().string();
            it.isDir = entry.is_directory();
            s_dirEntries.push_back(std::move(it));
        }

        // 3) sort: folders first, then files alpha
        std::sort(s_dirEntries.begin(), s_dirEntries.end(),
            [](AssetItem const& a, AssetItem const& b)
            {
                if (a.isDir != b.isDir)
                    return a.isDir > b.isDir;
                return a.name < b.name;
            });
    }

    // Load thumbnail texture lazily using your AssetsManager
    static void EnsureThumbnailLoaded(AssetItem& item)
    {
        if (item.triedLoad) return;     // already tried
        item.triedLoad = true;

        if (item.isDir) return;         // no thumb for folder

        if (!IsImageExt(item.fullPath)) return;

        // this assumes your AssetsManager returns a GLuint / unsigned int
        item.texID = Framework::AssetsManager::GetOrLoad(item.fullPath);
    }

    static void DrawAssetsBrowserPanelUnity(Entity selectedEntity)
    {
        // top bar: path + refresh + view size
        {
            if (ImGui::Button("Refresh"))
                RefreshCurrentDir();
            ImGui::SameLine();

            // breadcrumb-like display (simple version)
            ImGui::TextUnformatted(s_currentDir.c_str());
            ImGui::SameLine();

            ImGui::SetNextItemWidth(120.0f);
            ImGui::SliderFloat("Size", &s_thumbSize, 40.0f, 160.0f, "%.0f");
        }

        ImGui::Separator();

        // scrolling region
        ImGui::BeginChild("AssetsGrid", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        float availX = ImGui::GetContentRegionAvail().x;
        float cellSize = s_thumbSize + s_thumbPadding;
        int   cols = (int)(availX / cellSize);
        if (cols < 1) cols = 1;

        int itemIndex = 0;

        for (auto& item : s_dirEntries)
        {
            // start new row
            if (itemIndex % cols != 0)
                ImGui::SameLine();

            ImGui::BeginGroup();
            ImGui::PushID(itemIndex);

            // 1) Draw the thumbnail
            ImVec2 thumbSize(s_thumbSize, s_thumbSize);

            if (item.isDir)
            {
                // Folder: just draw a big button
                if (ImGui::Button(item.name.c_str(), thumbSize))
                {
                    // go into folder
                    s_currentDir = item.fullPath;
                    RefreshCurrentDir();
                    ImGui::PopID();
                    ImGui::EndGroup();
                    // since we changed directory, we shouldn't keep drawing old list
                    ImGui::EndChild();
                    return;
                }
            }
            else
            {
                // File
                EnsureThumbnailLoaded(item);

                bool clicked = false;

                if (item.texID != 0)
                {
                    ImTextureID tid = (ImTextureID)(intptr_t)item.texID;

                    // make a unique ID for ImGui so it doesn't clash with other items
                    std::string btnId = "##assetTex_" + item.fullPath;

                    clicked = ImGui::ImageButton(
                        btnId.c_str(),      // <- string id (required by your ImGui)
                        tid,                // texture
                        ImVec2(s_thumbSize, s_thumbSize),
                        ImVec2(0, 1),       // uv0  (flip Y if your textures are upside-down)
                        ImVec2(1, 0)        // uv1
                        // you can add frame_padding, bg_col, tint_col if needed
                    );

                }
                else
                {
                    // no texture -> generic button with extension
                    if (ImGui::Button(item.name.c_str(), thumbSize))
                        clicked = true;
                }

                if (clicked)
                {
                    // if an entity is selected, assign this asset to its Renderable
                    if (selectedEntity != INVALID_ENTITY)
                    {
                        auto* g_coordinator = Coordinator::GetInstance();
                        if (g_coordinator && g_coordinator->HasComponent<Framework::Renderable>(selectedEntity))
                        {
                            auto& rend = g_coordinator->GetComponent<Framework::Renderable>(selectedEntity);
                            rend.spriteName = item.fullPath;                       // or make it relative
                            rend.textureID = Framework::AssetsManager::GetOrLoad(rend.spriteName);
                        }
                    }
                }

                // optional: right click popup to "Ping", "Copy path", etc.
                if (ImGui::BeginPopupContextItem("AssetContext"))
                {
                    if (ImGui::MenuItem("Copy path"))
                    {
                        ImGui::SetClipboardText(item.fullPath.c_str());
                    }
                    if (selectedEntity != INVALID_ENTITY)
                    {
                        if (ImGui::MenuItem("Assign to selected"))
                        {
                            auto* g_coordinator = Coordinator::GetInstance();
                            if (g_coordinator && g_coordinator->HasComponent<Framework::Renderable>(selectedEntity))
                            {
                                auto& rend = g_coordinator->GetComponent<Framework::Renderable>(selectedEntity);
                                rend.spriteName = item.fullPath;
                                rend.textureID = Framework::AssetsManager::GetOrLoad(rend.spriteName);
                            }
                        }
                    }
                    ImGui::EndPopup();
                }
            }

            // 2) Draw label under thumb (clipped)
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + s_thumbSize);
            ImGui::TextUnformatted(item.name.c_str());
            ImGui::PopTextWrapPos();

            ImGui::PopID();
            ImGui::EndGroup();

            itemIndex++;
        }

        ImGui::EndChild();
    }

    // take current ECS and remember it as "start state"
    static void CaptureEditorBaseline()
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

            if (coord->HasComponent<GUIButton>(e))
            {
                st.hasGuiButton = true;
                st.guiButton = coord->GetComponent<GUIButton>(e);
            }

            s_editorBaseline.push_back(std::move(st));
        }

        s_hasEditorBaseline = true;
        std::cout << "[Editor] Captured baseline with "
            << s_editorBaseline.size() << " entities.\n";
    }

    // push the remembered state back into ECS
    static void RestoreEditorBaseline()
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

            if (coord->HasComponent<GUIButton>(st.id) && st.hasGuiButton)
                coord->GetComponent<GUIButton>(st.id) = st.guiButton;
        }

        std::cout << "[Editor] Restored baseline.\n";
    }

#ifdef IMGUI_HAS_DOCKING
    static void BuildMainDockspace()
    {
        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::SetNextWindowViewport(vp->ID);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("DockSpaceHost", nullptr, flags);
        ImGui::PopStyleVar(2);

        g_DockspaceID = ImGui::GetID("MyEditorDockSpace");
        ImGui::DockSpace(g_DockspaceID, ImVec2(0, 0), ImGuiDockNodeFlags_None);

        ImGui::End();
    }

    static void EnsureDefaultDockLayout()
    {
        if (g_DockLayoutBuilt || g_DockspaceID == 0) return;

        ImGui::DockBuilderRemoveNode(g_DockspaceID);
        ImGui::DockBuilderAddNode(g_DockspaceID, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(g_DockspaceID, ImGui::GetMainViewport()->WorkSize);

        g_NodeCenter = g_DockspaceID;
        g_NodeLeft = ImGui::DockBuilderSplitNode(g_NodeCenter, ImGuiDir_Left, 0.20f, nullptr, &g_NodeCenter);
        g_NodeRight = ImGui::DockBuilderSplitNode(g_NodeCenter, ImGuiDir_Right, 0.20f, nullptr, &g_NodeCenter);
        g_NodeBottom = ImGui::DockBuilderSplitNode(g_NodeCenter, ImGuiDir_Down, 0.25f, nullptr, &g_NodeCenter);

        // Place your known windows by name (must match ImGui::Begin titles)
        ImGui::DockBuilderDockWindow("Scene Hierarchy", g_NodeLeft);
        ImGui::DockBuilderDockWindow("Inspector", g_NodeRight);
        ImGui::DockBuilderDockWindow("Bottom", g_NodeBottom);
        ImGui::DockBuilderDockWindow("TopBar", g_NodeCenter);

        ImGui::DockBuilderFinish(g_DockspaceID);
        g_DockLayoutBuilt = true;
    }

    static ImGuiID GetTargetNodeID(DockTarget t)
    {
        switch (t)
        {
        case DockTarget::Left:   return g_NodeLeft;
        case DockTarget::Right:  return g_NodeRight;
        case DockTarget::Bottom: return g_NodeBottom;
        default:                 return g_NodeCenter;
        }
    }

    static void Dock_MoveWindowTo(const char* window_name, DockTarget target)
    {
        if (g_DockspaceID == 0) return;
        EnsureDefaultDockLayout();
        ImGuiID node = GetTargetNodeID(target);
        if (node == 0) node = g_DockspaceID;
        ImGui::DockBuilderDockWindow(window_name, node);
        ImGui::DockBuilderFinish(g_DockspaceID);
    }

    static void Dock_FloatWindow(const char* window_name, ImVec2 size = ImVec2(640, 360))
    {
        ImGuiID float_node = ImGui::DockBuilderAddNode(0, ImGuiDockNodeFlags_None);
        ImGui::DockBuilderSetNodeSize(float_node, size);
        ImVec2 mouse = ImGui::GetMousePos();
        ImGui::DockBuilderSetNodePos(float_node, mouse);
        ImGui::DockBuilderDockWindow(window_name, float_node);
        ImGui::DockBuilderFinish(float_node);
    }
#endif


    // Left splitter: drags the RIGHT edge of the Left panel => modifies left_ratio
    //static void SplitterV_Left(const char* id, float x_px, float top_h_px, float full_w_px, float& left_ratio,
    //    float right_ratio_limit)
    //{
    //    ImGui::SetCursorScreenPos(ImVec2(x_px - SPLIT_THICK * 0.5f, 0.0f));
    //    ImGui::InvisibleButton(id, ImVec2(SPLIT_THICK, top_h_px));
    //    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    //    if (ImGui::IsItemActive()) {
    //        left_ratio += ImGui::GetIO().MouseDelta.x / full_w_px;
    //        left_ratio = clamp_val(left_ratio, MIN_RATIO_W, 1.0f - right_ratio_limit - MIN_RATIO_W);
    //    }
    //    ImGui::GetForegroundDrawList()->AddLine(ImVec2(x_px, 0.0f), ImVec2(x_px, top_h_px),
    //        IM_COL32(180, 180, 180, 160), 1.0f);
    //}

    // Right splitter: drags the LEFT edge of the Right panel => modifies right_ratio
    //static void SplitterV_Right(const char* id, float x_px, float top_h_px, float full_w_px, float& right_ratio,
    //    float left_ratio_limit)
    //{
    //    ImGui::SetCursorScreenPos(ImVec2(x_px - SPLIT_THICK * 0.5f, 0.0f));
    //    ImGui::InvisibleButton(id, ImVec2(SPLIT_THICK, top_h_px));
    //    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    //    if (ImGui::IsItemActive()) {
    //        float edge = x_px + ImGui::GetIO().MouseDelta.x;        // new x of right panel’s left edge
    //        right_ratio = (full_w_px - edge) / full_w_px;           // convert to ratio
    //        right_ratio = clamp_val(right_ratio, MIN_RATIO_W, 1.0f - left_ratio_limit - MIN_RATIO_W);
    //    }
    //    ImGui::GetForegroundDrawList()->AddLine(ImVec2(x_px, 0.0f), ImVec2(x_px, top_h_px),
    //        IM_COL32(180, 180, 180, 160), 1.0f);
    //}

    // Bottom splitter: drags the TOP edge of the bottom area => modifies bottom_ratio
    //static void SplitterH_Bottom(const char* id, float y_px, float full_w_px, float full_h_px, float& bottom_ratio)
    //{
    //    ImGui::SetCursorScreenPos(ImVec2(0.0f, y_px - SPLIT_THICK * 0.5f));
    //    ImGui::InvisibleButton(id, ImVec2(full_w_px, SPLIT_THICK));
    //    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
    //    if (ImGui::IsItemActive()) {
    //        float edge = y_px + ImGui::GetIO().MouseDelta.y; // new y of top of bottom area
    //        bottom_ratio = (full_h_px - edge) / full_h_px;
    //        bottom_ratio = clamp_val(bottom_ratio, MIN_RATIO_H, 0.7f);
    //    }
    //    ImGui::GetForegroundDrawList()->AddLine(ImVec2(0.0f, y_px), ImVec2(full_w_px, y_px),
    //        IM_COL32(180, 180, 180, 160), 1.0f);
    //}


    void Init(GLFWwindow* w) {
        s_Window = w;
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // optional
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // ✅ docking ON
        // IMPORTANT: keep viewports OFF so nothing pops outside the GLFW window
        // (turn this on later only if you want multi-window OS-level popouts)
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // ❌ leave disabled

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(w, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        s_enabled = true;
        RefreshCurrentDir();

        // Tiny runtime check (prints once)
        std::cout << "[ImGui] Docking: "
            << ((io.ConfigFlags & ImGuiConfigFlags_DockingEnable) ? "ON" : "OFF")
            << ", Viewports: "
            << ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) ? "ON" : "OFF")
            << "\n";
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

        if (s_restoreNextFrame) {
            RestoreEditorBaseline();
            s_restoreNextFrame = false;
        }

        ImGuiIO& io = ImGui::GetIO();
        const float W = io.DisplaySize.x;
        const float H = io.DisplaySize.y;

        // --- Clamp layout ratios
        s_top_ratio = clamp_val(s_top_ratio, MIN_RATIO_H, MAX_TOTAL_VERT);
        s_bottom_ratio = clamp_val(s_bottom_ratio, MIN_RATIO_H, MAX_TOTAL_VERT);
        if (s_top_ratio + s_bottom_ratio > MAX_TOTAL_VERT) s_bottom_ratio = MAX_TOTAL_VERT - s_top_ratio;

        s_left_ratio = clamp_val(s_left_ratio, MIN_RATIO_W, MAX_TOTAL_SIDE);
        s_right_ratio = clamp_val(s_right_ratio, MIN_RATIO_W, MAX_TOTAL_SIDE);
        if (s_left_ratio + s_right_ratio > MAX_TOTAL_SIDE) s_right_ratio = MAX_TOTAL_SIDE - s_left_ratio;

        const float top_h = H * s_top_ratio;
        const float bottom_h = H * s_bottom_ratio;
        const float middle_h = H - top_h - bottom_h;
        const float safe_middle_h = (middle_h < 1.0f ? 1.0f : middle_h);

        const float left_w = W * s_left_ratio;
        const float right_w = W * s_right_ratio;
        const float mid_w = W - left_w - right_w;
        const float safe_mid_w = (mid_w < 1.0f ? 1.0f : mid_w);

        const float top_y = 0.0f;
        const float middle_y = top_h;
        const float bottom_y = top_h + safe_middle_h;
        const float left_x = 0.0f;
        const float right_x = left_w + safe_mid_w;

        ImGuiWindowFlags panel_flags =
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings;

        // =========================
        // TOP BAR
        // =========================
        {
            ImGuiWindowFlags top_flags =
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse |
                panel_flags;

            ImGui::SetNextWindowPos(ImVec2(0.0f, top_y));
            ImGui::SetNextWindowSize(ImVec2(W, top_h));

            if (ImGui::Begin("TopBar", nullptr, top_flags)) {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));

                if (ImGui::Button("Play", ImVec2(70, 0))) {
                    if (!s_hasEditorBaseline) CaptureEditorBaseline(); else RestoreEditorBaseline();
                    Engine->RestartRuntimeForPlay();
                    Engine->SetPaused(false);
                    s_isPlaying = true;
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop", ImVec2(70, 0))) {
                    Engine->SetPaused(true);
                    s_isPlaying = false;
                    RestoreEditorBaseline();
                }

                ImGui::SameLine(); ImGui::Dummy(ImVec2(30, 0)); ImGui::SameLine();

                if (ImGui::Button("Save Scene", ImVec2(120, 0))) {
                    if (auto sceneMgr = Coordinator::GetInstance()->GetSystem<SceneManager>())
                        sceneMgr->SaveCurrentScene();
                }
                ImGui::SameLine();
                if (ImGui::Button("Load Scene", ImVec2(120, 0))) {
                    std::string path = OpenJsonFileDialog();
                    if (!path.empty()) {
                        if (auto sceneMgr = Coordinator::GetInstance()->GetSystem<SceneManager>()) {
                            sceneMgr->LoadSceneFromJson(path);
                            std::cout << "[Editor] Loaded scene: " << path << "\n";
                            s_hasEditorBaseline = false;
                            selectedEntity = INVALID_ENTITY;
                        }
                    }
                    else {
                        std::cout << "[Editor] Load scene cancelled.\n";
                    }
                }

                ImGui::SameLine(); ImGui::Dummy(ImVec2(30, 0)); ImGui::SameLine();
                ImGui::TextUnformatted("| Game View");

                ImGui::PopStyleVar();
            }
            ImGui::End();
        }

        // ======================================================
        // FIXED HOST PANELS WITH DOCKSPACES: Left / Right / Bottom
        // (no DockBuilder: we only create DockSpace IDs)
        // ======================================================

        // --- LEFT HOST ---
        ImGui::SetNextWindowPos(ImVec2(left_x, middle_y));
        ImGui::SetNextWindowSize(ImVec2(left_w, safe_middle_h));
        ImGuiID leftDockID = 0;
        if (ImGui::Begin("Left Panel", nullptr, panel_flags)) {
            leftDockID = ImGui::GetID("LeftDockSpace");
            ImGui::DockSpace(leftDockID, ImVec2(0, 0));
        }
        ImGui::End();

        // --- RIGHT HOST ---
        ImGui::SetNextWindowPos(ImVec2(right_x, middle_y));
        ImGui::SetNextWindowSize(ImVec2(right_w, safe_middle_h));
        ImGuiID rightDockID = 0;
        if (ImGui::Begin("Right Panel", nullptr, panel_flags)) {
            rightDockID = ImGui::GetID("RightDockSpace");
            ImGui::DockSpace(rightDockID, ImVec2(0, 0));
        }
        ImGui::End();

        // --- BOTTOM HOST ---
        ImGui::SetNextWindowPos(ImVec2(0.0f, bottom_y));
        ImGui::SetNextWindowSize(ImVec2(W, bottom_h));
        ImGuiID bottomDockID = 0;
        if (ImGui::Begin("Bottom", nullptr, panel_flags)) {
            bottomDockID = ImGui::GetID("BottomDockSpace");
            ImGui::DockSpace(bottomDockID, ImVec2(0, 0));
        }
        ImGui::End();

        // =========================================
        // DOCKABLE TOOL WINDOWS (become tabs)
        // We "suggest" a default dock only on first run using ImGuiCond_FirstUseEver
        // =========================================

        // Scene Hierarchy
        if (leftDockID) ImGui::SetNextWindowDockID(leftDockID, ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Scene Hierarchy")) {
            auto* g = Coordinator::GetInstance();
            if (g) {
                ImGui::BeginChild("HierarchyList", ImVec2(-FLT_MIN, -FLT_MIN), true);
                for (auto entity : g->GetAllEntities()) {
                    std::string display;
                    if (g->HasComponent<Name>(entity)) {
                        display = g->GetComponent<Name>(entity).name;
                    }
                    else if (g->HasComponent<GUIButton>(entity)) {
                        display = g->GetComponent<GUIButton>(entity).buttonID;
                    }
                    else if (g->HasComponent<Framework::Renderable>(entity)) {
                        auto& rend = g->GetComponent<Framework::Renderable>(entity);
                        if (!rend.spriteName.empty()) {
                            size_t p = rend.spriteName.find_last_of("/\\");
                            display = (p != std::string::npos) ? rend.spriteName.substr(p + 1) : rend.spriteName;
                        }
                        else display = "Unnamed Renderable";
                    }
                    else {
                        display = "Entity " + std::to_string((int)entity);
                    }
                    bool isSelected = (selectedEntity == entity);
                    std::string label = display + "##" + std::to_string((int)entity);
                    if (ImGui::Selectable(label.c_str(), isSelected)) selectedEntity = entity;
                }
                ImGui::EndChild();
            }
            else {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Coordinator not found!");
            }
        }
        ImGui::End();

        // Inspector
        if (rightDockID) ImGui::SetNextWindowDockID(rightDockID, ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Inspector")) {
            ImGui::TextUnformatted("Entity Inspector");
            ImGui::Separator();

            auto* g = Coordinator::GetInstance();
            if (!g) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Coordinator not found!");
            }
            else if (selectedEntity == INVALID_ENTITY) {
                ImGui::TextUnformatted("No entity selected.");
            }
            else {
                ImGui::Text("Entity ID: %d", (int)selectedEntity);
                ImGui::Separator();

                if (g->HasComponent<Framework::Transform>(selectedEntity)) {
                    auto& tr = g->GetComponent<Framework::Transform>(selectedEntity);
                    ImGui::TextUnformatted("Transform");
                    ImGui::Separator();
                    float pos[2] = { tr.Pos.x, tr.Pos.y };
                    if (ImGui::DragFloat2("Position", pos, 0.5f)) { tr.Pos.x = pos[0]; tr.Pos.y = pos[1]; }
                    float sc[2] = { tr.Scale.x, tr.Scale.y };
                    if (ImGui::DragFloat2("Scale", sc, 0.5f)) { tr.Scale.x = sc[0]; tr.Scale.y = sc[1]; }
                    ImGui::DragFloat("Rotation (deg)", &tr.angle_disp, 1.0f, -360.f, 360.f);
                    ImGui::DragFloat("Rotation Speed", &tr.angle_speed, 0.1f, -10.0f, 10.0f);
                    ImGui::Checkbox("Visible", &tr.isVisible); ImGui::SameLine(); ImGui::Checkbox("Auto Hide", &tr.autoHide);
                    ImGui::Separator();
                }

                if (g->HasComponent<Framework::Renderable>(selectedEntity)) {
                    auto& r = g->GetComponent<Framework::Renderable>(selectedEntity);
                    ImGui::TextUnformatted("Renderable");
                    ImGui::Separator();
                    ImGui::Text("Model Ref: %.1f", r.mdl_ref);
                    ImGui::Text("Shader Ref: %.1f", r.shd_ref);
                    ImGui::Text("Texture ID: %u", r.textureID);
                    static char spriteBuf[256];
                    strncpy(spriteBuf, r.spriteName.c_str(), sizeof(spriteBuf)); spriteBuf[255] = '\0';
                    if (ImGui::InputText("Sprite Name", spriteBuf, IM_ARRAYSIZE(spriteBuf))) r.spriteName = spriteBuf;
                    if (ImGui::Button("Reload Texture")) r.textureID = Framework::AssetsManager::GetOrLoad(r.spriteName);
                    ImGui::Separator();
                }
            }
        }
        ImGui::End();

        // Assets
        if (bottomDockID) ImGui::SetNextWindowDockID(bottomDockID, ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Assets")) {
            DrawAssetsBrowserPanelUnity(selectedEntity);
        }
        ImGui::End();

        // Debug Console
        if (bottomDockID) ImGui::SetNextWindowDockID(bottomDockID, ImGuiCond_FirstUseEver);
        static DebugConsole debugConsole;
        if (ImGui::Begin("Debug Console")) {
            debugConsole.Draw();
            // put your debug UI here

            // Example on logging usage
            // Plain text
            debugConsole.AddLog("Hello plain text");

            // With level (colored automatically)
            debugConsole.AddLogLevel(ConsoleLevel::Info, "Loaded %d textures", &Systems::EntityMember);
            debugConsole.AddLogLevel(ConsoleLevel::Warn, "FPS is low: %.1f", GLHelper::fps);
            debugConsole.AddLogLevel(ConsoleLevel::Error, "Failed to open '%s'", "[filename]");

            //// With explicit color (custom)
            debugConsole.AddLogColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "Green success message");
        }
        ImGui::End();

        // Profiler
        if (bottomDockID) ImGui::SetNextWindowDockID(bottomDockID, ImGuiCond_FirstUseEver);
        static ProfilingPanel profilingPanel;
        if (ImGui::Begin("Profiler")) {
            profilingPanel.Draw();
        }
        ImGui::End();

        // =============================
        // SPLITTER OVERLAY (unchanged)
        // =============================
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

        // Top/Middle splitter
        {
            float bar_y = top_h;
            ImGui::SetCursorScreenPos(ImVec2(0.0f, bar_y - SPLIT_THICK * 0.5f));
            ImGui::InvisibleButton("##split_top_mid", ImVec2(W, SPLIT_THICK));
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            if (ImGui::IsItemActive()) {
                float new_edge = bar_y + ImGui::GetIO().MouseDelta.y;
                s_top_ratio = clamp_val(new_edge / H, MIN_RATIO_H, MAX_TOTAL_VERT);
                if (s_top_ratio + s_bottom_ratio > MAX_TOTAL_VERT) s_top_ratio = MAX_TOTAL_VERT - s_bottom_ratio;
            }
            DrawGuideLine(ImVec2(0.0f, bar_y), ImVec2(W, bar_y));
        }

        // Middle/Bottom splitter
        {
            float bar_y = bottom_y;
            ImGui::SetCursorScreenPos(ImVec2(0.0f, bar_y - SPLIT_THICK * 0.5f));
            ImGui::InvisibleButton("##split_mid_bottom", ImVec2(W, SPLIT_THICK));
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            if (ImGui::IsItemActive()) {
                float new_edge = bar_y + ImGui::GetIO().MouseDelta.y;
                float new_bottom_h = H - new_edge;
                s_bottom_ratio = clamp_val(new_bottom_h / H, MIN_RATIO_H, MAX_TOTAL_VERT);
                if (s_top_ratio + s_bottom_ratio > MAX_TOTAL_VERT) s_bottom_ratio = MAX_TOTAL_VERT - s_top_ratio;
            }
            DrawGuideLine(ImVec2(0.0f, bar_y), ImVec2(W, bar_y));
        }

        // Left/Middle splitter
        {
            float bar_x = left_w;
            ImGui::SetCursorScreenPos(ImVec2(bar_x - SPLIT_THICK * 0.5f, middle_y));
            ImGui::InvisibleButton("##split_left_mid", ImVec2(SPLIT_THICK, safe_middle_h));
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            if (ImGui::IsItemActive()) {
                float new_edge = bar_x + ImGui::GetIO().MouseDelta.x;
                s_left_ratio = clamp_val(new_edge / W, MIN_RATIO_W, MAX_TOTAL_SIDE);
                if (s_left_ratio + s_right_ratio > MAX_TOTAL_SIDE) s_left_ratio = MAX_TOTAL_SIDE - s_right_ratio;
            }
            DrawGuideLine(ImVec2(bar_x, middle_y), ImVec2(bar_x, middle_y + safe_middle_h));
        }

        // Middle/Right splitter
        {
            float bar_x = right_x;
            ImGui::SetCursorScreenPos(ImVec2(bar_x - SPLIT_THICK * 0.5f, middle_y));
            ImGui::InvisibleButton("##split_mid_right", ImVec2(SPLIT_THICK, safe_middle_h));
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            if (ImGui::IsItemActive()) {
                float new_edge = bar_x + ImGui::GetIO().MouseDelta.x;
                float new_right_w = (W - new_edge);
                s_right_ratio = clamp_val(new_right_w / W, MIN_RATIO_W, MAX_TOTAL_SIDE);
                if (s_left_ratio + s_right_ratio > MAX_TOTAL_SIDE) s_right_ratio = MAX_TOTAL_SIDE - s_left_ratio;
            }
            DrawGuideLine(ImVec2(bar_x, middle_y), ImVec2(bar_x, middle_y + safe_middle_h));
        }

        ImGui::End(); // LayoutOverlay
    }





    void Render() {
        if (!s_enabled) return;
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
}
