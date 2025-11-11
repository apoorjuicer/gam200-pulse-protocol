/******************************************************************************/
/**
 * @file        Editor.cpp
 * @project     Pulse Protocol
 * @author      Leu Jun Yong (primary) - 78%
 * @author      Goh Pin Kai (secondary) - 7%
 *              Chloe Lau Rey En (secondary) - 15%
 * @brief       Manages the full ImGui-based editor interface, including initialization, frame updates, rendering, and runtime toggling of editor features.
 * 
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
#include "CoreEngine/Asset/AssetsManager.h"

// ==========================================================================

namespace MyEditor
{
    // --------------------------------------------------------------------
    // CONSTANTS & BASIC STATE
    // --------------------------------------------------------------------

    const Entity INVALID_ENTITY = static_cast<Entity>(-1);
    Entity selectedEntity = INVALID_ENTITY;

    // this vector maintains the order entities appear in the hierarchy
    static std::vector<Entity> entityDisplayOrder;

    static GLFWwindow* s_Window = nullptr;
    static bool s_enabled = true;
    static bool s_restoreNextFrame = false;

    static bool s_showReplaceModal = false;
    static std::string s_replaceDstPath;   // file being replaced
    static std::string s_replaceSrcPath;

    static GizmoState g_gizmo;
    static CollisionManager g_col;
    static inline glm::vec2 ToGlm(const ImVec2& v) { return glm::vec2(v.x, v.y); }
    static inline ImVec2    ToIm(const glm::vec2& v) { return ImVec2(v.x, v.y); }
    static inline ImVec2    ToIm(const Vector2& v) { return ImVec2(v.x, v.y); } // your Vector2 (x,y)
    static inline Vector2   ToV2(const glm::vec2& v) { return Vector2{ v.x, v.y }; }
    static constexpr float kWorldW = 1920.0f;
    static constexpr float kWorldH = 1080.0f;

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
        {
        }
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

    [[maybe_unused]] static void DrawReplaceModal()
    {
        if (!s_showReplaceModal) return;

        ImGui::OpenPopup("Replace Texture");
        if (ImGui::BeginPopupModal("Replace Texture", &s_showReplaceModal, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextWrapped("Target:\n%s", s_replaceDstPath.c_str());
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextUnformatted("Replacement path (.png/.jpg/.bmp/.tga/.dds):");
            ImGui::SetNextItemWidth(420.0f);
            static char buf[512];
            std::strncpy(buf, s_replaceSrcPath.c_str(), sizeof(buf));
            buf[sizeof(buf) - 1] = '\0';
            if (ImGui::InputText("##replPath", buf, IM_ARRAYSIZE(buf)))
                s_replaceSrcPath = buf;

            // Allow dropping an internal texture payload into the text box to auto-fill
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("ASSET_TEXTURE"))
                {
                    const char* src = static_cast<const char*>(p->Data);
                    if (src && *src) s_replaceSrcPath = src;
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::Spacing();
            if (ImGui::Button("Replace", ImVec2(120, 0)))
            {
                if (!s_replaceDstPath.empty() && !s_replaceSrcPath.empty())
                {
                    if (ReplaceTextureFileOnDisk_AndRefreshAll(s_replaceDstPath, s_replaceSrcPath))
                    {
                        for (auto& it : s_dirEntries)
                            if (it.fullPath == s_replaceDstPath) { it.triedLoad = false; it.texID = 0; break; }

                        RefreshCurrentDir();
                        s_showReplaceModal = false;
                    }
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) s_showReplaceModal = false;

            ImGui::EndPopup();
        }
    }

    // ============================================================================
// Replace file on disk and hot-swap everywhere using AssetsManager API.
//  - Copies src -> dst
//  - mAssets.EvictTexture(dst) + mAssets.GetOrLoadTexture(dst) ⇒ NEW GL id
//  - Updates thumbnails and any Renderable using the file
// ============================================================================
    static bool ReplaceTextureFileOnDisk_AndRefreshAll(const std::string& dstPath,
        const std::string& srcPath)
    {
        namespace fs = std::filesystem;
        std::error_code ec;

        if (dstPath.empty() || srcPath.empty()) return false;
        if (!fs::exists(srcPath, ec)) return false;

        auto IsImage = [](const fs::path& p) {
            std::string ext = p.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            return ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                ext == ".bmp" || ext == ".tga" || ext == ".dds";
            };
        if (!IsImage(srcPath)) return false;

        // 1) Overwrite target file
        fs::copy_file(srcPath, dstPath, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            std::cerr << "[Project] Replace failed: " << srcPath << " -> "
                << dstPath << " (" << ec.message() << ")\n";
            return false;
        }

        // 2) Force a fresh GL texture id and cache entry
        mAssets.EvictTexture(dstPath);
        GLuint newTex = mAssets.GetOrLoadTexture(dstPath);
        if (!newTex) {
            std::cerr << "[Project] Reload failed for: " << dstPath << "\n";
            return false;
        }

        // 3) Update Project thumbnails
        for (auto& it : s_dirEntries) {
            if (!it.isDir && it.fullPath == dstPath) {
                it.texID = newTex;
                it.triedLoad = true;
            }
        }

        // 4) Update all live scene users (match by exact path OR same filename)
        if (auto* coord = Coordinator::GetInstance()) {
            for (Entity e : coord->GetAllEntities()) {
                if (!coord->HasComponent<Framework::Renderable>(e)) continue;
                auto& r = coord->GetComponent<Framework::Renderable>(e);

                std::string rBase = std::filesystem::path(r.spriteName).filename().string();
                std::string dBase = std::filesystem::path(dstPath).filename().string();

                if (!r.spriteName.empty() && (r.spriteName == dstPath || rBase == dBase)) {
                    r.spriteName = dstPath;     // normalize to definitive path
                    r.textureID = newTex;      // swap immediately
                }
            }
        }

        // 5) Refresh directory view (labels/order)
        RefreshCurrentDir();

        std::cout << "[Project] Replaced & hot-swapped: " << dstPath
            << "  <=  " << srcPath << "\n";
        return true;
    }

    

    static void RefreshCurrentDir()
    {
        s_dirEntries.clear();

        namespace fs = std::filesystem;
        std::error_code ec;

        // Normalize the current dir to an absolute, canonical path
        fs::path cur = fs::weakly_canonical(fs::absolute(s_currentDir), ec);
        if (ec || cur.empty() || !fs::exists(cur, ec)) return;

        // Add ".." (only if not at the asset root)
        fs::path root = fs::weakly_canonical(fs::absolute(s_assetRoot), ec);
        if (!ec && cur != root)
        {
            AssetItem up;
            up.name = "..";
            up.fullPath = fs::weakly_canonical(cur.parent_path(), ec).string();
            up.isDir = true;
            s_dirEntries.push_back(up);
        }

        // List the directory (non-recursive)
        for (auto const& entry : fs::directory_iterator(cur, ec))
        {
            AssetItem it;
            it.fullPath = fs::weakly_canonical(entry.path(), ec).string(); // absolute
            it.name = entry.path().filename().string();
            it.isDir = entry.is_directory();
            s_dirEntries.push_back(std::move(it));
        }

        // Sort: folders first, then alpha
        std::sort(s_dirEntries.begin(), s_dirEntries.end(),
            [](AssetItem const& a, AssetItem const& b)
            {
                if (a.isDir != b.isDir) return a.isDir > b.isDir;
                return a.name < b.name;
            });
    }


    // Load thumbnail texture lazily using your AssetsManager
    static void EnsureThumbnailLoaded(AssetItem& item)
    {
        if (item.triedLoad) return;     // already tried
        item.triedLoad = true;

        if (item.isDir) return;         // no thumb for folder

        if (!IsImageExt(item.fullPath)) return ;

        // this assumes your AssetsManager returns a GLuint / unsigned int
        item.texID = mAssets.GetOrLoadTexture(item.fullPath);
    }

    // ============================================================================
    // Replace a texture file on disk, then reload it everywhere in editor/runtime.
    // Safe version that doesn't need access to AssetsManager internals.
    // ============================================================================
    [[maybe_unused]] static bool ReplaceTextureFileOnDisk(const std::string& dstPath, const std::string& srcPath)
    {
        namespace fs = std::filesystem;
        std::error_code ec;

        if (dstPath.empty() || srcPath.empty()) return false;
        if (!fs::exists(srcPath, ec)) return false;

        // --- Validate image extension ---
        auto extLower = [](std::string s) {
            for (auto& c : s) c = (char)tolower(c);
            return s;
            };
        std::string srcExt = extLower(fs::path(srcPath).extension().string());
        if (!(srcExt == ".png" || srcExt == ".jpg" || srcExt == ".jpeg" ||
            srcExt == ".bmp" || srcExt == ".tga" || srcExt == ".dds"))
            return false;

        // --- 1) Copy new file over existing one ---
        fs::copy_file(srcPath, dstPath, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            std::cerr << "[Project] Replace failed: " << srcPath << " -> " << dstPath
                << " (" << ec.message() << ")\n";
            return false;
        }

        // --- 2) Force reload of new texture on next frame ---
        // We don't need to access the internal cache; just reload the new file.
        GLuint newTex = mAssets.GetOrLoadTexture(dstPath);

        // Bind it once to ensure GL has the new texture data
        glBindTexture(GL_TEXTURE_2D, newTex);

        std::cout << "[Project] Replaced texture file: " << dstPath
            << " with " << srcPath << "\n";
        return true;
    }


    static inline void ComputeGameDrawRect(const GameViewport& vp, float& drawX, float& drawY,
        float& drawW, float& drawH, float& scale)
    {
        const float sx = vp.w / kWorldW;
        const float sy = vp.h / kWorldH;
        scale = (sx < sy) ? sx : sy;           // fit inside
        drawW = kWorldW * scale;
        drawH = kWorldH * scale;
        drawX = vp.x + (vp.w - drawW) * 0.5f;  // pillar/letter box offsets
        drawY = vp.y + (vp.h - drawH) * 0.5f;
    }

    glm::vec2 WorldToScreen(const Vector2& wpos, const GameViewport& vp)
    {
        // world -> NDC
        const float sx = 2.0f / kWorldW, sy = 2.0f / kWorldH;
        const glm::mat3 N(sx, 0, 0, 0, sy, 0, 0, 0, 1);
        const glm::mat3 Cam = GLApp::BuildPanZoomNDC(GLApp::gCamera);
        const glm::vec3 ndc = Cam * (N * glm::vec3(wpos.x, wpos.y, 1.0f)); // -1..1

        // NDC -> framebuffer px inside the *drawn* game rect (letterbox-aware)
        float drawX, drawY, drawW, drawH, scale;
        ComputeGameDrawRect(vp, drawX, drawY, drawW, drawH, scale);

        const float x = drawX + (ndc.x * 0.5f + 0.5f) * drawW;
        const float y = drawY + (1.0f - (ndc.y * 0.5f + 0.5f)) * drawH;
        return { x, y };
    }


    Vector2 ScreenToWorld(glm::vec2 screen, const GameViewport& vp)
    {
        // framebuffer px -> NDC using the *drawn* game rect
        float drawX, drawY, drawW, drawH, scale;
        ComputeGameDrawRect(vp, drawX, drawY, drawW, drawH, scale);

        const float ndcX = ((screen.x - drawX) / drawW) * 2.0f - 1.0f;
        const float ndcY = 1.0f - ((screen.y - drawY) / drawH) * 2.0f;
        const glm::vec3 ndc(ndcX, ndcY, 1.0f);

        // invert camera + normalization (as you already do)
        const float sx = 2.0f / kWorldW, sy = 2.0f / kWorldH;
        const glm::mat3 N(sx, 0, 0, 0, sy, 0, 0, 0, 1);
        const glm::mat3 Cam = GLApp::BuildPanZoomNDC(GLApp::gCamera);
        const glm::mat3 invAll = glm::inverse(Cam * N);

        const glm::vec3 w = invAll * ndc;
        return Vector2{ w.x, w.y };
    }



    // Lightweight: click anywhere inside an entity's AABB to select; drag to move.
    // Uses BuildAABBFromTransform + CheckMouseCollision.
    void MyEditor::Gizmo_DrawAndHandle(const GameViewport& vp)
    {
        auto* coord = Coordinator::GetInstance();
        if (!coord) return;

        ImGuiIO& io = ImGui::GetIO();

        // ImGui UI px  <->  Framebuffer px (handles DPI / HiDPI)
        const float sx_ui = io.DisplayFramebufferScale.x > 0.f ? io.DisplayFramebufferScale.x : 1.f;
        const float sy_ui = io.DisplayFramebufferScale.y > 0.f ? io.DisplayFramebufferScale.y : 1.f;

        // --- Letterbox-aware draw rectangle (in framebuffer px) ---
        float drawX, drawY, drawW, drawH, scale;
        ComputeGameDrawRect(vp, drawX, drawY, drawW, drawH, scale);

        // Mouse in framebuffer pixels
        const glm::vec2 mouseFB(io.MousePos.x * sx_ui, io.MousePos.y * sy_ui);

        // FB (draw rect) -> NDC
        const float ndcX = ((mouseFB.x - drawX) / drawW) * 2.0f - 1.0f;
        const float ndcY = 1.0f - ((mouseFB.y - drawY) / drawH) * 2.0f;
        const glm::vec3 mouseNDC(ndcX, ndcY, 1.0f);

        // Camera pan/zoom used by the renderer
        const glm::mat3 PanZoom = GLApp::BuildPanZoomNDC(GLApp::gCamera);

        // Normalization matrix used by renderer (world −1..1)
        const float nsx = 2.0f / kWorldW, nsy = 2.0f / kWorldH;
        const glm::mat3 N(nsx, 0, 0, 0, nsy, 0, 0, 0, 1);
        const glm::mat3 invAll = glm::inverse(PanZoom * N);

        auto ScreenToWorld_DrawRect = [&](glm::vec2 fb)->Vector2 {
            const float x = ((fb.x - drawX) / drawW) * 2.0f - 1.0f;
            const float y = 1.0f - ((fb.y - drawY) / drawH) * 2.0f;
            glm::vec3 w = invAll * glm::vec3(x, y, 1.0f);
            return Vector2{ w.x, w.y };
            };

        // Hover/pick using the same pipeline as rendering
        Entity hovered = INVALID_ENTITY;

        // Clip gizmo to the drawn game rect (convert FB px -> ImGui UI px)
        ImDrawList* dl = ImGui::GetForegroundDrawList();
        dl->PushClipRect(ImVec2(drawX / sx_ui, drawY / sy_ui),
            ImVec2((drawX + drawW) / sx_ui, (drawY + drawH) / sy_ui),
            true);

        const bool inGameRect =
            (mouseFB.x >= drawX && mouseFB.x <= drawX + drawW &&
                mouseFB.y >= drawY && mouseFB.y <= drawY + drawH);

        for (Entity e : coord->GetAllEntities())
        {
            if (coord->HasComponent<GUIButton>(e)) continue;
            if (!coord->HasComponent<Framework::Transform>(e) ||
                !coord->HasComponent<Framework::Renderable>(e)) continue;

            auto& tr = coord->GetComponent<Framework::Transform>(e);
            auto& re = coord->GetComponent<Framework::Renderable>(e);
            if (!tr.isVisible) continue;

            // Model->NDC (same as draw path)
            glm::mat3 M = glm::make_mat3(tr.mdl_to_ndc_xform.Begin());
            glm::mat3 modelToNDC = PanZoom * M;
            glm::mat3 invModelToNDC = glm::inverse(modelToNDC);

            // Background uses [-1,1], sprites use [-0.5,0.5]
            glm::vec2 qmin, qmax;
            if (re.mdl_ref == 0) { qmin = { -1.f, -1.f }; qmax = { 1.f,  1.f }; }
            else { qmin = { -0.5f,-0.5f }; qmax = { 0.5f, 0.5f }; }

            // Hit test
            if (inGameRect) {
                glm::vec3 m = invModelToNDC * mouseNDC;
                if (m.x >= qmin.x && m.x <= qmax.x && m.y >= qmin.y && m.y <= qmax.y)
                    hovered = e;
            }

            // Draw bounds for the selected entity
            if (e == selectedEntity)
            {
                auto toUI = [&](glm::vec2 modelP)->ImVec2 {
                    glm::vec3 n = modelToNDC * glm::vec3(modelP, 1.0f);       // model -> NDC
                    const float x = drawX + (n.x * 0.5f + 0.5f) * drawW;       // NDC -> FB (draw rect)
                    const float y = drawY + (1.0f - (n.y * 0.5f + 0.5f)) * drawH;
                    return ImVec2(x / sx_ui, y / sy_ui);                       // FB -> UI
                    };

                ImVec2 a = toUI({ qmin.x, qmin.y });
                ImVec2 b = toUI({ qmax.x, qmin.y });
                ImVec2 c = toUI({ qmax.x, qmax.y });
                ImVec2 d = toUI({ qmin.x, qmax.y });

                dl->AddLine(a, b, IM_COL32(255, 220, 80, 180), 2.0f);
                dl->AddLine(b, c, IM_COL32(255, 220, 80, 180), 2.0f);
                dl->AddLine(c, d, IM_COL32(255, 220, 80, 180), 2.0f);
                dl->AddLine(d, a, IM_COL32(255, 220, 80, 180), 2.0f);
            }
        }

        // Drag logic (world-space; now using draw-rect aware conversion)
        if (inGameRect && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            if (hovered != INVALID_ENTITY) {
                selectedEntity = hovered;
                auto& tr = coord->GetComponent<Framework::Transform>(selectedEntity);
                Vector2 mouseWorld = ScreenToWorld_DrawRect(mouseFB);
                g_gizmo.dragging = true;
                g_gizmo.active = selectedEntity;
                g_gizmo.dragStartWorld = mouseWorld;
                g_gizmo.entityStartPos = tr.Pos;
            }
            else {
                selectedEntity = INVALID_ENTITY;
            }
        }

        if (g_gizmo.dragging && g_gizmo.active != INVALID_ENTITY && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            if (coord->HasComponent<Framework::Transform>(g_gizmo.active))
            {
                auto& tr = coord->GetComponent<Framework::Transform>(g_gizmo.active);
                Vector2 mouseWorld = ScreenToWorld_DrawRect(mouseFB);
                Vector2 d{ mouseWorld.x - g_gizmo.dragStartWorld.x,
                           mouseWorld.y - g_gizmo.dragStartWorld.y };
                tr.Pos.x = g_gizmo.entityStartPos.x + d.x;
                tr.Pos.y = g_gizmo.entityStartPos.y + d.y;
            }
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            g_gizmo.dragging = false;
            g_gizmo.active = INVALID_ENTITY;
        }

        dl->PopClipRect();
    }



    // ===================== ProjectPanel =====================
    static void ProjectPanel(Entity)
    {
        // ── Top bar ────────────────────────────────────────────────
        {
            if (ImGui::Button("Refresh")) {
                // Rebuild thumbnails from current disk contents
                for (auto& it : s_dirEntries) {
                    if (!it.isDir) {
                        mAssets.EvictTexture(it.fullPath);
                        it.texID = mAssets.GetOrLoadTexture(it.fullPath);
                        it.triedLoad = true;
                    }
                }
                RefreshCurrentDir();
            }
            ImGui::SameLine();
            ImGui::TextUnformatted(s_currentDir.c_str());
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120.0f);
            ImGui::SliderFloat("Size", &s_thumbSize, 40.0f, 160.0f, "%.0f");
        }
        ImGui::Separator();

        // ── OS drag-and-drop into project panel ───────────────────
        if (!s_pendingOSDrops.empty())
        {
            namespace fs = std::filesystem;
            std::error_code ec;

            fs::path dstDir = fs::weakly_canonical(fs::absolute(s_currentDir), ec);
            if (!ec && !dstDir.empty())
            {
                if (!fs::exists(dstDir, ec)) fs::create_directories(dstDir, ec);

                auto IsImageExt = [](const fs::path& p) {
                    std::string ext = p.extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    return ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                        ext == ".bmp" || ext == ".tga" || ext == ".dds";
                    };

                int changed = 0;
                for (const std::string& srcStr : s_pendingOSDrops)
                {
                    fs::path src = fs::weakly_canonical(fs::path(srcStr), ec);
                    if (ec || src.empty() || !fs::exists(src, ec)) continue;
                    fs::path dst = dstDir / src.filename();

                    if (fs::exists(dst, ec))
                    {
                        // same name → replace
                        if (IsImageExt(dst)) {
                            if (ReplaceTextureFileOnDisk_AndRefreshAll(dst.string(), src.string()))
                                ++changed;
                        }
                        else {
                            fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
                            if (!ec) ++changed;
                        }
                    }
                    else
                    {
                        // new file → import
                        fs::copy_file(src, dst, fs::copy_options::none, ec);
                        if (!ec) {
                            ++changed;
                            if (IsImageExt(dst)) {
                                mAssets.EvictTexture(dst.string()); // be explicit
                                mAssets.GetOrLoadTexture(dst.lexically_normal().string());
                            }
                        }
                    }
                }
                if (changed > 0) RefreshCurrentDir();
            }
            s_pendingOSDrops.clear();
        }

        // ── Grid ──────────────────────────────────────────────────
        ImGui::BeginChild("AssetsGrid", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        float availX = ImGui::GetContentRegionAvail().x;
        float cellSize = s_thumbSize + s_thumbPadding;
        int   cols = (int)(availX / cellSize);
        if (cols < 1) cols = 1;

        int itemIndex = 0;
        for (auto& item : s_dirEntries)
        {
            if (itemIndex % cols != 0) ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::PushID(itemIndex);

            ImVec2 thumbSize(s_thumbSize, s_thumbSize);
            bool clicked = false;

            if (item.isDir)
            {
                if (ImGui::Button(item.name.c_str(), thumbSize)) {
                    s_currentDir = item.fullPath;
                    RefreshCurrentDir();
                    ImGui::PopID(); ImGui::EndGroup(); ImGui::EndChild(); return;
                }
            }
            else
            {
                EnsureThumbnailLoaded(item);  // your existing function

                if (item.texID != 0) {
                    std::string btnId = "##assetTex_" + item.fullPath;
                    clicked = ImGui::ImageButton(btnId.c_str(), (ImTextureID)(intptr_t)item.texID,
                        ImVec2(s_thumbSize, s_thumbSize), ImVec2(0, 1), ImVec2(1, 0));
                }
                else {
                    if (ImGui::Button(item.name.c_str(), thumbSize)) clicked = true;
                }

                // Allow drag source (image files only)
                auto IsImageExtStr = [](const std::string& s) {
                    std::string ext;
                    auto dot = s.find_last_of('.');
                    if (dot != std::string::npos) ext = s.substr(dot);
                    for (auto& c : ext) c = (char)tolower(c);
                    return ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                        ext == ".bmp" || ext == ".tga" || ext == ".dds";
                    };

                if (IsImageExtStr(item.fullPath))
                {
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        const char* path = item.fullPath.c_str();
                        ImGui::SetDragDropPayload("ASSET_TEXTURE", path, (int)std::strlen(path) + 1);
                        ImGui::TextUnformatted(("Texture: " + item.name).c_str());
                        ImGui::EndDragDropSource();
                    }

                    // Drag target: drop one image onto another to replace
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("ASSET_TEXTURE"))
                        {
                            const char* src = static_cast<const char*>(p->Data);
                            if (src && *src)
                            {
                                std::string srcPath = src;
                                std::string dstPath = item.fullPath;
                                if (srcPath != dstPath)
                                    ReplaceTextureFileOnDisk_AndRefreshAll(dstPath, srcPath);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                // Left-click assigns to selected entity
                if (clicked)
                {
                    // Check if this is a prefab (.json file)
                    if (item.fullPath.ends_with(".json"))
                    {
                        // Instantiate prefab as a new entity
                        PrefabManager prefabMgr;

                        // Get just the filename without extension for the prefab name
                        std::string prefabName = item.name;
                        if (prefabName.ends_with(".json")) {
                            prefabName = prefabName.substr(0, prefabName.length() - 5);
                        }

                        auto result = prefabMgr.InstantiatePrefab(prefabName);
                        if (result.has_value()) {
                            Entity newEntity = result.value();

                            // Add to hierarchy list so it shows up
                            entityDisplayOrder.push_back(newEntity);

                            // Select the newly created entity
                            selectedEntity = newEntity;

                            std::cout << "[Editor] Instantiated prefab: " << prefabName
                                << " (Entity " << newEntity << ")" << std::endl;
                        }
                        else {
                            std::cerr << "[Editor] Failed to instantiate prefab: " << prefabName << std::endl;
                        }
                    }
                    else if (selectedEntity != INVALID_ENTITY)
                    {
                        // For non-prefab files (textures), assign to selected entity
                        auto* coord = Coordinator::GetInstance();
                        if (coord && coord->HasComponent<Framework::Renderable>(selectedEntity))
                        {
                            auto& rend = coord->GetComponent<Framework::Renderable>(selectedEntity);
                            rend.spriteName = item.fullPath;
                            rend.textureID = mAssets.GetOrLoadTexture(rend.spriteName);
                        }
                    }
                }
            }

            // label
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + s_thumbSize);
            std::string displayName = item.name;
            if (displayName.ends_with(".json")) displayName.resize(displayName.size() - 5);
            ImGui::TextUnformatted(displayName.c_str());
            ImGui::PopTextWrapPos();

            ImGui::PopID();
            ImGui::EndGroup();
            ++itemIndex;
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


    void EnqueueExternalDrop(const std::vector<std::string>& paths)
    {
        // Append (don’t clear) so multiple drops are handled
        s_pendingOSDrops.insert(s_pendingOSDrops.end(), paths.begin(), paths.end());
    }

    void PushOSDropPath(const char* path)
    {
        if (path && *path)
            s_pendingOSDrops.emplace_back(path);
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

        // Make s_assetRoot and s_currentDir absolute/canonical so all I/O hits the real disk location.
        {
            namespace fs = std::filesystem;
            std::error_code ec;

            // s_assetRoot currently holds "../../MyEngine/Assets"
            fs::path abs = fs::weakly_canonical(fs::absolute(s_assetRoot), ec);
            if (!ec && !abs.empty())
                s_assetRoot = abs.string();

            s_currentDir = s_assetRoot;  // start in the real absolute root
        }

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

                // Calculate button sizes
                const char* playStopLabel = s_isPlaying ? "Stop" : "Play";
                ImVec2 playStopTextSize = ImGui::CalcTextSize(playStopLabel);

                // Make Pause button show "Resume" when paused
                static bool isPaused = false;
                const char* pauseLabel = isPaused ? "Resume" : "Pause";
                ImVec2 pauseTextSize = ImGui::CalcTextSize(pauseLabel);

                ImVec2 playStopButtonSize = ImVec2(playStopTextSize.x + framePadding.x * 2 + 20.0f, 0);
                ImVec2 pauseButtonSize = ImVec2(pauseTextSize.x + framePadding.x * 2 + 20.0f, 0);

                float windowWidth = ImGui::GetWindowWidth();
                float totalButtonWidth = playStopButtonSize.x + pauseButtonSize.x + itemSpacing;
                float offset = (windowWidth - totalButtonWidth) * 0.5f;

                // Stay on same line, position at centered offset
                if (offset > 0.0f) {
                    ImGui::SameLine(offset);
                }
                else {
                    ImGui::SameLine();
                }

                // --- Play ---
                if (ImGui::Button(playStopLabel, playStopButtonSize))
                {
                    if (!s_isPlaying) {
                        // START PLAYING
                        if (!s_hasEditorBaseline) {
                            CaptureEditorBaseline();
                        }
                        Engine->PlayGame();
                        s_isPlaying = true;
                        isPaused = false; // Reset pause when starting fresh
                        std::cout << "[Editor] Play Started\n";
                    }
                    else {
                        // STOP PLAYING & RESTORE
                        Engine->StopGame();
                        s_isPlaying = false;
                        isPaused = false;
                        RestoreEditorBaseline();
                        s_hasEditorBaseline = false;
                        std::cout << "[Editor] Stopped - Scene Restored\n";
                    }
                }

                ImGui::SameLine();

                // --- Pause ---
                
                if (!s_isPlaying) {
                    ImGui::BeginDisabled(); // Grey out when not playing
                }
                
                if (ImGui::Button(pauseLabel, pauseButtonSize))
                {
                    if (s_isPlaying) {
                        isPaused = !isPaused;
                        
                        if (isPaused) {
                            Engine->PauseGame();
                            std::cout << "[Editor] Game Paused\n";
                        } else {
                            Engine->ResumeGame();
                            std::cout << "[Editor] Game Resumed\n";
                        }
                    }
                }

                if (!s_isPlaying) {
                    ImGui::EndDisabled(); // End grey out
                }

                // Show tooltip when hovering over disabled Pause button
                if (!s_isPlaying && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                    ImGui::SetTooltip("Start playing to enable Pause");
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



                // Right click to add component
                if (selectedEntity != INVALID_ENTITY &&
                    ImGui::BeginPopupContextWindow("AddCompPopup",
                        ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
                {
                    auto* coord = Coordinator::GetInstance();

                    // Transform
                    if (!coord->HasComponent<Framework::Transform>(selectedEntity))
                    {
                        if (ImGui::MenuItem("Transform"))
                        {
                            Framework::Transform t{};
                            t.Pos = { 0.0f, 0.0f };
                            t.Scale = { 1.0f, 1.0f };
                            t.Rotation.x = 0.0f;
                            t.Rotation.y = 0.0f;
                            t.isVisible = true;
                            t.autoHide = false;
                            coord->AddComponent<Framework::Transform>(selectedEntity, t);
                        }
                    }

                    // Renderable
                    if (!coord->HasComponent<Framework::Renderable>(selectedEntity))
                    {
                        if (ImGui::MenuItem("Renderable"))
                        {
                            Framework::Renderable r{};
                            r.mdl_ref = 0.0f;
                            r.shd_ref = 2.0f;
                            r.textureID = 0;
                            r.spriteName.clear();
                            coord->AddComponent<Framework::Renderable>(selectedEntity, r);
                        }
                    }

                    // Animation
                    if (!coord->HasComponent<Framework::Animation>(selectedEntity))
                    {
                        if (ImGui::MenuItem("Animation"))
                        {
                            Framework::Animation a{};
                            a.rows = 1; a.columns = 1;
                            a.totalFrames = 1;
                            a.currentFrame = 0;
                            a.elapsedTime = 0.0f;
                            coord->AddComponent<Framework::Animation>(selectedEntity, a);
                        }
                    }

                    // Name
                    if (!coord->HasComponent<Name>(selectedEntity))
                    {
                        if (ImGui::MenuItem("Name"))
                        {
                            Name nm{};
                            nm.name = std::string("Entity ") + std::to_string((int)selectedEntity);
                            coord->AddComponent<Name>(selectedEntity, nm);
                        }
                    }

                    // GUIButton (optional, if used in your project)
                    if (!coord->HasComponent<GUIButton>(selectedEntity))
                    {
                        if (ImGui::MenuItem("GUIButton"))
                        {
                            GUIButton b{};
                            b.buttonID = "button";
                            coord->AddComponent<GUIButton>(selectedEntity, b);
                        }
                    }

                    ImGui::EndPopup();
                }


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
                    ImGui::DragFloat("Rotation (deg)", &tr.Rotation.x, 1.0f, -360.f, 360.f);
                    ImGui::DragFloat("Rotation Speed", &tr.Rotation.y, 0.1f, -10.0f, 10.0f);
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

                    // --- Reload texture button + Drag&Drop target ---
                    {
                        // Draw the button first
                        bool reloadClicked = ImGui::Button("Reload Texture");

                        // Allow dropping a texture file onto this button
                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_TEXTURE"))
                            {
                                // -------------- Apply dropped path --------------
                                const char* droppedPath = static_cast<const char*>(payload->Data);
                                r.spriteName = droppedPath ? std::string(droppedPath) : std::string();
                                reloadClicked = true; // reuse the same reload routine below
                            }
                            ImGui::EndDragDropTarget();
                        }

                        if (reloadClicked)
                        {
                            namespace fs = std::filesystem;
                            const std::string kTexturesRoot = "../../MyEngine/Assets/Textures";

                            // Adjust these to your actual shader indices
                            constexpr int SPRITE_SHADER_INDEX = 1; // plain sprite shader
                            constexpr int SPRITESHEET_SHADER_INDEX = 2; // spritesheet shader

                            // 1) Resolve path if user typed only a filename or an invalid path
                            {
                                namespace fs = std::filesystem;

                                std::string candidate = r.spriteName;
                                std::error_code ec;

                                // If user already provided a valid path on disk, keep it.
                                if ((fs::path(candidate).has_parent_path() || fs::path(candidate).is_absolute()) &&
                                    fs::exists(candidate, ec))
                                {
                                    // nothing to do; r.spriteName already points to an existing file
                                }
                                else
                                {
                                    // Reduce to just the filename (e.g., "hero.png")
                                    std::string base = fs::path(candidate).filename().string();
                                    if (base.empty()) base = candidate;

                                    // Where to search (add/remove roots as you need)
                                    const fs::path roots[] = {
                                        fs::path(kTexturesRoot),                      // "../../MyEngine/Assets/Textures"
                                        fs::path("../../MyEngine/Assets")             // optional broader scan
                                    };

                                    std::string resolved;

                                    for (const fs::path& root : roots)
                                    {
                                        fs::path rootAbs = fs::weakly_canonical(root, ec);
                                        if (ec || rootAbs.empty() || !fs::exists(rootAbs, ec)) continue;

                                        // Scan this root recursively and stop at the first filename match
                                        for (fs::recursive_directory_iterator it(rootAbs, ec), end; it != end && !ec; ++it)
                                        {
                                            if (!it->is_regular_file(ec)) continue;
                                            if (it->path().filename().string() == base)
                                            {
                                                resolved = it->path().lexically_normal().string();
                                                break;
                                            }
                                        }
                                        if (!resolved.empty()) break;
                                    }

                                    if (!resolved.empty())
                                    {
                                        // Keep the fully-resolved, normalized absolute path for next time
                                        r.spriteName = resolved;
                                    }
                                    // else: leave r.spriteName as-is; the subsequent load attempt will surface any error
                                }
                            }


                            // 2) Load (cached by AssetsManager)
                            r.textureID = mAssets.GetOrLoadTexture(r.spriteName);
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
                            PrefabManager prefabMgr;
                            bool success = prefabMgr.SaveEntityAsPrefab(selectedEntity, prefabNameBuffer);

                            if (success) {
                                std::cout << "[Editor] Saved prefab: " << prefabNameBuffer << " with ALL components" << std::endl;
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

        // ===== In-scene picking/dragging (gizmo) on the middle viewport =====
        MyEditor::GameViewport vp;
        vp.x = (int)left_w;
        vp.y = (int)middle_y;
        vp.w = (int)safe_mid_w;
        vp.h = (int)safe_middle_h;


        MyEditor::Gizmo_DrawAndHandle(vp);





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
        if (fbW <= 0 || fbH <= 0) {
            // window minimized / no drawable area
            return {0, 0, 0, 0};
        }


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