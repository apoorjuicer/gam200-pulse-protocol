/******************************************************************************/
/**
 * @file        ProjectPanel.cpp
 * @project     Pulse Protocol
 * @author      Leu Jun Yong (primary) - 80%
 *              Chloe Lau Rey En (secondary) - 20%
 * @brief       Implements the Project Panel (asset browser) for the editor.
 *
 *              Features:
 *              - Directory browsing with navigation and thumbnail previews
 *              - Adjustable thumbnail size via slider
 *              - Drag-and-drop texture assignment to selected entities
 *              - Drag-and-drop entity-to-prefab creation (from Hierarchy)
 *              - OS file import via external drag-and-drop
 *              - Texture hot-swapping: drop image onto another to replace
 *              - Prefab instantiation on click, deletion via context menu
 *              - Auto-detection of spritesheets based on filename keywords
 *              - Editing disabled during play mode
 *
 *              Prefab Drop Zones:
 *              - Dropping onto Prefab folder button (from parent directory)
 *              - Dropping into empty space inside Prefab folder
 *              Both zones validate entities via PrefabManager::CanSaveAsPrefab()
 *              and display a warning popup for engine-managed entities
 *              (names starting with '_', e.g., _FPS_Display).
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "pch/pch_temp.h"

#include "ProjectPanel.h"
#include "TopBarPanel.h"
#include "imgui.h"

#include "CoreEngine/Asset/AssetsManager.h"
#include "CoreEngine/ECS/Coordinator.h"
#include "Graphics/Renderable.h"
#include "Graphics/Transform.h"
#include "CoreEngine/Prefab/PrefabManager.h"

namespace PulseEditor
{
    // Externs from elsewhere
    extern std::vector<Entity> entityDisplayOrder;   // defined in Editor / Hierarchy panel

    // ── Asset browser state ─────────────────────────────────────
    static std::string s_assetRoot = "../../PulseEngine/Assets";
    static std::string s_currentDir = s_assetRoot;
    static std::vector<AssetItem> s_dirEntries;

    static float s_thumbSize = 72.0f;
    static float s_thumbPadding = 10.0f;

    static bool        s_showReplaceModal = false;
    static std::string s_replaceDstPath;
    static std::string s_replaceSrcPath;


    // ── Helper declarations ─────────────────────────────────────
    static bool IsImageExt(const std::string& path);
    static void EnsureThumbnailLoaded(AssetItem& item);
    static void RefreshCurrentDir();
    static bool ReplaceTextureFileOnDisk_AndRefreshAll(const std::string& dstPath,
        const std::string& srcPath);
    [[maybe_unused]] static bool ReplaceTextureFileOnDisk(const std::string& dstPath,
        const std::string& srcPath);

    static bool s_showEngineManagedWarning = false;
    static std::string s_blockedEntityName = "";

    // ── Audio import error popup state ───────────────────────────
    static bool s_showAudioImportError = false;
    static std::string s_invalidAudioFileName = "";
    static std::string s_targetFolderName = "";

    // ── Init ────────────────────────────────────────────────────
    void InitProjectPanel()
    {
        namespace fs = std::filesystem;
        std::error_code ec;

        fs::path abs = fs::weakly_canonical(fs::absolute(s_assetRoot), ec);
        if (!ec && !abs.empty())
            s_assetRoot = abs.string();

        s_currentDir = s_assetRoot;
        RefreshCurrentDir();
    }

    // ── OS drag-and-drop queue API ─────────────────────────────
    void EnqueueExternalDrop(const std::vector<std::string>& paths)
    {
        s_pendingOSDrops.insert(s_pendingOSDrops.end(), paths.begin(), paths.end());
    }

    void PushOSDropPath(const char* path)
    {
        if (path && *path)
            s_pendingOSDrops.emplace_back(path);
    }

    // ── Directory scan / thumbnails ────────────────────────────
    static void RefreshCurrentDir()
    {
        s_dirEntries.clear();

        namespace fs = std::filesystem;
        std::error_code ec;

        fs::path cur = fs::weakly_canonical(fs::absolute(s_currentDir), ec);
        if (ec || cur.empty() || !fs::exists(cur, ec))
            return;

        fs::path root = fs::weakly_canonical(fs::absolute(s_assetRoot), ec);

        // Add ".." (only if not at the asset root)
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
            it.fullPath = fs::weakly_canonical(entry.path(), ec).string();
            it.name = entry.path().filename().string();
            it.isDir = entry.is_directory();
            s_dirEntries.push_back(std::move(it));
        }

        // Sort: folders first, then alpha
        std::sort(s_dirEntries.begin(), s_dirEntries.end(),
            [](const AssetItem& a, const AssetItem& b)
            {
                if (a.isDir != b.isDir)
                    return a.isDir > b.isDir;
                return a.name < b.name;
            });
    }

    static bool IsImageExt(const std::string& path)
    {
        const auto dot = path.find_last_of('.');
        if (dot == std::string::npos)
            return false;

        std::string ext = path.substr(dot + 1);
        for (char& c : ext)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        return (ext == "png" || ext == "jpg" || ext == "jpeg" ||
            ext == "bmp" || ext == "tga" || ext == "dds");
    }

    static void EnsureThumbnailLoaded(AssetItem& item)
    {
        if (item.triedLoad) return;     // already tried
        item.triedLoad = true;

        if (item.isDir) return;         // no thumb for folder

        if (!IsImageExt(item.fullPath)) return;

        // this assumes your AssetsManager returns a GLuint / unsigned int
        item.texID = mAssets.GetOrLoadTexture(item.fullPath);
    }

    bool IsAudioExt(const std::string& path)
    {
        const auto dot = path.find_last_of('.');
        if (dot == std::string::npos)
            return false;

        std::string ext = path.substr(dot + 1);
        for (char& c : ext)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        return (ext == "mp3" || ext == "wav");
    }

    bool IsAudioFolder(const std::string& folderName)
    {
        std::string lower = folderName;
        for (char& c : lower)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        return (lower == "sounds" || lower == "sound" ||
            lower == "music" || lower == "audio");
    }

    // Replace file on disk and hot-swap everywhere using AssetsManager API.
    static bool ReplaceTextureFileOnDisk_AndRefreshAll(const std::string& dstPath,
        const std::string& srcPath)
    {
        namespace fs = std::filesystem;
        std::error_code ec;

        if (dstPath.empty() || srcPath.empty())
            return false;
        if (!fs::exists(srcPath, ec))
            return false;

        auto IsImage = [](const fs::path& p)
            {
                std::string ext = p.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                return ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                    ext == ".bmp" || ext == ".tga" || ext == ".dds";
            };

        if (!IsImage(srcPath))
            return false;

        // 1) Overwrite target file
        fs::copy_file(srcPath, dstPath, fs::copy_options::overwrite_existing, ec);
        if (ec)
        {
            std::cerr << "[Project] Replace failed: " << srcPath << " -> "
                << dstPath << " (" << ec.message() << ")\n";
            return false;
        }

        // 2) Force a fresh GL texture id and cache entry
        mAssets.EvictTexture(dstPath);
        GLuint newTex = mAssets.GetOrLoadTexture(dstPath);
        if (!newTex)
        {
            std::cerr << "[Project] Reload failed for: " << dstPath << "\n";
            return false;
        }

        // 3) Update Project thumbnails
        for (auto& it : s_dirEntries)
        {
            if (!it.isDir && it.fullPath == dstPath)
            {
                it.texID = newTex;
                it.triedLoad = true;
            }
        }

        // 4) Update all live scene users (match by exact path OR same filename)
        if (auto* coord = Coordinator::GetInstance())
        {
            for (Entity e : coord->GetAllEntities())
            {
                if (!coord->HasComponent<Framework::Renderable>(e))
                    continue;

                auto& r = coord->GetComponent<Framework::Renderable>(e);

                std::string rBase = std::filesystem::path(r.spriteName).filename().string();
                std::string dBase = std::filesystem::path(dstPath).filename().string();

                if (!r.spriteName.empty() &&
                    (r.spriteName == dstPath || rBase == dBase))
                {
                    r.spriteName = dstPath;
                    r.textureID = newTex;
                }
            }
        }

        // 5) Refresh directory view (labels/order)
        RefreshCurrentDir();

        std::cout << "[Project] Replaced & hot-swapped: " << dstPath
            << "  <=  " << srcPath << "\n";
        return true;
    }

    // ── Main panel UI ──────────────────────────────────────────
    void ProjectPanel()
    {
        // Disable all editing when playing
        bool isPlaying = IsPlaying();
        if (isPlaying)
        {
            ImGui::BeginDisabled();
        }

        // Top bar ------------------------------------------------
        if (ImGui::Button("Refresh"))
        {
            // Rebuild thumbnails from current disk contents
            for (auto& it : s_dirEntries)
            {
                if (!it.isDir)
                {
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

        ImGui::Separator();

        // OS drag-and-drop into project panel --------------------
        if (!s_pendingOSDrops.empty())
        {
            namespace fs = std::filesystem;
            std::error_code ec;

            fs::path dstDir = fs::weakly_canonical(fs::absolute(s_currentDir), ec);
            if (!ec && !dstDir.empty())
            {
                if (!fs::exists(dstDir, ec))
                    fs::create_directories(dstDir, ec);

                auto IsImageExtFs = [](const fs::path& p)
                    {
                        std::string ext = p.extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        return ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                            ext == ".bmp" || ext == ".tga" || ext == ".dds";
                    };

                // Check if current folder is an audio folder
                std::string currentFolderName = dstDir.filename().string();
                bool isAudioFolder = IsAudioFolder(currentFolderName);

                int changed = 0;
                for (const std::string& srcStr : s_pendingOSDrops)
                {
                    fs::path src = fs::weakly_canonical(fs::path(srcStr), ec);
                    if (ec || src.empty() || !fs::exists(src, ec))
                        continue;

                    // ═══════════════════════════════════════════════════════
                    // AUDIO FOLDER VALIDATION
                    // ═══════════════════════════════════════════════════════
                    if (isAudioFolder && !IsAudioExt(src.string()))
                    {
                        // Invalid file type for audio folder - show error popup
                        s_showAudioImportError = true;
                        s_invalidAudioFileName = src.filename().string();
                        s_targetFolderName = currentFolderName;
                        continue;  // Skip this file, don't import it
                    }

                    fs::path dst = dstDir / src.filename();

                    if (fs::exists(dst, ec))
                    {
                        // if same name, replace
                        if (IsImageExtFs(dst))
                        {
                            if (ReplaceTextureFileOnDisk_AndRefreshAll(dst.string(), src.string()))
                                ++changed;
                        }
                        else
                        {
                            fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
                            if (!ec) ++changed;
                        }
                    }
                    else
                    {
                        // new file → import
                        fs::copy_file(src, dst, fs::copy_options::none, ec);
                        if (!ec)
                        {
                            ++changed;
                            if (IsImageExtFs(dst))
                            {
                                mAssets.EvictTexture(dst.string());
                                mAssets.GetOrLoadTexture(dst.lexically_normal().string());
                            }
                        }
                    }
                }

                if (changed > 0)
                    RefreshCurrentDir();
            }

            s_pendingOSDrops.clear();
        }

        // Asset grid ---------------------------------------------
        ImGui::BeginChild("AssetsGrid", ImVec2(0, 0), false,
            ImGuiWindowFlags_HorizontalScrollbar);

        float availX = ImGui::GetContentRegionAvail().x;
        float cellSize = s_thumbSize + s_thumbPadding;
        int   cols = static_cast<int>(availX / cellSize);
        if (cols < 1)
            cols = 1;

        int itemIndex = 0;
        for (auto& item : s_dirEntries)
        {
            if (itemIndex % cols != 0)
                ImGui::SameLine();

            ImGui::BeginGroup();
            ImGui::PushID(itemIndex);

            ImVec2 thumbSize(s_thumbSize, s_thumbSize);
            bool   clicked = false;

            if (item.isDir)
            {
                if (ImGui::Button(item.name.c_str(), thumbSize))
                {
                    s_currentDir = item.fullPath;
                    RefreshCurrentDir();
                    ImGui::PopID();
                    ImGui::EndGroup();
                    ImGui::EndChild();
                    return;
                }
                // Add drop target ONLY for Prefab/Prefabs folder
                bool isPrefabFolder = (item.name == "Prefab" || item.name == "Prefabs" || item.name == "prefab" || item.name == "prefabs");

                if (isPrefabFolder && ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY_REORDER", ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
                    {
                        // Extract entity from the payload
                        struct HierarchyDragData {
                            int index;
                            Entity entity;
                        };

                        const HierarchyDragData* dragData = (const HierarchyDragData*)payload->Data;
                        Entity droppedEntity = dragData->entity;

                        //std::cout << "[ProjectPanel] Entity " << droppedEntity << " dropped into Prefabs folder: " << item.fullPath << std::endl;

                        // Get entity name for prefab
                        auto* coord = Coordinator::GetInstance();
                        if (coord && droppedEntity != INVALID_ENTITY)
                        {
                            std::string prefabName = "Prefab";

                            // Try to get the entity's name
                            if (coord->HasComponent<Name>(droppedEntity))
                            {
                                prefabName = coord->GetComponent<Name>(droppedEntity).name;
                            }
                            else
                            {
                                prefabName = "Entity_" + std::to_string(static_cast<int>(droppedEntity));
                            }

                            // Create prefab using PrefabManager
                            PrefabManager prefabMgr;

                            // =====================================================
                            // Check if entity can be saved as prefab
                            // =====================================================
                            if (!prefabMgr.CanSaveAsPrefab(droppedEntity))
                            {
                                // Entity is engine-managed - show warning popup
                                s_showEngineManagedWarning = true;
                                s_blockedEntityName = prefabName;
                            }
                            else
                            {
                                // Safe to save
                                if (prefabMgr.SaveEntityAsPrefab(droppedEntity, prefabName))
                                {
                                    std::cout << "[ProjectPanel] Successfully created prefab: "
                                        << prefabName << ".json" << std::endl;
                                    RefreshCurrentDir();  // Refresh to show the new prefab file
                                }
                                else
                                {
                                    std::cerr << "[ProjectPanel] Failed to create prefab: "
                                        << prefabName << ".json" << std::endl;
                                }
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
            }
            else
            {
                EnsureThumbnailLoaded(item);

                if (item.texID != 0)
                {
                    std::string btnId = "##assetTex_" + item.fullPath;
                    clicked = ImGui::ImageButton(btnId.c_str(),
                        (ImTextureID)(intptr_t)item.texID,
                        ImVec2(s_thumbSize, s_thumbSize),
                        ImVec2(0, 1), ImVec2(1, 0));
                }
                else
                {
                    if (ImGui::Button(item.name.c_str(), thumbSize))
                        clicked = true;

                    if (item.fullPath.ends_with(".json"))
                    {
                        if (ImGui::BeginPopupContextItem(""))
                        {
                            std::string displayName = item.name;
                            if (displayName.ends_with(".json"))
                                displayName.resize(displayName.size() - 5);

                            ImGui::TextDisabled("%s", displayName.c_str());
                            ImGui::Separator();

                            if (ImGui::MenuItem("Delete Prefab"))
                            {
                                PrefabManager prefabMgr;
                                std::string prefabName = item.name;
                                if (prefabName.ends_with(".json"))
                                    prefabName.resize(prefabName.size() - 5);

                                if (prefabMgr.DeletePrefab(prefabName))
                                {
                                    std::cout << "[ProjectPanel] Successfully deleted prefab: " << prefabName << std::endl;
                                    RefreshCurrentDir();
                                }
                                else
                                {
                                    std::cerr << "[ProjectPanel] Failed to delete prefab: " << prefabName << std::endl;
                                }
                            }

                            ImGui::EndPopup();
                        }
                    }
                }

                // Drag source (image files only)
                auto IsImageExtStr = [](const std::string& s)
                    {
                        std::string ext;
                        auto dot = s.find_last_of('.');
                        if (dot != std::string::npos)
                            ext = s.substr(dot);

                        for (char& c : ext)
                            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

                        return ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                            ext == ".bmp" || ext == ".tga" || ext == ".dds";
                    };

                if (IsImageExtStr(item.fullPath))
                {
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        const char* path = item.fullPath.c_str();
                        ImGui::SetDragDropPayload("ASSET_TEXTURE",
                            path,
                            static_cast<int>(std::strlen(path)) + 1);
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

                // Left-click: prefab instantiate OR assign texture to selected entity
                if (clicked)
                {
                    if (item.fullPath.ends_with(".json"))
                    {
                        PrefabManager prefabMgr;

                        std::string prefabName = item.name;
                        if (prefabName.ends_with(".json"))
                            prefabName.resize(prefabName.size() - 5);

                        auto result = prefabMgr.InstantiatePrefab(prefabName);
                        if (result.has_value())
                        {
                            Entity newEntity = result.value();
                            entityDisplayOrder.push_back(newEntity);
                            selectedEntity = newEntity;

                            std::cout << "[Editor] Instantiated prefab: " << prefabName
                                << " (Entity " << newEntity << ")\n";
                        }
                        else
                        {
                            std::cerr << "[Editor] Failed to instantiate prefab: "
                                << prefabName << "\n";
                        }
                    }
                    else if (selectedEntity != INVALID_ENTITY)
                    {
                        auto* coord = Coordinator::GetInstance();
                        if (coord &&
                            coord->HasComponent<Framework::Renderable>(selectedEntity))
                        {
                            auto& rend = coord->GetComponent<Framework::Renderable>(selectedEntity);
                            // Convert to relative path with ../../PulseEngine/Assets prefix
                            namespace fs = std::filesystem;
                            std::string pathStr = item.fullPath;
                            std::replace(pathStr.begin(), pathStr.end(), '\\', '/');

                            size_t myEnginePos = pathStr.find("PulseEngine/Assets");
                            if (myEnginePos != std::string::npos)
                            {
                                rend.spriteName = "../../" + pathStr.substr(myEnginePos);
                            }
                            else
                            {
                                rend.spriteName = pathStr;
                            }
                            rend.textureID = mAssets.GetOrLoadTexture(rend.spriteName);

                            // Auto-detect spritesheet based on filename
                            std::string filename = fs::path(rend.spriteName).filename().string();
                            std::string lowerName = filename;
                            for (auto& c : lowerName) c = (char)tolower(c);

                            bool isSheet = (lowerName.find("sheet") != std::string::npos ||
                                lowerName.find("atlas") != std::string::npos);

                            rend.shd_ref = isSheet ? 2.0f : 1.0f;  // 2 for spritesheet, 1 for sprite
                        }
                    }
                }
            }

            // Label under thumbnail
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + s_thumbSize);
            std::string displayName = item.name;
            if (displayName.ends_with(".json"))
                displayName.resize(displayName.size() - 5);
            ImGui::TextUnformatted(displayName.c_str());
            ImGui::PopTextWrapPos();

            ImGui::PopID();
            ImGui::EndGroup();
            ++itemIndex;
        }

        // =====================================================================
        // DROP ZONE: Accept entity drops in empty space when INSIDE Prefab folder
        // =====================================================================

        // Check if current directory is the Prefab folder
        namespace fs = std::filesystem;
        std::string currentFolderName = fs::path(s_currentDir).filename().string();
        bool isInPrefabFolder = (currentFolderName == "Prefab" || currentFolderName == "Prefabs" ||
            currentFolderName == "prefab" || currentFolderName == "prefabs");

        if (isInPrefabFolder)
        {
            // Create an invisible button covering the empty space
            ImVec2 availSpace = ImGui::GetContentRegionAvail();
            ImGui::InvisibleButton("##prefab_dropzone", availSpace);

            // Add drop target to this empty space
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY_REORDER", ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
                {
                    // Extract entity from the payload
                    struct HierarchyDragData {
                        int index;
                        Entity entity;
                    };

                    const HierarchyDragData* dragData = (const HierarchyDragData*)payload->Data;
                    Entity droppedEntity = dragData->entity;

                    std::cout << "[ProjectPanel] Entity " << droppedEntity << " dropped into Prefabs folder (empty space)\n";

                    // Get entity name for prefab
                    auto* coord = Coordinator::GetInstance();
                    if (coord && droppedEntity != INVALID_ENTITY)
                    {
                        std::string prefabName = "Prefab";

                        // Try to get the entity's name
                        if (coord->HasComponent<Name>(droppedEntity))
                        {
                            prefabName = coord->GetComponent<Name>(droppedEntity).name;
                        }
                        else
                        {
                            prefabName = "Entity_" + std::to_string(static_cast<int>(droppedEntity));
                        }

                        // =====================================================
                        // Check if entity can be saved as prefab
                        // =====================================================
                        PrefabManager prefabMgr;

                        if (!prefabMgr.CanSaveAsPrefab(droppedEntity))
                        {
                            // Entity is engine-managed - show warning popup
                            s_showEngineManagedWarning = true;
                            s_blockedEntityName = prefabName;
                            //std::cout << "[ProjectPanel] Blocked: Cannot save engine-managed entity as prefab: " << prefabName << std::endl;
                        }
                        else
                        {
                            // Safe to save
                            if (prefabMgr.SaveEntityAsPrefab(droppedEntity, prefabName))
                            {
                                //std::cout << "[ProjectPanel] Successfully created prefab: " << prefabName << ".json" << std::endl;
                                RefreshCurrentDir();  // Refresh to show the new prefab file
                            }
                            else
                            {
                                std::cerr << "[ProjectPanel] Failed to create prefab: " << prefabName << ".json" << std::endl;
                            }
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }

        // =====================================================
        // Warning Popup for Engine-Managed Entities (Debug Text) 
        // =====================================================
        if (s_showEngineManagedWarning)
        {
            ImGui::OpenPopup("Cannot Save as Prefab##EngineManagedWarning");
            s_showEngineManagedWarning = false;
        }

        if (ImGui::BeginPopupModal("Cannot Save as Prefab##EngineManagedWarning", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "  Warning  ");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Cannot save '%s' as prefab.", s_blockedEntityName.c_str());
            ImGui::Spacing();
            ImGui::TextWrapped("Entities with names starting with '_' are debug texts "
                "(e.g., _FPS_Display, _TopSystem_Display) and cannot be saved as prefabs.");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Center the OK button
            float buttonWidth = 120.0f;
            float windowWidth = ImGui::GetWindowSize().x;
            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

            if (ImGui::Button("OK", ImVec2(buttonWidth, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            // Also close on Enter key
            if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Escape))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        // =====================================================
        // Error Popup for Invalid Audio File Import
        // =====================================================
        if (s_showAudioImportError)
        {
            ImGui::OpenPopup("Invalid Audio Format##AudioImportError");
            s_showAudioImportError = false;
        }

        if (ImGui::BeginPopupModal("Invalid Audio Format##AudioImportError", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "  Error  ");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Cannot import '%s' into %s folder.",
                s_invalidAudioFileName.c_str(),
                s_targetFolderName.c_str());
            ImGui::Spacing();
            ImGui::TextWrapped("Supported audio formats: .mp3, .wav");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Center the OK button
            float buttonWidth = 120.0f;
            float windowWidth = ImGui::GetWindowSize().x;
            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

            if (ImGui::Button("OK", ImVec2(buttonWidth, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            // Also close on Enter key
            if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Escape))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::EndChild();

        // Re-enable if we disabled for play mode
        if (isPlaying)
        {
            ImGui::EndDisabled();
        }
    }
}