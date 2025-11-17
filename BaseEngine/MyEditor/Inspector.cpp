// -----------------------------------------------------------------------------
// Inspector.cpp  -  Entity inspector (components UI, prefab save, etc.)
// -----------------------------------------------------------------------------

#include "pch/pch_temp.h"

#include "Inspector.h"
#include "Editor.h"                       // selectedEntity, INVALID_ENTITY, PrefabManager, etc.
#include "../Graphics/glapp.h"           // GLApp::shdrpgms
#include "CoreEngine/Asset/AssetsManager.h"

#include <filesystem>
#include <algorithm>
#include <cstring>

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
        ImGui::Separator();

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

            // GUIButton
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

        // ---------------------------------------------------------------------
        // GUIButton component
        // ---------------------------------------------------------------------
        if (g->HasComponent<GUIButton>(selectedEntity))
        {
            auto& b = g->GetComponent<GUIButton>(selectedEntity);

            ImGui::TextUnformatted("GUIButton");
            ImGui::Separator();

            static char idBuf[256];
            std::strncpy(idBuf, b.buttonID.c_str(), sizeof(idBuf));
            idBuf[sizeof(idBuf) - 1] = '\0';
            if (ImGui::InputText("Button ID", idBuf, IM_ARRAYSIZE(idBuf)))
                b.buttonID = idBuf;

            if (ImGui::Button("Remove GUIButton"))
                g->RemoveComponent<GUIButton>(selectedEntity);

            ImGui::Separator();
        }

        ImGui::End();
    }

} // namespace MyEditor
