/******************************************************************************/
/**
 * @file        LayerManagerPanel.cpp
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Layer management window implementation
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "pch/pch_temp.h"
#include "LayerManagerPanel.h"
#include "Editor.h"

#include "Graphics/Layer.h"
#include "CoreEngine/ECS/Coordinator.h"
#include "Graphics/GraphicsManager.h"  // For SetEditorLayerFilter

namespace PulseEditor
{
    // -------------------------------------------------------------------------
    // Layer visibility state (persists across frames)
    // -------------------------------------------------------------------------
    static bool s_layerVisible[3] = { true, true, true };  // Background, World, UI
    static bool s_layerLocked[3] = { false, false, false }; // Lock layers from selection

    // Helper to convert LayerMask to index
    static int LayerMaskToIndex(unsigned int mask)
    {
        if (mask == LAYER_BACKGROUND) return 0;
        if (mask == LAYER_WORLD) return 1;
        if (mask == LAYER_UI) return 2;
        return 1; // default to World
    }

    // Helper to compute combined visible layer mask
    static unsigned int ComputeVisibleLayerMask()
    {
        unsigned int mask = 0;
        if (s_layerVisible[0]) mask |= LAYER_BACKGROUND;
        if (s_layerVisible[1]) mask |= LAYER_WORLD;
        if (s_layerVisible[2]) mask |= LAYER_UI;
        return mask;
    }

    // Update the graphics manager filter whenever visibility changes
    static void UpdateGraphicsFilter()
    {
        auto* coord = Coordinator::GetInstance();
        if (!coord) return;

        auto gfxMgr = coord->GetSystem<GraphicsManager>();
        if (!gfxMgr) return;

        unsigned int visibleMask = ComputeVisibleLayerMask();
        gfxMgr->SetEditorLayerFilter(visibleMask);
    }

    // -------------------------------------------------------------------------
    // Public API
    // -------------------------------------------------------------------------
    bool IsLayerVisible(unsigned int layer)
    {
        return s_layerVisible[LayerMaskToIndex(layer)];
    }

    bool IsLayerLocked(unsigned int layer)
    {
        return s_layerLocked[LayerMaskToIndex(layer)];
    }

    // Set visibility for a specific layer (for external controls like TopBar)
    void SetLayerVisibility(unsigned int layer, bool visible)
    {
        int index = LayerMaskToIndex(layer);
        if (s_layerVisible[index] != visible) {
            s_layerVisible[index] = visible;
            UpdateGraphicsFilter();
        }
    }

    // Set visibility for all layers at once (for quick filters)
    void SetAllLayersVisibility(bool bg, bool world, bool ui)
    {
        bool changed = false;
        if (s_layerVisible[0] != bg) { s_layerVisible[0] = bg; changed = true; }
        if (s_layerVisible[1] != world) { s_layerVisible[1] = world; changed = true; }
        if (s_layerVisible[2] != ui) { s_layerVisible[2] = ui; changed = true; }

        if (changed) {
            UpdateGraphicsFilter();
        }
    }

    // -------------------------------------------------------------------------
    // Main Layer Manager Window
    // -------------------------------------------------------------------------
    void DrawLayerManagerWindow()
    {
        ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Layer Manager", nullptr))
        {
            ImGui::End();
            return;
        }

        bool isPlaying = IsPlaying();
        if (isPlaying)
        {
            ImGui::BeginDisabled();
        }

        ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Scene Layers");
        ImGui::Separator();
        ImGui::Spacing();

        auto* coord = Coordinator::GetInstance();
        if (!coord)
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Coordinator not found!");
            ImGui::End();
            return;
        }

        // Count objects per layer
        int layerCounts[3] = { 0, 0, 0 }; // Background, World, UI

        for (Entity e : coord->GetAllEntities())
        {
            if (coord->HasComponent<LayerTag>(e))
            {
                auto& tag = coord->GetComponent<LayerTag>(e);
                layerCounts[LayerMaskToIndex(tag.mask)]++;
            }
        }

        // =====================================================================
        // Background Layer
        // =====================================================================
        ImGui::PushID("Background");

        // Visibility toggle
        if (ImGui::Checkbox("Visible##BG", &s_layerVisible[0]))
        {
            UpdateGraphicsFilter();  // Update renderer when visibility changes
        }

        ImGui::SameLine();

        // Lock toggle
        if (ImGui::Checkbox("Locked##BG", &s_layerLocked[0]))
        {
            // Layer lock changed
        }

        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 0.6f, 1.0f), "Background");
        ImGui::SameLine();
        ImGui::TextDisabled("(%d objects)", layerCounts[0]);

        if (!s_layerVisible[0])
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "[HIDDEN]");
        }

        ImGui::PopID();
        ImGui::Spacing();

        // =====================================================================
        // World Layer
        // =====================================================================
        ImGui::PushID("World");

        if (ImGui::Checkbox("Visible##World", &s_layerVisible[1]))
        {
            UpdateGraphicsFilter();  // Update renderer when visibility changes
        }

        ImGui::SameLine();

        if (ImGui::Checkbox("Locked##World", &s_layerLocked[1]))
        {
            // Layer lock changed
        }

        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "World");
        ImGui::SameLine();
        ImGui::TextDisabled("(%d objects)", layerCounts[1]);

        if (!s_layerVisible[1])
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "[HIDDEN]");
        }

        ImGui::PopID();
        ImGui::Spacing();

        // =====================================================================
        // UI Layer
        // =====================================================================
        ImGui::PushID("UI");

        if (ImGui::Checkbox("Visible##UI", &s_layerVisible[2]))
        {
            UpdateGraphicsFilter();  // Update renderer when visibility changes
        }

        ImGui::SameLine();

        if (ImGui::Checkbox("Locked##UI", &s_layerLocked[2]))
        {
            // Layer lock changed
        }

        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.6f, 1.0f), "UI");
        ImGui::SameLine();
        ImGui::TextDisabled("(%d objects)", layerCounts[2]);

        if (!s_layerVisible[2])
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "[HIDDEN]");
        }

        ImGui::PopID();

        // =====================================================================
        // Summary Section
        // =====================================================================
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Total Objects: %d", layerCounts[0] + layerCounts[1] + layerCounts[2]);

        ImGui::Spacing();
        ImGui::TextDisabled("Toggle visibility and lock layers");

        // =====================================================================
        // Quick Actions
        // =====================================================================
        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::Button("Show All Layers"))
        {
            s_layerVisible[0] = s_layerVisible[1] = s_layerVisible[2] = true;
            UpdateGraphicsFilter();  // Update renderer
        }

        ImGui::SameLine();

        if (ImGui::Button("Hide All Layers"))
        {
            s_layerVisible[0] = s_layerVisible[1] = s_layerVisible[2] = false;
            UpdateGraphicsFilter();  // Update renderer
        }

        if (isPlaying)
        {
            ImGui::EndDisabled();
        }

        ImGui::End();
    }

} // namespace PulseEditor