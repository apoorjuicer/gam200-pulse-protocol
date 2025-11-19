/******************************************************************************/
/**
 * @file        Inspector.cpp
 * @project     Pulse Protocol
 * @author      Leu Jun Yong (primary) - 70%
 *              Chloe Lau Rey En (secondary) - 30%
 * @brief       TBF
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

// -----------------------------------------------------------------------------
// Inspector.cpp  -  Entity inspector (components UI, prefab save, etc.)
// -----------------------------------------------------------------------------

#include "pch/pch_temp.h"

#include "Inspector.h"
#include "Editor.h"                       // selectedEntity, INVALID_ENTITY, PrefabManager, etc.
#include "../Graphics/glapp.h"           // GLApp::shdrpgms
#include "../Graphics/Layer.h"           // GLApp::shdrpgms
#include "CoreEngine/Asset/AssetsManager.h"
#include "CoreEngine/Prefab/PrefabInstance.h"

namespace MyEditor
{
    // -------------------------------------------------------------------------
    // Local helper: parse "<rows>x<cols>" from filename ("hero_4x3.png")
    // (copied from your old Editor.cpp; now lives with the inspector)
    // -------------------------------------------------------------------------
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

    // -------------------------------------------------------------------------
    // Main Inspector window
    // -------------------------------------------------------------------------
    void DrawInspectorWindow(ImGuiID rightDockID)
    {
        // Dock on the right
        if (rightDockID)
            ImGui::SetNextWindowDockID(rightDockID, ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Inspector"))
        {
            ImGui::End();
            return;
        }

        ImGui::TextUnformatted("Entity Inspector");
        ImGui::Separator();

        auto* g = Coordinator::GetInstance();
        if (!g)
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Coordinator not found!");
            ImGui::End();
            return;
        }

        if (selectedEntity == INVALID_ENTITY)
        {
            ImGui::TextUnformatted("No entity selected.");
            ImGui::End();
            return;
        }

        ImGui::Text("Entity ID: %d", (int)selectedEntity);

        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();

        // Create LayerTag if it doesn't exist
        if (!g->HasComponent<LayerTag>(selectedEntity))
        {
            LayerTag tag{};
            tag.mask = LAYER_WORLD;
            tag.orderInLayer = 1;
            tag.z = 0.0f;
            g->AddComponent<LayerTag>(selectedEntity, tag);
        }

        auto& layer = g->GetComponent<LayerTag>(selectedEntity);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.0f, 1.0f)); // Light blue
        ImGui::TextUnformatted("Layer:");
        ImGui::PopStyleColor();

        ImGui::SameLine();

        // Layer dropdown
        const char* layerNames[] = { "Background", "World", "UI" };
        int currentLayerIndex = 1; // Default to World

        // Convert LayerMask to dropdown index
        if (layer.mask == LAYER_BACKGROUND) currentLayerIndex = 0;
        else if (layer.mask == LAYER_WORLD) currentLayerIndex = 1;
        else if (layer.mask == LAYER_UI) currentLayerIndex = 2;

        ImGui::SetNextItemWidth(100.0f);
        if (ImGui::Combo("##LayerDropdown", &currentLayerIndex, layerNames, 3))
        {
            // Convert dropdown index back to LayerMask
            switch (currentLayerIndex)
            {
            case 0:
                layer.mask = LAYER_BACKGROUND;
                layer.orderInLayer = 0;
                layer.z = 0.0f;
                break;
            case 1:
                layer.mask = LAYER_WORLD;
                layer.orderInLayer = 1;
                layer.z = 0.0f;
                break;
            case 2:
                layer.mask = LAYER_UI;
                layer.orderInLayer = 10;
                layer.z = 100.0f;
                break;
            }
        }

        ImGui::Separator();

        // ---------------------------------------------------------------------
        // Prefab Instance Info
        // ---------------------------------------------------------------------
        if (g->HasComponent<PrefabInstance>(selectedEntity))
        {
            auto& prefabInstance = g->GetComponent<PrefabInstance>(selectedEntity);

            // Show prefab info in a colored header (like Unity)
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.7f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

            if (ImGui::CollapsingHeader("Prefab Instance", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::PopStyleColor(2);

                ImGui::Indent();

                // Show prefab name
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Prefab:");
                ImGui::SameLine();
                ImGui::Text("%s", prefabInstance.prefabName.c_str());

                // Linked checkbox
                bool isLinked = prefabInstance.isLinked;
                if (ImGui::Checkbox("Linked to Prefab", &isLinked))
                {
                    prefabInstance.isLinked = isLinked;
                    if (!isLinked) {
                        std::cout << "[Inspector] Unlinked entity " << selectedEntity
                            << " from prefab '" << prefabInstance.prefabName << "'" << std::endl;
                    }
                    else {
                        std::cout << "[Inspector] Linked entity " << selectedEntity
                            << " to prefab '" << prefabInstance.prefabName << "'" << std::endl;
                    }
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Uncheck to make this instance independent from the prefab.\n"
                        "When unchecked, this entity won't update when the prefab changes.");
                }

                ImGui::Spacing();

                // CRITICAL: Apply to Prefab button
                if (ImGui::Button("Apply to Prefab", ImVec2(-1, 30)))
                {
                    PrefabManager prefabMgr;

                    // Save this entity's current state as the new prefab definition
                    if (prefabMgr.SaveEntityAsPrefab(selectedEntity, prefabInstance.prefabName))
                    {
                        std::cout << "[Inspector] Saved entity " << selectedEntity
                            << " as prefab '" << prefabInstance.prefabName << "'" << std::endl;

                        // Update all other instances
                        prefabMgr.UpdateAllInstancesOfPrefab(prefabInstance.prefabName);

                        std::cout << "[Inspector] Updated all instances of prefab '"
                            << prefabInstance.prefabName << "'" << std::endl;
                    }
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Save this entity's current properties as the new prefab definition.\n"
                        "All other instances of this prefab will update to match.");
                }

                ImGui::Unindent();
            }
            else
            {
                ImGui::PopStyleColor(2);
            }

            ImGui::Separator();
        }

        // ---------------------------------------------------------------------
        // Right-click anywhere in inspector to add components
        // ---------------------------------------------------------------------
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

            //// GUIButton
            //if (!coord->HasComponent<UIElement>(selectedEntity))
            //{
            //    if (ImGui::MenuItem("UIElement"))
            //    {
            //        UIElement ui{};
            //        ui.elementID = "ui_element";
            //        ui.type = UIType::HEALTH_BAR;  // Default to health bar for now
            //        coord->AddComponent<UIElement>(selectedEntity, ui);
            //    }
            //} 

            ImGui::EndPopup();
        }

        // ---------------------------------------------------------------------
        // Name component
        // ---------------------------------------------------------------------
        if (g->HasComponent<Name>(selectedEntity))
        {
            auto& nm = g->GetComponent<Name>(selectedEntity);

            ImGui::TextUnformatted("Name");
            ImGui::SameLine();
            if (ImGui::Button("Remove Name"))
                g->RemoveComponent<Name>(selectedEntity);
            ImGui::Separator();

            static char nameBuf[256];
            std::strncpy(nameBuf, nm.name.c_str(), sizeof(nameBuf));
            nameBuf[sizeof(nameBuf) - 1] = '\0';
            if (ImGui::InputText("Entity Name", nameBuf, IM_ARRAYSIZE(nameBuf)))
                nm.name = nameBuf;

            ImGui::Separator();
        }

        // ---------------------------------------------------------------------
        // Transform component
        // ---------------------------------------------------------------------
        if (g->HasComponent<Framework::Transform>(selectedEntity))
        {
            auto& tr = g->GetComponent<Framework::Transform>(selectedEntity);
            ImGui::TextUnformatted("Transform");
            ImGui::SameLine();
            if (ImGui::Button("Remove Transform"))
                g->RemoveComponent<Framework::Transform>(selectedEntity);

            ImGui::Separator();

            float pos[2] = { tr.Pos.x, tr.Pos.y };
            if (ImGui::DragFloat2("Position", pos, 0.5f))
            { tr.Pos.x = pos[0]; tr.Pos.y = pos[1]; }

            float sc[2] = { tr.Scale.x, tr.Scale.y };
            if (ImGui::DragFloat2("Scale", sc, 0.5f))
            { tr.Scale.x = sc[0]; tr.Scale.y = sc[1]; }

            ImGui::DragFloat("Rotation (deg)", &tr.Rotation.x, 1.0f, -360.f, 360.f);
            ImGui::DragFloat("Rotation Speed", &tr.Rotation.y, 0.1f, -10.0f, 10.0f);
            ImGui::Checkbox("Visible", &tr.isVisible);

            ImGui::Separator();
        }

        // ---------------------------------------------------------------------
        // Renderable component
        // ---------------------------------------------------------------------
        if (g->HasComponent<Framework::Renderable>(selectedEntity))
        {
            auto& r = g->GetComponent<Framework::Renderable>(selectedEntity);

            ImGui::TextUnformatted("Renderable");
            ImGui::SameLine();
            if (ImGui::Button("Remove Renderable"))
                g->RemoveComponent<Framework::Renderable>(selectedEntity);
            ImGui::Separator();

            // --- Model reference (background vs sprite) ----------------------
            {
                int modelIdx = static_cast<int>(r.mdl_ref);
                modelIdx = std::clamp(modelIdx, 0, 1);

                const char* kModelNames[2] = { "Background Quad", "Sprite Quad" };

                ImGui::Text("Model: %s", kModelNames[modelIdx]);
                ImGui::SameLine();
                if (ImGui::SmallButton(" - ")) modelIdx = std::max(0, modelIdx - 1);
                ImGui::SameLine();
                if (ImGui::SmallButton(" + ")) modelIdx = std::min(1, modelIdx + 1);

                r.mdl_ref = static_cast<float>(modelIdx);
            }

            // --- Shader reference --------------------------------------------
            {
                const int maxShaders = static_cast<int>(GLApp::shdrpgms.size());
                int shaderIdx = std::clamp(static_cast<int>(r.shd_ref), 0, std::max(0, maxShaders - 1));

                auto ShaderName = [&](int i) -> const char*
                {
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
                if (ImGui::SmallButton(" - ")) shaderIdx = std::max(0, shaderIdx - 1);
                ImGui::SameLine();
                if (ImGui::SmallButton(" + ")) shaderIdx = std::min(std::max(0, maxShaders - 1), shaderIdx + 1);

                r.shd_ref = static_cast<float>(shaderIdx);
            }

            // --- Texture info ------------------------------------------------
            ImGui::Text("Texture ID: %u", r.textureID);

            // --- Sprite filename editing (name only) -------------------------
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

                    if (ext.empty())
                        ext = ".png";

                    bool typedHasSlash = newName.find_first_of("/\\") != std::string::npos;
                    if (typedHasSlash) r.spriteName = newName + ext;
                    else               r.spriteName = newName + ext;

                    r.needsTextureReload = true;
                }
            }

            // --- Reload texture button + drag & drop target ------------------
            {
                bool reloadClicked = ImGui::Button("Reload Texture");

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_TEXTURE"))
                    {
                        const char* droppedPath = static_cast<const char*>(payload->Data);
                        r.spriteName = droppedPath ? std::string(droppedPath) : std::string();
                        reloadClicked = true;
                    }
                    ImGui::EndDragDropTarget();
                }

                if (reloadClicked)
                {
                    namespace fs = std::filesystem;
                    const std::string kTexturesRoot = "../../MyEngine/Assets/Textures";

                    constexpr int SPRITE_SHADER_INDEX = 1;
                    constexpr int SPRITESHEET_SHADER_INDEX = 2;

                    // 1) Resolve path if user typed only a filename or invalid path
                    {
                        std::string candidate = r.spriteName;
                        std::error_code ec;

                        if ((fs::path(candidate).has_parent_path() || fs::path(candidate).is_absolute()) &&
                            fs::exists(candidate, ec))
                        {
                            // Already valid
                        }
                        else
                        {
                            std::string base = fs::path(candidate).filename().string();
                            if (base.empty()) base = candidate;

                            const fs::path roots[] = {
                                fs::path(kTexturesRoot),
                                fs::path("../../MyEngine/Assets")
                            };

                            std::string resolved;
                            for (const fs::path& root : roots)
                            {
                                fs::path rootAbs = fs::weakly_canonical(root, ec);
                                if (ec || rootAbs.empty() || !fs::exists(rootAbs, ec)) continue;

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
                                r.spriteName = resolved;
                        }
                    }

                    // 2) Load (cached by AssetsManager)
                    r.textureID = mAssets.GetOrLoadTexture(r.spriteName);
                    r.loadedSpriteName = r.spriteName;
                    r.needsTextureReload = false;

                    // 3) Choose shader based on filename hint, but DO NOT touch Animation
                    int  hintRows = 1, hintCols = 1;
                    bool isSheet = false;

                    {
                        int rr = 1, cc = 1;
                        std::string base = fs::path(r.spriteName).filename().string();
                        if (ParseGridFromFilename(base, rr, cc) && rr * cc > 1) {
                            hintRows = rr; hintCols = cc;
                            isSheet = true;
                        }
                    }

                    if (!isSheet)
                    {
                        std::string base = fs::path(r.spriteName).filename().string();
                        for (auto& c : base) c = (char)tolower(c);
                        if (base.find("sheet") != std::string::npos ||
                            base.find("atlas") != std::string::npos)
                            isSheet = true;
                    }

                    r.shd_ref = static_cast<float>(isSheet ? SPRITESHEET_SHADER_INDEX
                                                           : SPRITE_SHADER_INDEX);
                }
            }

            ImGui::Separator();
        }

        // ---------------------------------------------------------------------
        // Animation component
        // ---------------------------------------------------------------------
        if (g->HasComponent<Framework::Animation>(selectedEntity))
        {
            auto& a = g->GetComponent<Framework::Animation>(selectedEntity);

            ImGui::TextUnformatted("Animation");
            ImGui::SameLine();
            if (ImGui::Button("Remove Animation"))
                g->RemoveComponent<Framework::Animation>(selectedEntity);

            ImGui::Separator();

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

            // Current frame
            {
                const int maxFrame = std::max(0, a.totalFrames - 1);
                int cur = std::clamp(a.currentFrame, 0, maxFrame);
                if (ImGui::SliderInt("Current Frame", &cur, 0, maxFrame))
                {
                    a.currentFrame = cur;
                    a.elapsedTime = 0.0f;
                }
            }

            // Animation speed
            {
                float sp = a.animationSpeed;
                if (StepperFloat("Anim Speed (fps)", &sp, 0.0f, 120.0f, 0.1f))
                    a.animationSpeed = std::max(0.0f, sp);
            }

            ImGui::Separator();
        }

        ImGui::End();
    }

} // namespace MyEditor
