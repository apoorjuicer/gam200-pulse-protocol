/******************************************************************************/
/**
 * @file        ProjectPanel.cpp
 * @project     Pulse Protocol
 * @author      Leu Jun Yong (primary) - 80%
 *              Chloe Lau Rey En (secondary) - 20%
 * 
 * @brief       Implements the Project Panel (asset browser) for the editor.
 *              Helper functions moved to ProjectPanelHelpers.cpp for readability.
 *
 *              Features:
 *              - Directory browsing with navigation and thumbnail previews
 *              - Adjustable thumbnail size via slider
 *              - Drag-and-drop texture assignment to selected entities
 *              - Drag-and-drop entity-to-prefab creation (from Hierarchy)
 *              - OS file import via external drag-and-drop
 *              - Texture hot-swapping: drop image onto another to replace
 *              - Prefab instantiation on click, deletion via context menu
 *              - Right-click delete for all files and folders
 *              - Auto-detection of spritesheets based on filename keywords
 *              - Editing disabled during play mode
 *              - Folder-type validation for imports (Audio, Texture, Font, Prefab)
 *              - Invalid file highlighting (red text + tooltip)
 *
 *              Prefab Drop Zones:
 *              - Dropping onto Prefab folder button (from parent directory)
 *              - Dropping into empty space inside Prefab folder
 *              Both zones validate entities via PrefabManager::CanSaveAsPrefab()
 *              and display a warning popup for engine-managed entities
 *              (names starting with '_', e.g., _FPS_Display).
 *
 * @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "ProjectPanel.h"
#include "ProjectPanelHelper.h"
#include "FolderValidation.h"

#include "CoreEngine/Asset/AssetsManager.h"
#include "CoreEngine/ECS/Coordinator.h"
#include "Graphics/Renderable.h"
#include "Graphics/Transform.h"

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

    static std::vector<std::string> s_pendingOSDrops;

    // Popup states
    static bool s_showEngineManagedWarning = false;
    static std::string s_blockedEntityName = "";
    static bool s_showImportError = false;
    static std::string s_invalidFileName = "";
    static std::string s_targetFolderName = "";
    static FolderType s_targetFolderType = FolderType::Generic;

    // ══════════════════════════════════════════════════════════════
    // LOCAL HELPER - Refresh wrapper
    // Calls RefreshDirectory() and ValidateFolderContents() from 
    // ProjectPanelHelpers.cpp
    // ══════════════════════════════════════════════════════════════
    static void RefreshCurrentDir()
    {
        RefreshDirectory(s_assetRoot, s_currentDir, s_dirEntries);
        ValidateFolderContents(s_currentDir, s_dirEntries);
    }

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

    void SetupDropCallback(GLFWwindow* window)
    {
        glfwSetDropCallback(window,
            [](GLFWwindow*, int count, const char** paths)
            {
                for (int i = 0; i < count; ++i)
                {
                    PushOSDropPath(paths[i]);
                }
            });
    }

    // ── Main panel UI ──────────────────────────────────────────
    void ProjectPanel()
    {
        // Disable all editing when playing
        bool isPlaying = IsPlaying();

        // ──────────────────────────────────────────────────────────
        // HEADER: Refresh button and path display
        // ──────────────────────────────────────────────────────────
        ImGui::Text("Project");
        ImGui::SameLine();

        if (ImGui::Button("Refresh"))
            RefreshCurrentDir();

        ImGui::SameLine();
        ImGui::Text("%s", s_currentDir.c_str());

        ImGui::Separator();

        // Thumbnail size slider
        ImGui::Text("Size");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("##ThumbSize", &s_thumbSize, 48.0f, 128.0f, "%.0f");

        if (isPlaying)
        {
            ImGui::BeginDisabled();
        }

        // ──────────────────────────────────────────────────────────
        // HANDLE OS FILE DROPS
        // Uses HandleOSFileDrop() from ProjectPanelHelpers.cpp
        // ──────────────────────────────────────────────────────────
        if (!s_pendingOSDrops.empty())
        {
            HandleOSFileDrop(s_pendingOSDrops, s_currentDir,
                s_showImportError, s_invalidFileName,
                s_targetFolderName, s_targetFolderType);
            s_pendingOSDrops.clear();
            RefreshCurrentDir();
        }

        // ──────────────────────────────────────────────────────────
        // ASSET GRID
        // ──────────────────────────────────────────────────────────
        ImGui::BeginChild("AssetsGrid", ImVec2(0, 0), false,
            ImGuiWindowFlags_HorizontalScrollbar);

        float availX = ImGui::GetContentRegionAvail().x;
        float cellSize = s_thumbSize + s_thumbPadding;
        int cols = static_cast<int>(availX / cellSize);
        if (cols < 1) cols = 1;

        int itemIndex = 0;
        for (auto& item : s_dirEntries)
        {
            if (itemIndex % cols != 0)
                ImGui::SameLine();

            ImGui::BeginGroup();
            ImGui::PushID(itemIndex);

            ImVec2 thumbSize(s_thumbSize, s_thumbSize);
            bool clicked = false;

            if (item.isDir)
            {
                // ══════════════════════════════════════════════════
                // FOLDER ITEM
                // ══════════════════════════════════════════════════
                if (ImGui::Button(item.name.c_str(), thumbSize))
                {
                    s_currentDir = item.fullPath;
                    RefreshCurrentDir();
                    ImGui::PopID();
                    ImGui::EndGroup();
                    ImGui::EndChild();
                    if (isPlaying) ImGui::EndDisabled();
                    return;
                }

                // Folder context menu (delete)
                if (item.name != ".." && ImGui::BeginPopupContextItem(""))
                {
                    ImGui::TextDisabled("%s", item.name.c_str());
                    ImGui::Separator();

                    if (ImGui::MenuItem("Delete Folder"))
                    {
                        namespace fs = std::filesystem;
                        std::error_code ec;
                        if (fs::remove_all(item.fullPath, ec) > 0)
                        {
                            std::cout << "[ProjectPanel] Deleted folder: " << item.name << std::endl;
                            RefreshCurrentDir();
                        }
                        else
                        {
                            std::cerr << "[ProjectPanel] Failed to delete folder: " << item.name
                                << " (" << ec.message() << ")" << std::endl;
                        }
                    }
                    ImGui::EndPopup();
                }

                // ══════════════════════════════════════════════════
                // PREFAB FOLDER DROP TARGET
                // Uses IsPrefabFolder() and HandleEntityDropToPrefab() 
                // from ProjectPanelHelpers.cpp
                // ══════════════════════════════════════════════════
                if (IsPrefabFolder(item.name) && ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY_REORDER", ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
                    {
                        struct HierarchyDragData { int index; Entity entity; };
                        const HierarchyDragData* data = (const HierarchyDragData*)payload->Data;

                        HandleEntityDropToPrefab(data->entity, s_showEngineManagedWarning, s_blockedEntityName);
                        RefreshCurrentDir();
                    }
                    ImGui::EndDragDropTarget();
                }
            }
            else
            {
                // ══════════════════════════════════════════════════
                // FILE ITEM
                // Uses EnsureThumbnailLoaded() from ProjectPanelHelpers.cpp
                // ══════════════════════════════════════════════════
                EnsureThumbnailLoaded(item, s_currentDir);

                // Draw thumbnail or button
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
                }

                // ══════════════════════════════════════════════════════════════
                // THUMBNAIL TOOLTIP FOR INVALID FILES
                // ══════════════════════════════════════════════════════════════
                if (!item.isDir && ImGui::IsItemHovered())
                {
                    FolderType folderType = GetFolderType(s_currentDir);
                    if (!IsValidExtensionForFolder(folderType, item.fullPath))
                    {
                        ImGui::BeginTooltip();
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Invalid file type!");
                        ImGui::Text("This file doesn't belong in %s folder.", GetFolderTypeName(folderType));
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Expected: %s", GetAllowedExtensions(folderType));
                        ImGui::EndTooltip();
                    }
                }

                // ══════════════════════════════════════════════════
                // FILE CONTEXT MENU (DELETE)
                // ══════════════════════════════════════════════════
                if (ImGui::BeginPopupContextItem(""))
                {
                    std::string displayName = item.name;
                    if (displayName.ends_with(".json"))
                        displayName.resize(displayName.size() - 5);

                    ImGui::TextDisabled("%s", displayName.c_str());
                    ImGui::Separator();

                    if (ImGui::MenuItem("Delete"))
                    {
                        namespace fs = std::filesystem;
                        std::error_code ec;

                        // Only treat as prefab if it's a .json file IN a Prefab folder
                        FolderType currentFolderType = GetFolderType(s_currentDir);
                        bool isPrefabFile = item.fullPath.ends_with(".json") &&
                            currentFolderType == FolderType::Prefab;

                        if (isPrefabFile)
                        {
                            // Delete prefab via PrefabManager
                            PrefabManager prefabMgr;
                            std::string prefabName = item.name;
                            if (prefabName.ends_with(".json"))
                                prefabName.resize(prefabName.size() - 5);

                            if (prefabMgr.DeletePrefab(prefabName))
                            {
                                std::cout << "[ProjectPanel] Deleted prefab: " << prefabName << std::endl;
                            }
                        }
                        else
                        {
                            // Delete regular file
                            if (IsImageExt(item.fullPath))
                                mAssets.EvictTexture(item.fullPath);

                            if (fs::remove(item.fullPath, ec))
                            {
                                std::cout << "[ProjectPanel] Deleted file: " << item.name << std::endl;
                            }
                            else
                            {
                                std::cerr << "[ProjectPanel] Failed to delete: " << item.name
                                    << " (" << ec.message() << ")" << std::endl;
                            }
                        }
                        RefreshCurrentDir();
                    }
                    ImGui::EndPopup();
                }

                // ══════════════════════════════════════════════════
                // IMAGE DRAG SOURCE (for texture assignment)
                // Uses IsImageExt() from ProjectPanelHelpers.cpp
                // ══════════════════════════════════════════════════
                if (IsImageExt(item.fullPath) && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                {
                    const char* path = item.fullPath.c_str();
                    ImGui::SetDragDropPayload("ASSET_TEXTURE", path, static_cast<int>(strlen(path)) + 1);
                    ImGui::TextUnformatted(("Texture: " + item.name).c_str());
                    ImGui::EndDragDropSource();
                }

                // ══════════════════════════════════════════════════
                // IMAGE DRAG TARGET (for texture hot-swap replacement)
                // Uses IsImageExt() and ReplaceTextureFile() from 
                // ProjectPanelHelpers.cpp
                // Note: ReplaceTextureFile now takes s_dirEntries to update thumbnails
                // ══════════════════════════════════════════════════
                if (IsImageExt(item.fullPath) && ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("ASSET_TEXTURE"))
                    {
                        const char* src = static_cast<const char*>(p->Data);
                        if (src && *src)
                        {
                            std::string srcPath = src;
                            if (srcPath != item.fullPath)
                            {
                                // Full hot-swap: updates file, cache, thumbnails, and live scene entities
                                ReplaceTextureFile(item.fullPath, srcPath, s_dirEntries);
                                RefreshCurrentDir();
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                // ══════════════════════════════════════════════════
                // CLICK HANDLING
                // ══════════════════════════════════════════════════
                if (clicked)
                {
                    FolderType currentFolderType = GetFolderType(s_currentDir);

                    // DEBUG - Remove later
                    std::cout << "[DEBUG] Clicked: " << item.name
                        << " | Folder: " << s_currentDir
                        << " | FolderType: " << static_cast<int>(currentFolderType)
                        << " | IsValid: " << IsValidExtensionForFolder(currentFolderType, item.fullPath)
                        << std::endl;

                    // ── PREFAB: Only instantiate if in Prefab folder ──
                    if (item.fullPath.ends_with(".json") && currentFolderType == FolderType::Prefab)
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
                            std::cout << "[ProjectPanel] Instantiated prefab: " << prefabName
                                << " (Entity " << newEntity << ")" << std::endl;
                        }
                        else
                        {
                            std::cerr << "[ProjectPanel] Failed to instantiate prefab: " << prefabName << std::endl;
                        }
                    }
                    // ── TEXTURE: Only assign if valid image in correct folder ──
                    else if (selectedEntity != INVALID_ENTITY &&
                        IsImageExt(item.fullPath) &&
                        IsValidExtensionForFolder(currentFolderType, item.fullPath))
                    {
                        // ── TEXTURE: Assign to selected entity ──
                        auto* coord = Coordinator::GetInstance();
                        if (coord && coord->HasComponent<Framework::Renderable>(selectedEntity))
                        {
                            auto& rend = coord->GetComponent<Framework::Renderable>(selectedEntity);

                            namespace fs = std::filesystem;

                            // Use GetRelativeAssetPath to get the correct absolute path
                            std::string spritePath = GetRelativeAssetPath(item.fullPath);

                            rend.spriteName = spritePath;
                            rend.textureID = mAssets.GetOrLoadTexture(rend.spriteName);

                            // Auto-detect spritesheet based on filename
                            std::string filename = fs::path(rend.spriteName).filename().string();
                            std::string lowerName = filename;
                            for (auto& c : lowerName) c = static_cast<char>(tolower(c));

                            bool isSheet = (lowerName.find("sheet") != std::string::npos ||
                                lowerName.find("atlas") != std::string::npos);
                            rend.shd_ref = isSheet ? 2.0f : 1.0f;

                            std::cout << "[ProjectPanel] Assigned texture to entity " << selectedEntity << std::endl;
                        }
                    }
                }
            }

            // ══════════════════════════════════════════════════════
            // DRAW LABEL UNDER THUMBNAIL
            // Uses DrawFileLabel() from ProjectPanelHelpers.cpp
            // Shows RED text for invalid files + tooltip on hover
            // ══════════════════════════════════════════════════════
            DrawFileLabel(item, s_thumbSize, s_currentDir);

            ImGui::PopID();
            ImGui::EndGroup();
            ++itemIndex;
        }

        // ──────────────────────────────────────────────────────────
        // PREFAB DROP ZONE (empty space inside Prefab folder)
        // Uses IsPrefabFolder() and HandleEntityDropToPrefab() from
        // ProjectPanelHelpers.cpp
        // ──────────────────────────────────────────────────────────
        namespace fs = std::filesystem;
        std::string currentFolderName = fs::path(s_currentDir).filename().string();

        if (IsPrefabFolder(currentFolderName))
        {
            ImVec2 availSpace = ImGui::GetContentRegionAvail();
            if (availSpace.x > 0 && availSpace.y > 0)
            {
                ImGui::InvisibleButton("##prefab_dropzone", availSpace);

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY_REORDER", ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
                    {
                        struct HierarchyDragData { int index; Entity entity; };
                        const HierarchyDragData* data = (const HierarchyDragData*)payload->Data;

                        std::cout << "[ProjectPanel] Entity " << data->entity
                            << " dropped into Prefabs folder (empty space)" << std::endl;

                        HandleEntityDropToPrefab(data->entity, s_showEngineManagedWarning, s_blockedEntityName);
                        RefreshCurrentDir();
                    }
                    ImGui::EndDragDropTarget();
                }
            }
        }

        ImGui::EndChild();

        // ──────────────────────────────────────────────────────────
        // POPUP MODALS
        // Uses DrawEngineManagedWarningPopup() and DrawImportErrorPopup()
        // from ProjectPanelHelpers.cpp
        // ──────────────────────────────────────────────────────────
        DrawEngineManagedWarningPopup(s_showEngineManagedWarning, s_blockedEntityName);
        DrawImportErrorPopup(s_showImportError, s_invalidFileName, s_targetFolderName, s_targetFolderType);

        if (isPlaying)
            ImGui::EndDisabled();
    }

}  // namespace PulseEditor