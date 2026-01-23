/******************************************************************************/
/**
 * @file        ProjectPanelHelpers.cpp
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 *
 * @brief       Implementation of ProjectPanel helper functions.
 *              Handles file validation, directory operations, drag-drop,
 *              and UI drawing utilities.
 *
 * @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "ProjectPanelHelper.h"

#include "CoreEngine/Asset/AssetsManager.h"
#include "CoreEngine/ECS/Coordinator.h"
#include "Graphics/Renderable.h"
#include "Graphics/Transform.h"

namespace PulseEditor
{
    // =============================================================
    // HELPER FUNCTION: Convert Absolute Path to Relative Path
    // =============================================================

    /**
     * Converts an absolute path to a path relative to the Assets folder.
     * This works regardless of where the project folder is located.
     *
     * Example:
     *   Input:  "C:/Users/Me/Project/PulseEngine/Assets/Textures/UI/button.png"
     *   Output: "Assets/Textures/UI/button.png"  (relative to executable)
     */
    std::string GetRelativeAssetPath(const std::string& absolutePath)
    {
        std::string pathStr = absolutePath;
        std::replace(pathStr.begin(), pathStr.end(), '\\', '/');

        // Convert absolute path to relative
        size_t assetsPos = pathStr.find("Assets/");
        if (assetsPos != std::string::npos)
        {
            return "../../PulseEngine/" + pathStr.substr(assetsPos);
        }

        // Fallback: return as-is
        return pathStr;
    }

    // =============================================================
    // FILE VALIDATION HELPERS
    // =============================================================

    bool IsImageExt(const std::string& path)
    {
        const auto dot = path.find_last_of('.');
        if (dot == std::string::npos)
            return false;

        std::string ext = path.substr(dot + 1);
        for (char& c : ext)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        return (ext == "png" || ext == "jpg" || ext == "jpeg");
    }

    bool IsPrefabFolder(const std::string& folderName)
    {
        // Use existing FolderValidation system
        return GetFolderType(folderName) == FolderType::Prefab;
    }

    // =============================================================
    // DIRECTORY OPERATIONS
    // =============================================================

    void RefreshDirectory(const std::string& assetRoot, const std::string& currentDir, std::vector<AssetItem>& entries)
    {
        entries.clear();

        namespace fs = std::filesystem;
        std::error_code ec;

        fs::path cur = fs::weakly_canonical(fs::absolute(currentDir), ec);
        if (ec || cur.empty() || !fs::exists(cur, ec))
            return;

        fs::path root = fs::weakly_canonical(fs::absolute(assetRoot), ec);

        // Add ".." (only if not at the asset root)
        if (!ec && cur != root)
        {
            AssetItem up;
            up.name = "..";
            up.fullPath = fs::weakly_canonical(cur.parent_path(), ec).string();
            up.isDir = true;
            entries.push_back(up);
        }

        // List the directory (non-recursive)
        for (auto const& entry : fs::directory_iterator(cur, ec))
        {
            AssetItem it;
            it.fullPath = fs::weakly_canonical(entry.path(), ec).string();
            it.name = entry.path().filename().string();
            it.isDir = entry.is_directory();
            entries.push_back(std::move(it));
        }

        // Sort: folders first, then alphabetically
        std::sort(entries.begin(), entries.end(),
            [](const AssetItem& a, const AssetItem& b)
            {
                if (a.isDir != b.isDir)
                    return a.isDir > b.isDir;
                return a.name < b.name;
            });
    }

    void ValidateFolderContents(const std::string& currentDir, const std::vector<AssetItem>& entries)
    {
        FolderType folderType = GetFolderType(currentDir);

        for (const auto& item : entries)
        {
            if (!item.isDir && !IsValidExtensionForFolder(folderType, item.fullPath))
            {
                std::cerr << "[ProjectPanel] Warning: Invalid file in "
                    << GetFolderTypeName(folderType) << " folder: "
                    << item.name
                    << " (Expected: " << GetAllowedExtensions(folderType) << ")"
                    << std::endl;
            }
        }
    }

    // =============================================================
    // TEXTURE OPERATIONS
    // =============================================================

    void EnsureThumbnailLoaded(AssetItem& item, const std::string& currentDir)
    {
        if (item.triedLoad) return;
        item.triedLoad = true;

        if (item.isDir) return;

        if (!IsImageExt(item.fullPath)) return;

        // =============================================================
        // NOTE: We intentionally do NOT validate folder type here.
        // We want to SHOW thumbnails for all images regardless of 
        // whether they're in the "correct" folder type.
        // Invalid files are already highlighted in red by DrawFileLabel().
        // =============================================================
        (void)currentDir;  // Unused now, kept for API compatibility

        // =============================================================
        // Convert absolute path to relative path for AssetsManager
        // =============================================================
        std::string pathStr = GetRelativeAssetPath(item.fullPath);
        std::replace(pathStr.begin(), pathStr.end(), '\\', '/');

        item.texID = mAssets.GetOrLoadTexture(pathStr);
    }

    bool ReplaceTextureFile(const std::string& dstPath, const std::string& srcPath, std::vector<AssetItem>& dirEntries)
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
                return ext == ".png" || ext == ".jpg" || ext == ".jpeg";
            };

        if (!IsImage(srcPath))
            return false;

        // 1) Overwrite target file
        fs::copy_file(srcPath, dstPath, fs::copy_options::overwrite_existing, ec);
        if (ec)
        {
            std::cerr << "[ProjectPanel] Replace failed: " << srcPath << " -> "
                << dstPath << " (" << ec.message() << ")\n";
            return false;
        }

        // 2) Force a fresh GL texture id and cache entry
        std::string relativePath = GetRelativeAssetPath(dstPath);
        mAssets.EvictTexture(relativePath);
        GLuint newTex = mAssets.GetOrLoadTexture(relativePath);

        if (!newTex)
        {
            std::cerr << "[ProjectPanel] Reload failed for: " << dstPath << "\n";
            return false;
        }

        // 3) Update Project thumbnails
        for (auto& it : dirEntries)
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

                // Compare both the full path and just the filename
                // This handles cases where spriteName might be absolute or relative
                bool pathMatches = (r.spriteName == dstPath || r.spriteName == relativePath);
                bool nameMatches = (rBase == dBase);

                if (!r.spriteName.empty() && (pathMatches || nameMatches))
                {
                    r.spriteName = dstPath;  // Use the absolute path (dstPath)
                    r.textureID = newTex;
                }
            }
        }

        std::cout << "[ProjectPanel] Replaced & hot-swapped: " << dstPath
            << "  <=  " << srcPath << "\n";
        return true;
    }

    // =============================================================
    // DRAG & DROP HANDLERS
    // =============================================================

    void HandleOSFileDrop(const std::vector<std::string>& droppedPaths, const std::string& currentDir, bool& showImportError, std::string& invalidFileName, std::string& targetFolderName, FolderType& targetFolderType)
    {
        namespace fs = std::filesystem;
        std::error_code ec;

        FolderType folderType = GetFolderType(currentDir);
        std::string currentFolderName = fs::path(currentDir).filename().string();

        bool hadInvalidFile = false;
        std::string firstInvalidFile;
        int changed = 0;

        for (const auto& srcPath : droppedPaths)
        {
            if (!fs::exists(srcPath, ec))
                continue;

            // Validate file type for this folder
            if (!IsValidExtensionForFolder(folderType, srcPath))
            {
                if (!hadInvalidFile)
                {
                    hadInvalidFile = true;
                    firstInvalidFile = fs::path(srcPath).filename().string();
                }
                std::cout << "[ProjectPanel] Blocked import: " << fs::path(srcPath).filename().string()
                    << " (Invalid for " << GetFolderTypeName(folderType) << " folder)" << std::endl;
                continue;
            }

            // Copy file to current directory
            fs::path dst = fs::path(currentDir) / fs::path(srcPath).filename();

            if (fs::exists(dst, ec))
            {
                // Handle replacement
                fs::copy_file(srcPath, dst, fs::copy_options::overwrite_existing, ec);
            }
            else
            {
                fs::copy_file(srcPath, dst, ec);
            }

            if (!ec)
            {
                ++changed;
                std::cout << "[ProjectPanel] Imported: " << fs::path(srcPath).filename().string() << std::endl;
            }
        }

        if (hadInvalidFile)
        {
            showImportError = true;
            invalidFileName = firstInvalidFile;
            targetFolderName = currentFolderName;
            targetFolderType = folderType;
        }
    }

    void HandleEntityDropToPrefab(Entity droppedEntity,
        bool& showWarning,
        std::string& blockedName)
    {
        auto* coord = Coordinator::GetInstance();
        if (!coord || droppedEntity == INVALID_ENTITY)
            return;

        std::string prefabName = "Prefab";

        // Get entity name
        if (coord->HasComponent<Name>(droppedEntity))
        {
            prefabName = coord->GetComponent<Name>(droppedEntity).name;
        }
        else
        {
            prefabName = "Entity_" + std::to_string(static_cast<int>(droppedEntity));
        }

        PrefabManager prefabMgr;

        if (!prefabMgr.CanSaveAsPrefab(droppedEntity))
        {
            showWarning = true;
            blockedName = prefabName;
            std::cout << "[ProjectPanel] Blocked: Cannot save engine-managed entity as prefab: "
                << prefabName << std::endl;
        }
        else
        {
            if (prefabMgr.SaveEntityAsPrefab(droppedEntity, prefabName))
            {
                std::cout << "[ProjectPanel] Successfully created prefab: "
                    << prefabName << ".json" << std::endl;
            }
            else
            {
                std::cerr << "[ProjectPanel] Failed to create prefab: "
                    << prefabName << ".json" << std::endl;
            }
        }
    }

    // =============================================================
    // UI DRAWING HELPERS
    // =============================================================

    void DrawFileLabel(const AssetItem& item, float thumbSize, const std::string& currentDir)
    {
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + thumbSize);

        std::string displayName = item.name;
        if (displayName.ends_with(".json"))
            displayName.resize(displayName.size() - 5);

        // =============================================================
        // VALIDATE FILE TYPE FOR CURRENT FOLDER
        // Invalid files show in RED with tooltip warning
        // =============================================================
        bool isInvalidFile = false;
        FolderType currentFolderType = FolderType::Generic;

        if (!item.isDir)
        {
            currentFolderType = GetFolderType(currentDir);
            isInvalidFile = !IsValidExtensionForFolder(currentFolderType, item.fullPath);
        }

        // Red text for invalid files
        if (isInvalidFile)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        }

        ImGui::TextUnformatted(displayName.c_str());

        // Check hover IMMEDIATELY after the text
        bool isHovered = ImGui::IsItemHovered();

        if (isInvalidFile)
        {
            ImGui::PopStyleColor();

            // Tooltip on hover explaining the issue
            if (isHovered)
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Invalid file type!");
                ImGui::Text("This file doesn't belong in %s folder.", GetFolderTypeName(currentFolderType));
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Expected: %s", GetAllowedExtensions(currentFolderType));
                ImGui::EndTooltip();
            }
        }

        ImGui::PopTextWrapPos();
    }

    // =============================================================
    // POPUP MODALS
    // =============================================================

    void DrawEngineManagedWarningPopup(bool& showWarning, const std::string& blockedName)
    {
        if (showWarning)
        {
            ImGui::OpenPopup("Cannot Save as Prefab##EngineManagedWarning");
            showWarning = false;
        }

        if (ImGui::BeginPopupModal("Cannot Save as Prefab##EngineManagedWarning", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "  Warning  ");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Cannot save '%s' as prefab.", blockedName.c_str());
            ImGui::Spacing();
            ImGui::TextWrapped("Entities with names starting with '_' are debug texts "
                "(e.g., _FPS_Display, _TopSystem_Display) and cannot be saved as prefabs.");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            float buttonWidth = 120.0f;
            float windowWidth = ImGui::GetWindowSize().x;
            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

            if (ImGui::Button("OK", ImVec2(buttonWidth, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Escape))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void DrawImportErrorPopup(bool& showError, const std::string& invalidFile, const std::string& targetFolder, FolderType targetType)
    {
        if (showError)
        {
            ImGui::OpenPopup("Invalid File Type##ImportError");
            showError = false;
        }

        if (ImGui::BeginPopupModal("Invalid File Type##ImportError", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "  Import Error  ");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Cannot import '%s' into '%s' folder.",
                invalidFile.c_str(),
                targetFolder.c_str());

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Allowed file types: %s",
                GetAllowedExtensions(targetType));

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            float buttonWidth = 120.0f;
            float windowWidth = ImGui::GetWindowSize().x;
            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

            if (ImGui::Button("OK", ImVec2(buttonWidth, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Escape))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

}  // namespace PulseEditor