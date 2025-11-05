/******************************************************************************/
/**
 * @file        Editor.cpp
 * @project     Pulse Protocol
 * @author      Leu Jun Yong(93%)
 * @author      Goh Pin Kai(7%)
 * @brief       Initializes, updates, renders, and manages the ImGui
                editor interface with toggle support.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "Editor.h"
#include <Graphics/glhelper.h>

#define NOMINMAX        
#include <windows.h>
#include <commdlg.h>

namespace MyEditor
{
    // --------------------------------------------------------------------
    // CONSTANTS & BASIC STATE
    // --------------------------------------------------------------------

    // the entity selected in the hierarchy; INVALID_ENTITY (-1) simply means nothing is selected
    constexpr Entity INVALID_ENTITY = static_cast<Entity>(-1);
    static Entity selectedEntity = INVALID_ENTITY;

    // this vector maintains the order entities appear in the hierarchy
    static std::vector<Entity> entityDisplayOrder;

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

    static void ProjectPanel(Entity selectedEntity)
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
                    // Check if this is a prefab JSON file
                    bool isPrefabFile = (item.fullPath.find("/Prefabs/") != std::string::npos ||
                        item.fullPath.find("\\Prefabs\\") != std::string::npos) &&
                        (item.fullPath.find(".json") != std::string::npos);

                    if (isPrefabFile)
                    {
                        // Instantiate prefab
                        PrefabManager prefabMgr;

                        // Extract prefab name from path
                        size_t lastSlash = item.fullPath.find_last_of("/\\");
                        size_t lastDot = item.fullPath.find_last_of(".");
                        std::string prefabName = item.fullPath.substr(lastSlash + 1, lastDot - lastSlash - 1);

                        auto entity = prefabMgr.InstantiatePrefab(prefabName);
                        if (entity.has_value()) {
                            std::cout << "[Project] Instantiated prefab: " << prefabName << " (Entity " << entity.value() << ")" << std::endl;
                        }
                    }
                    // if an entity is selected, assign this asset to its Renderable (for textures)
                    else if (selectedEntity != INVALID_ENTITY)
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

            // aesthetic purpose
            std::string displayName = item.name;
            if (displayName.ends_with(".json")) {
                displayName = displayName.substr(0, displayName.length() - 5);
            }

            ImGui::TextUnformatted(displayName.c_str());
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

    // Heuristic: look for "<rows>x<cols>" just before the file extension.
    static bool ParseGridFromFilename(const std::string& full, int& outRows, int& outCols)
    {
        outRows = 1; outCols = 1;

        if (full.empty()) return false;

        // grab the filename portion
        size_t slash = full.find_last_of("/\\");
        std::string filename = (slash != std::string::npos) ? full.substr(slash + 1) : full;

        // strip extension
        size_t dot = filename.find_last_of('.');
        std::string stem = (dot != std::string::npos) ? filename.substr(0, dot) : filename;

        // find an 'x' with digits on both sides, scanning from the end
        for (size_t i = stem.size(); i-- > 0; )
        {
            if (stem[i] != 'x' && stem[i] != 'X') continue;

            // parse left number
            size_t l = i;
            while (l > 0 && isdigit((unsigned char)stem[l - 1])) --l;
            if (l == i) continue; // no digits on the left

            // parse right number
            size_t r = i + 1;
            while (r < stem.size() && isdigit((unsigned char)stem[r])) ++r;
            if (r == i + 1) continue; // no digits on the right

            try {
                int rows = std::stoi(stem.substr(l, i - l));
                int cols = std::stoi(stem.substr(i + 1, r - (i + 1)));
                if (rows > 0 && cols > 0) {
                    outRows = rows;
                    outCols = cols;
                    return true;
                }
            }
            catch (...) {
                // ignore and continue searching
            }
        }
        return false;
    }


    void Init(GLFWwindow* w) {
        s_Window = w;
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // optional
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       //  docking ON
        // IMPORTANT: keep viewports OFF so nothing pops outside the GLFW window
        // (turn this on later only if you want multi-window OS-level popouts)
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   //  leave disabled

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

        // ----------------------------
        // TOP BAR (Play / Pause / Scene Controls)
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
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));

                ImVec2 framePadding = ImGui::GetStyle().FramePadding;
                float itemSpacing = ImGui::GetStyle().ItemSpacing.x;

                // Scene control buttons on the left
                ImVec2 saveTextSize = ImGui::CalcTextSize("Save Scene");
                ImVec2 loadTextSize = ImGui::CalcTextSize("Load Scene");
                ImVec2 saveButtonSize = ImVec2(saveTextSize.x + framePadding.x * 2, 0);
                ImVec2 loadButtonSize = ImVec2(loadTextSize.x + framePadding.x * 2, 0);

                if (ImGui::Button("Save Scene", saveButtonSize)) {
                    auto sceneMgr = Coordinator::GetInstance()->GetSystem<SceneManager>();
                    if (sceneMgr) {
                        sceneMgr->SaveCurrentScene();
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Load Scene", loadButtonSize)) {
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

                ImVec2 playTextSize = ImGui::CalcTextSize("Play");
                ImVec2 stopTextSize = ImGui::CalcTextSize("Stop");
                ImVec2 playButtonSize = ImVec2(playTextSize.x + framePadding.x * 2, 0);
                ImVec2 stopButtonSize = ImVec2(stopTextSize.x + framePadding.x * 2, 0);

                float windowWidth = ImGui::GetWindowWidth();
                float totalButtonWidth = playButtonSize.x + stopButtonSize.x + itemSpacing;
                float offset = (windowWidth - totalButtonWidth) * 0.5f;

                // Stay on same line, position at centered offset
                if (offset > 0.0f) {
                    ImGui::SameLine(offset);
                }
                else {
                    ImGui::SameLine();
                }

                // --- Play ---
                if (ImGui::Button("Play", playButtonSize))
                {
                    // restore ECS snapshot
                    if (!s_hasEditorBaseline)
                        CaptureEditorBaseline();
                    else
                        RestoreEditorBaseline();

                    Engine->RestartRuntimeForPlay();
                    Engine->SetPaused(false);
                    s_isPlaying = true;
                }

                ImGui::SameLine();

                // --- Stop ---
                if (ImGui::Button("Stop", stopButtonSize))
                {
                    Engine->SetPaused(true);
                    s_isPlaying = false;
                    RestoreEditorBaseline();
                }

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
        if (ImGui::Begin("Hierarchy", nullptr, panel_flags))
        {
            // Access ECS Coordinator
            auto* g_coordinator = Coordinator::GetInstance();
            if (g_coordinator)
            {
                // --- Entity Creation with Type Selection ---
                struct EntityPrefab {
                    const char* displayName;
                    const char* jsonPath;
                    bool isDefault;
                    float defaultScaleX;
                    float defaultScaleY;
                };

                static std::vector<EntityPrefab> entityPrefabs = {
                    {"Empty Entity", nullptr, true, 200.0f, 200.0f},
                    {"Hero", "../../MyEngine/JSON/HeroPrefab.json", false, 0, 0},
                    {"Monster", "../../MyEngine/JSON/MonsterPrefab.json", false, 0, 0},
                    {"Background", nullptr, true, 800.0f, 600.0f}
                };

                // which entity type to create (index into prefabs array)
                static int selectedType = 0;

                // Build display names for combo box
                std::vector<const char*> displayNames;
                for (const auto& prefab : entityPrefabs) {
                    displayNames.push_back(prefab.displayName);
                }

                // ============================================
                // UNITY-STYLE TOOLBAR
                // ============================================

                float windowWidth = ImGui::GetContentRegionAvail().x;
                float dropdownWidth = 120.0f;

                // toolbar buttons (left side)
                bool createClicked = ImGui::SmallButton("+");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Create at End");

                ImGui::SameLine();

                // disable the below, aboe and delete buttons if nothing is selected because no reference entity to insert the new entity between
                ImGui::BeginDisabled(selectedEntity == INVALID_ENTITY);

                bool createAboveClicked = ImGui::SmallButton("^");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Create Above Selected");

                ImGui::SameLine();
                bool createBelowClicked = ImGui::SmallButton("v");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Create Below Selected");

                ImGui::SameLine();
                bool deleteClicked = ImGui::SmallButton("X");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Delete Selected");

                ImGui::EndDisabled(); // this re-enables the UI for the buttons

                // this is to create the dropdown
                ImGui::SameLine(windowWidth - dropdownWidth + 5);
                ImGui::PushItemWidth(dropdownWidth);

                // selectedType is index into entityPrefabs array
                ImGui::Combo("##Type", &selectedType, displayNames.data(), static_cast<int>(displayNames.size()));
                ImGui::PopItemWidth();

                ImGui::Separator();

                // ============================================
                // HANDLE BUTTON CLICKS - Create Entity
                // ============================================

                if (createClicked || createAboveClicked || createBelowClicked)
                {
                    // initialize a new default game obj from class Factory and set the new Entity to non-selected first in drop-down
                    Factory defaultEntity;
                    Entity newEntity = INVALID_ENTITY;

                    // get the selected entity type from dropdown
                    const EntityPrefab& selectedPrefab = entityPrefabs[selectedType];

                    // then create the entity BASED on the type chosen
                    if (selectedPrefab.isDefault)
                    {
                        // this is for a simple default entity that only has these 3 components -> Transform, Renderable, Animation
                        newEntity = defaultEntity.DefaultObj();

                        // apply custom scale from prefab definition
                        if (g_coordinator->HasComponent<Framework::Transform>(newEntity)) {
                            auto& t = g_coordinator->GetComponent<Framework::Transform>(newEntity);
                            t.Scale = { selectedPrefab.defaultScaleX, selectedPrefab.defaultScaleY };
                        }
                    }
                    else if (selectedPrefab.jsonPath != nullptr)
                    {
                        // complex entity loaded from JSON -> Hero & Monster
                        auto result = defaultEntity.CreateEntityFromJson(selectedPrefab.jsonPath);
                        if (result.has_value()) {
                            newEntity = result.value();
                        }
                        else
                        {
                            // fallback if JSON load fails
                            /*std::cerr << "[Editor] Failed to load prefab: " << selectedPrefab.jsonPath << std::endl;*/
                            newEntity = defaultEntity.DefaultObj();
                        }
                    }
                    else
                    {
                        // fallback for edge cases
                        newEntity = defaultEntity.DefaultObj();
                    }

                    // this part is for ordering of the entities at the correct position
                    if (newEntity != INVALID_ENTITY) {

                        // [+] button: add to end of list (aka. at the very last position in the list)
                        if (createClicked) {
                            entityDisplayOrder.push_back(newEntity);
                            /*std::cout << "[Editor] Created entity " << newEntity << " at end" << std::endl;*/
                        }
                        
                        // [^] button: insert above selected entity
                        else if (createAboveClicked && selectedEntity != INVALID_ENTITY) {

                            // finds position of selected entity in the vector entityDisplayOrder
                            auto it = std::find(entityDisplayOrder.begin(), entityDisplayOrder.end(), selectedEntity);
                            if (it != entityDisplayOrder.end()) {

                                // insert at selected position (and pushes selected down)
                                // etc. [A, B, C] + insert at B → [A, NEW, B, C]
                                entityDisplayOrder.insert(it, newEntity);
                                /*std::cout << "[Editor] Created entity " << newEntity << " above selected" << std::endl;*/
                            }
                            else {
                                // if selected not found, then just add at end of the list
                                entityDisplayOrder.push_back(newEntity);
                            }
                        }

                        // [v] button: insert below selected entity
                        else if (createBelowClicked && selectedEntity != INVALID_ENTITY) {

                            // finds position of selected entity
                            auto it = std::find(entityDisplayOrder.begin(), entityDisplayOrder.end(), selectedEntity);
                                
                            // insert after selected position
                            // etc. [A, B, C] + insert below B → [A, NEW, B, C]
                            if (it != entityDisplayOrder.end()) {
                                entityDisplayOrder.insert(it + 1, newEntity);
                                /*std::cout << "[Editor] Created entity " << newEntity << " below selected" << std::endl;*/
                            }
                            else {
                                entityDisplayOrder.push_back(newEntity);
                            }
                        }

                        // automatically select newly created entity; user can immediately edit or delete what they just created
                        selectedEntity = newEntity;
                    }
                }

                // ============================================
                // HANDLE BUTTON CLICKS - Delete Entity
                // ============================================
                if (deleteClicked && selectedEntity != INVALID_ENTITY)
                {

                    // remove from display order vector (entityDisplayOrder); vector must stay in sync with actual entities
                    auto it = std::find(entityDisplayOrder.begin(), entityDisplayOrder.end(), selectedEntity);
                    if (it != entityDisplayOrder.end()) {

                        // opertion used: vector.erase(position)
                        // etc. [A, B, C] + erase B → [A, C]
                        entityDisplayOrder.erase(it);
                    }

                    // delete entity from ECS system; so this means the vector only TRACKS ORDER, but Factory handles ACTUAL DELETIION of the entity
                    Factory defaultEntity;
                    defaultEntity.DeleteEntity(selectedEntity);

                    // need to clear selection because cannot keep selecting an entity that does not exist as it has been deleted already
                    selectedEntity = INVALID_ENTITY;
                    /*std::cout << "[Editor] Deleted entity" << std::endl;*/
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Use full available size for the scrollable list
                ImGui::BeginChild("HierarchyList", ImVec2(-FLT_MIN, -FLT_MIN), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

                if (ImGui::IsWindowHovered()) {
                    ImGui::SetWindowFocus();
                }

                bool anySelectableClicked = false;

                // Sync display order with actual entities
                auto allEntities = g_coordinator->GetAllEntities();
                std::set<Entity> entitySet(allEntities.begin(), allEntities.end());

                entityDisplayOrder.erase(
                    std::remove_if(entityDisplayOrder.begin(), entityDisplayOrder.end(),
                        [&entitySet](Entity e) { return entitySet.find(e) == entitySet.end(); }),
                    entityDisplayOrder.end()
                );

                // Add new entities not yet tracked
                for (auto entity : allEntities) {
                    if (std::find(entityDisplayOrder.begin(), entityDisplayOrder.end(), entity) == entityDisplayOrder.end()) {
                        entityDisplayOrder.push_back(entity);
                    }
                }

                // Display in tracked order
                for (auto entity : entityDisplayOrder)
                {
                    std::string display;
                    if (g_coordinator->HasComponent<Name>(entity)) {
                        display = g_coordinator->GetComponent<Name>(entity).name;
                    }
                    else if (g_coordinator->HasComponent<GUIButton>(entity)) {
                        display = g_coordinator->GetComponent<GUIButton>(entity).buttonID;
                    }
                    else if (g_coordinator->HasComponent<Framework::Renderable>(entity)) {
                        auto& rend = g_coordinator->GetComponent<Framework::Renderable>(entity);
                        if (!rend.spriteName.empty()) {
                            // Extract just the filename from the path
                            size_t lastSlash = rend.spriteName.find_last_of("/\\");
                            if (lastSlash != std::string::npos) {
                                display = rend.spriteName.substr(lastSlash + 1);
                            }
                            else {
                                display = rend.spriteName;
                            }

                            // this js removes the .json when we load the prefab into the hierarchy list
                            size_t jsonPos = display.find(".json");
                            if (jsonPos != std::string::npos) {
                                display = display.substr(0, jsonPos);
                            }
                        }
                        else {
                            display = "New Entity";
                        }

                    }

                    // Fallback: Show entity ID
                    else
                    {
                        display = "Entity " + std::to_string((int)entity);
                    }
                    bool isSelected = (selectedEntity == entity);

                    std::string uniqueLabel = display + "##" + std::to_string((int)entity);
                    if (ImGui::Selectable(uniqueLabel.c_str(), isSelected))
                    {
                        selectedEntity = entity;
                        anySelectableClicked = true;  // Track that we clicked an entity
                    }
                }

                // Deselect if clicked on empty space in hierarchy
                if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !anySelectableClicked) {
                    selectedEntity = INVALID_ENTITY;
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


                // -----------------------------
                // Add Component button + popup
                // -----------------------------
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Components"); ImGui::SameLine();
                if (ImGui::Button("Add Component")) ImGui::OpenPopup("AddCompPopup");

                if (ImGui::BeginPopup("AddCompPopup"))
                {
                    // Transform
                    if (!g->HasComponent<Framework::Transform>(selectedEntity))
                    {
                        if (ImGui::MenuItem("Transform"))
                        {
                            Framework::Transform t{};
                            t.Pos = { 0.0f, 0.0f };
                            t.Scale = { 1.0f, 1.0f };
                            t.angle_disp = 0.0f;
                            t.angle_speed = 0.0f;
                            t.isVisible = true;
                            t.autoHide = false;
                            g->AddComponent<Framework::Transform>(selectedEntity, t);
                        }
                    }
                    // Renderable
                    if (!g->HasComponent<Framework::Renderable>(selectedEntity))
                    {
                        if (ImGui::MenuItem("Renderable"))
                        {
                            Framework::Renderable r{};
                            r.mdl_ref = 0.0f;
                            r.shd_ref = 2.0f;
                            r.textureID = 0;
                            r.spriteName.clear();
                            g->AddComponent<Framework::Renderable>(selectedEntity, r);
                        }
                    }
                    // Animation
                    if (!g->HasComponent<Framework::Animation>(selectedEntity))
                    {
                        if (ImGui::MenuItem("Animation"))
                        {
                            Framework::Animation a{};
                            a.rows = 1; a.columns = 1;
                            a.totalFrames = 1;
                            a.currentFrame = 0;
                            a.elapsedTime = 0.0f;
                            g->AddComponent<Framework::Animation>(selectedEntity, a);
                        }
                    }
                    // Name
                    if (!g->HasComponent<Name>(selectedEntity))
                    {
                        if (ImGui::MenuItem("Name"))
                        {
                            Name nm{};
                            nm.name = std::string("Entity ") + std::to_string((int)selectedEntity);
                            g->AddComponent<Name>(selectedEntity, nm);
                        }
                    }
                    // GUIButton (if your project uses it)
                    if (!g->HasComponent<GUIButton>(selectedEntity))
                    {
                        if (ImGui::MenuItem("GUIButton"))
                        {
                            GUIButton b{};
                            b.buttonID = "button";
                            g->AddComponent<GUIButton>(selectedEntity, b);
                        }
                    }

                    ImGui::EndPopup();
                }

                ImGui::Separator();


                // --- Name --------------------------------------------------------------------
                if (g->HasComponent<Name>(selectedEntity))
                {
                    auto& nm = g->GetComponent<Name>(selectedEntity);

                    ImGui::TextUnformatted("Name");
                    // remove
                    ImGui::SameLine();
                    if (ImGui::Button("Remove Name"))
                        g->RemoveComponent<Name>(selectedEntity);
                    ImGui::Separator();

                    // editable name
                    static char nameBuf[256];
                    std::strncpy(nameBuf, nm.name.c_str(), sizeof(nameBuf));
                    nameBuf[sizeof(nameBuf) - 1] = '\0';
                    if (ImGui::InputText("Entity Name", nameBuf, IM_ARRAYSIZE(nameBuf)))
                        nm.name = nameBuf;



                    ImGui::Separator();
                }


                // Transform
                if (g->HasComponent<Framework::Transform>(selectedEntity)) {
                    auto& tr = g->GetComponent<Framework::Transform>(selectedEntity);
                    ImGui::TextUnformatted("Transform");
                    // Remove button
                    ImGui::SameLine();
                    if (ImGui::Button("Remove Transform")) {
                        g->RemoveComponent<Framework::Transform>(selectedEntity);
                    }

                    ImGui::Separator();

                    float pos[2] = { tr.Pos.x, tr.Pos.y };
                    if (ImGui::DragFloat2("Position", pos, 0.5f)) { tr.Pos.x = pos[0]; tr.Pos.y = pos[1]; }
                    float sc[2] = { tr.Scale.x, tr.Scale.y };
                    if (ImGui::DragFloat2("Scale", sc, 0.5f)) { tr.Scale.x = sc[0]; tr.Scale.y = sc[1]; }
                    ImGui::DragFloat("Rotation (deg)", &tr.angle_disp, 1.0f, -360.f, 360.f);
                    ImGui::DragFloat("Rotation Speed", &tr.angle_speed, 0.1f, -10.0f, 10.0f);
                    ImGui::Checkbox("Visible", &tr.isVisible);
                    ImGui::Separator();


                }

                // Renderable
                if (g->HasComponent<Framework::Renderable>(selectedEntity))
                {
                    auto& r = g->GetComponent<Framework::Renderable>(selectedEntity);

                    ImGui::TextUnformatted("Renderable");
                    // remove button
                    ImGui::SameLine();
                    if (ImGui::Button("Remove Renderable")) {
                        g->RemoveComponent<Framework::Renderable>(selectedEntity);
                    }
                    ImGui::Separator();

                    // --- Model Reference (read-only label + +/-) ---
                    {
                        // In your renderer, mdl_ref==0 is Background, otherwise Sprite. (Two choices)
                        // See DrawAABB_NDC and the Push() comment in BatchRender. 
                        int modelIdx = static_cast<int>(r.mdl_ref);
                        modelIdx = std::clamp(modelIdx, 0, 1);

                        const char* kModelNames[2] = { "Background Quad", "Sprite Quad" };

                        // Show name (read-only), with +/- controls
                        ImGui::Text("Model: %s", kModelNames[modelIdx]);
                        ImGui::SameLine();
                        if (ImGui::SmallButton(" - ")) { modelIdx = std::max(0, modelIdx - 1); }
                        ImGui::SameLine();
                        if (ImGui::SmallButton(" + ")) { modelIdx = std::min(1, modelIdx + 1); }

                        // Write back
                        r.mdl_ref = static_cast<float>(modelIdx);
                    }

                    // --- Shader Reference (read-only label + +/-) ---
                    {
                        // We can clamp by the number of loaded shader programs
                        const int maxShaders = static_cast<int>(GLApp::shdrpgms.size());
                        int shaderIdx = std::clamp(static_cast<int>(r.shd_ref), 0, std::max(0, maxShaders - 1));

                        // Friendly names for the first few shaders in your init order:
                        // 0: test, 1: background, 2: sprite, 3: font  (see init_shdrpgms_cont order)
                        auto ShaderName = [&](int i) -> const char* {
                            switch (i) {
                            case 0: return "Test Shader";
                            case 1: return "Background Shader";
                            case 2: return "Sprite Shader";
                            case 3: return "Font Shader";
                            default: return "Shader (unnamed)";
                            }
                            };

                        ImGui::Text("Shader: %s", ShaderName(shaderIdx));
                        ImGui::SameLine();
                        if (ImGui::SmallButton(" - ")) { shaderIdx = std::max(0, shaderIdx - 1); }
                        ImGui::SameLine();
                        if (ImGui::SmallButton(" + ")) { shaderIdx = std::min(std::max(0, maxShaders - 1), shaderIdx + 1); }

                        r.shd_ref = static_cast<float>(shaderIdx);
                    }


                    // --- Texture info ---
                    ImGui::Text("Texture ID: %u", r.textureID);

                    // --- Sprite name: filename editing (no extension typing needed) ---
                    {
                        const std::string& full = r.spriteName;
                        std::string dir, fileNoExt, ext;

                        if (!full.empty()) {
                            size_t slash = full.find_last_of("/\\");
                            std::string filename = (slash != std::string::npos) ? full.substr(slash + 1) : full;
                            size_t dot = filename.find_last_of('.');
                            if (dot != std::string::npos) {
                                fileNoExt = filename.substr(0, dot);
                                ext = filename.substr(dot);
                            }
                            else fileNoExt = filename;
                            if (slash != std::string::npos)
                                dir = full.substr(0, slash + 1);
                        }

                        if (!dir.empty())
                            ImGui::TextWrapped("Folder: %s", dir.c_str());

                        static char fileBuf[256];
                        std::strncpy(fileBuf, fileNoExt.c_str(), sizeof(fileBuf));
                        fileBuf[sizeof(fileBuf) - 1] = '\0';

                        if (ImGui::InputText("Sprite Name", fileBuf, IM_ARRAYSIZE(fileBuf)))
                        {
                            std::string newName = fileBuf;

                            // Strip extension if user typed one
                            size_t dot = newName.find_last_of('.');
                            if (dot != std::string::npos)
                                newName = newName.substr(0, dot);

                            // Default extension if original was empty
                            if (ext.empty())
                                ext = ".png";

                            // Store filename only (no dir) so Reload can resolve anywhere in Textures/
                            bool typedHasSlash = newName.find_first_of("/\\") != std::string::npos;
                            if (typedHasSlash) r.spriteName = newName + ext;
                            else               r.spriteName = newName + ext;

                            r.needsTextureReload = true; // mark dirty; Reload button does the load+resolve
                        }
                    }

                    // --- Reload texture button ---
                    if (ImGui::Button("Reload Texture"))
                    {
                        namespace fs = std::filesystem;
                        const std::string kTexturesRoot = "../../MyEngine/Assets/Textures";

                        // Adjust these to your actual shader indices
                        constexpr int SPRITE_SHADER_INDEX = 1; // plain sprite shader
                        constexpr int SPRITESHEET_SHADER_INDEX = 2; // spritesheet shader

                        // 1) Resolve path if user typed only a filename or an invalid path
                        {
                            std::string candidate = r.spriteName;
                            std::error_code ec;
                            if (!fs::path(candidate).has_parent_path() || !fs::exists(candidate, ec)) {
                                std::string base = fs::path(candidate).filename().string();
                                if (base.empty()) base = candidate;

                                std::string resolved = Framework::AssetsManager::FindTextureByFilename(base, kTexturesRoot);
                                if (!resolved.empty())
                                    r.spriteName = resolved; // keep resolved full path for next time
                            }
                        }

                        // 2) Load (cached by AssetsManager)
                        r.textureID = Framework::AssetsManager::GetOrLoad(r.spriteName);
                        r.loadedSpriteName = r.spriteName;
                        r.needsTextureReload = false;

                        // 3) OPTIONAL: choose shader based on filename hint, but DO NOT touch Animation
                        int  hintRows = 1, hintCols = 1;
                        bool isSheet = false;

                        // Try strict grid parser on the *base filename*
                        {
                            int rr = 1, cc = 1;
                            std::string base = fs::path(r.spriteName).filename().string();
                            if (ParseGridFromFilename(base, rr, cc) && rr * cc > 1) {
                                hintRows = rr; hintCols = cc;
                                isSheet = true;
                            }
                        }

                        // Tiny heuristic backup ("sheet"/"atlas")
                        if (!isSheet) {
                            std::string base = fs::path(r.spriteName).filename().string();
                            for (auto& c : base) c = (char)tolower(c);
                            if (base.find("sheet") != std::string::npos || base.find("atlas") != std::string::npos)
                                isSheet = true;
                        }

                        // Switch only the shader; leave Animation component untouched
                        r.shd_ref = static_cast<float>(isSheet ? SPRITESHEET_SHADER_INDEX
                            : SPRITE_SHADER_INDEX);


                    }

                    ImGui::Separator();

                        

                    // --- Animation ---------------------------------------------------------------
                    if (g->HasComponent<Framework::Animation>(selectedEntity))
                    {
                        auto& a = g->GetComponent<Framework::Animation>(selectedEntity);

                        ImGui::TextUnformatted("Animation");
                        ImGui::SameLine();
                        if (ImGui::Button("Remove Animation")) {
                            g->RemoveComponent<Framework::Animation>(selectedEntity);
                        }

                        ImGui::Separator();

                        // Small helpers so fields look like: [value] [-] [+]  Label
                        auto StepperInt = [&](const char* label, int* v, int minv, int maxv, int step = 1)
                            {
                                ImGui::PushID(label);
                                ImGui::SetNextItemWidth(120.0f);
                                bool changed = ImGui::InputInt("", v);
                                ImGui::SameLine();
                                if (ImGui::SmallButton(" - ")) { *v = std::max(minv, *v - step); changed = true; }
                                ImGui::SameLine();
                                if (ImGui::SmallButton(" + ")) { *v = std::min(maxv, *v + step); changed = true; }
                                ImGui::SameLine();
                                ImGui::TextUnformatted(label);
                                ImGui::PopID();
                                return changed;
                            };
                        auto StepperFloat = [&](const char* label, float* v, float minv, float maxv, float step = 0.1f)
                            {
                                ImGui::PushID(label);
                                ImGui::SetNextItemWidth(120.0f);
                                bool changed = ImGui::InputFloat("", v, 0.0f, 0.0f, "%.1f");
                                ImGui::SameLine();
                                if (ImGui::SmallButton(" - ")) { *v = std::max(minv, *v - step); changed = true; }
                                ImGui::SameLine();
                                if (ImGui::SmallButton(" + ")) { *v = std::min(maxv, *v + step); changed = true; }
                                ImGui::SameLine();
                                ImGui::TextUnformatted(label);
                                ImGui::PopID();
                                return changed;
                            };

                        // Rows
                        {
                            int rows = a.rows;
                            if (StepperInt("Rows", &rows, 1, 999))
                            {
                                a.rows = std::max(1, rows);
                                a.totalFrames = std::max(1, a.rows * a.columns);
                                a.currentFrame = std::min(a.currentFrame, a.totalFrames - 1);
                                a.elapsedTime = 0.0f;
                            }
                        }
                        // Columns
                        {
                            int cols = a.columns;
                            if (StepperInt("Columns", &cols, 1, 999))
                            {
                                a.columns = std::max(1, cols);
                                a.totalFrames = std::max(1, a.rows * a.columns);
                                a.currentFrame = std::min(a.currentFrame, a.totalFrames - 1);
                                a.elapsedTime = 0.0f;
                            }
                        }

                        ImGui::Text("Total Frames: %d", a.totalFrames);

                        // --- Current Frame ---
                        {
                            // Always recompute max based on current rows*cols
                            const int maxFrame = std::max(0, a.totalFrames - 1);

                            // Use a to edit, then write back if changed
                            int cur = std::clamp(a.currentFrame, 0, maxFrame);

                            if (ImGui::SliderInt("Current Frame", &cur, 0, maxFrame)) {
                                a.currentFrame = cur;          // apply
                                a.elapsedTime = 0.0f;          // resync timer
                            }
                        }


                        // Animation speed (fps)
                        {
                            float sp = a.animationSpeed;
                            if (StepperFloat("Anim Speed (fps)", &sp, 0.0f, 120.0f, 0.1f))
                            {
                                a.animationSpeed = std::max(0.0f, sp);
                            }
                        }




                        ImGui::Separator();

                    }


    


                    // --- GUIButton ---------------------------------------------------------------
                    if (g->HasComponent<GUIButton>(selectedEntity))
                    {
                        auto& b = g->GetComponent<GUIButton>(selectedEntity);

                        ImGui::TextUnformatted("GUIButton");
                        ImGui::Separator();

                        // editable button id (safe basic field you already have)
                        static char idBuf[256];
                        std::strncpy(idBuf, b.buttonID.c_str(), sizeof(idBuf));
                        idBuf[sizeof(idBuf) - 1] = '\0';
                        if (ImGui::InputText("Button ID", idBuf, IM_ARRAYSIZE(idBuf)))
                            b.buttonID = idBuf;

                        // remove
                        if (ImGui::Button("Remove GUIButton"))
                            g->RemoveComponent<GUIButton>(selectedEntity);

                        ImGui::Separator();
                    }


                }
                // ============================================
                // PREFAB SAVE SECTION (at bottom of inspector)
                // ============================================
                if (selectedEntity != INVALID_ENTITY)
                {
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Prefab");
                    ImGui::Separator();

                    static char prefabNameBuffer[128] = "";
                    ImGui::SetNextItemWidth(-1);  // Full width
                    ImGui::InputTextWithHint("##prefabname", "Enter prefab name...", prefabNameBuffer, IM_ARRAYSIZE(prefabNameBuffer));

                    bool canSave = (prefabNameBuffer[0] != '\0');  // Check if not empty
                    if (!canSave) {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::Button("Save as Prefab")) {
                        if (strlen(prefabNameBuffer) > 0) {
                            auto* coord = Coordinator::GetInstance();

                            // Create temporary entity with ONLY safe components
                            Entity tempEntity = coord->CreateEntity();

                            // Copy ONLY Transform, Renderable, Animation
                            if (coord->HasComponent<Framework::Transform>(selectedEntity)) {
                                coord->AddComponent(tempEntity, coord->GetComponent<Framework::Transform>(selectedEntity));
                            }

                            if (coord->HasComponent<Framework::Renderable>(selectedEntity)) {
                                coord->AddComponent(tempEntity, coord->GetComponent<Framework::Renderable>(selectedEntity));
                            }

                            if (coord->HasComponent<Framework::Animation>(selectedEntity)) {
                                coord->AddComponent(tempEntity, coord->GetComponent<Framework::Animation>(selectedEntity));
                            }

                            // Save the temp entity (only has safe components)
                            PrefabManager prefabMgr;
                            bool success = prefabMgr.SaveEntityAsPrefab(tempEntity, prefabNameBuffer);

                            // Clean up temp entity
                            coord->DestroyEntity(tempEntity);

                            if (success) {
                                std::cout << "[Editor] Saved prefab: " << prefabNameBuffer << std::endl;
                                memset(prefabNameBuffer, 0, sizeof(prefabNameBuffer));
                            }
                            else {
                                std::cerr << "[Editor] Failed to save prefab!" << std::endl;
                            }
                        }
                    }

                    if (!canSave) {
                        ImGui::EndDisabled();
                    }

                    if (ImGui::IsItemHovered() && !canSave) {
                        ImGui::SetTooltip("Enter a prefab name first");
                    }
                }
            }
        }
        ImGui::End();

        // Project (Assets & Prefabs)
        if (bottomDockID) ImGui::SetNextWindowDockID(bottomDockID, ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Project")) {
            // folder browser
            ProjectPanel(selectedEntity);
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
