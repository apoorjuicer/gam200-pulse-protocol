/******************************************************************************/
/**
 * @file        Hierarchy.cpp
 * @project     Pulse Protocol
 * @author      Leu Jun Yong (primary) - 60%
 * @author      Chloe Lau Rey En (secondary) - 20%
 * @author      Ban Kai Wei Benjamin (secondary) - 20%
 * @brief       Scene hierarchy panel with entity list, drag-drop reordering, and inline renaming.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

// -----------------------------------------------------------------------------
// Hierarchy.cpp  -  Scene hierarchy panel (entity list + toolbar)
// -----------------------------------------------------------------------------
#include "pch/pch_temp.h"
#include "Hierarchy.h"
#include "Editor.h"   // selectedEntity, INVALID_ENTITY, entityDisplayOrder, Game/ECS helpers
#include "TopBarPanel.h"

#include "../CoreEngine/Prefab/PrefabInstance.h"

// ImGui
#include "imgui.h"
//
#include "SnapShot.h"

// Everything else (Factory, Name, GUIButton, Renderable, Transform, Animation)
// is already included via Editor.h

namespace MyEditor
{
    
    // -------------------------------------------------------------------------
    // CONFIGURATION CONSTANTS (Change these to customize behavior)
    // -------------------------------------------------------------------------

    namespace HierarchyConfig
    {
        // Default name for new entities
        constexpr const char* DEFAULT_ENTITY_NAME = "GameObject";

        // Default transform scale for new entities
        constexpr float DEFAULT_SCALE_X = 200.0f;
        constexpr float DEFAULT_SCALE_Y = 200.0f;

        // Maximum attempts to find a unique name before giving up
        constexpr int MAX_NAME_ATTEMPTS = 10000;

        // Fallback name if all else fails
        constexpr const char* FALLBACK_ENTITY_NAME = "GameObject (New)";
    }

    // -------------------------------------------------------------------------
    // RENAME STATE (Inline renaming)
    // -------------------------------------------------------------------------

    struct RenameState
    {
        Entity renamingEntity = INVALID_ENTITY;
        char renameBuffer[256] = "";
        bool justStartedRenaming = false;
    };

    static RenameState s_renameState;

    // -------------------------------------------------------------------------
    // HELPER FUNCTIONS
    // -------------------------------------------------------------------------

    // Helper function to generate unique GameObject names
    static std::string GenerateUniqueGameObjectName()
    {
        using namespace HierarchyConfig;

        auto* g_coordinator = Coordinator::GetInstance();
        if (!g_coordinator)
            return DEFAULT_ENTITY_NAME;

        std::set<std::string> existingNames;

        // Collect all existing names
        auto allEntities = g_coordinator->GetAllEntities();
        for (auto entity : allEntities)
        {
            if (g_coordinator->HasComponent<Name>(entity))
            {
                existingNames.insert(g_coordinator->GetComponent<Name>(entity).name);
            }
        }

        // Try default name first
        if (existingNames.find(DEFAULT_ENTITY_NAME) == existingNames.end())
            return DEFAULT_ENTITY_NAME;

        // Then try numbered variants
        for (int i = 1; i < MAX_NAME_ATTEMPTS; ++i)
        {
            std::string candidate = std::string(DEFAULT_ENTITY_NAME) + " (" + std::to_string(i) + ")";
            if (existingNames.find(candidate) == existingNames.end())
                return candidate;
        }

        // Fallback if we exhausted all attempts (shouldn't happen)
        return FALLBACK_ENTITY_NAME;
    }

    // Helper function to create entity with auto-generated name
    Entity CreateEntityWithName(const std::string& baseName = "")
    {
        using namespace HierarchyConfig;

        Factory factory;
        Entity newEntity = factory.DefaultObj();

        if (newEntity == INVALID_ENTITY)
            return INVALID_ENTITY;

        auto* g_coordinator = Coordinator::GetInstance();
        if (!g_coordinator)
            return newEntity;

        // Set default transform scale
        if (g_coordinator->HasComponent<Framework::Transform>(newEntity))
        {
            auto& t = g_coordinator->GetComponent<Framework::Transform>(newEntity);
            t.Scale = { DEFAULT_SCALE_X, DEFAULT_SCALE_Y };
        }

        // Auto-generate or use provided name
        std::string entityName = baseName.empty() ? GenerateUniqueGameObjectName() : baseName;

        // Add or update Name component
        if (g_coordinator->HasComponent<Name>(newEntity))
        {
            g_coordinator->GetComponent<Name>(newEntity).name = entityName;
        }
        else
        {
            Name nameComp;
            nameComp.name = entityName;
            g_coordinator->AddComponent<Name>(newEntity, nameComp);
        }

        return newEntity;
    }

    // Helper to start renaming an entity
    static void StartRenaming(Entity entity)
    {
        auto* g_coordinator = Coordinator::GetInstance();
        if (!g_coordinator || entity == INVALID_ENTITY)
            return;

        // Auto-add Name component if missing
        if (!g_coordinator->HasComponent<Name>(entity))
        {
            Name newName;
            newName.name = "Entity " + std::to_string(static_cast<int>(entity));
            g_coordinator->AddComponent<Name>(entity, newName);
        }


        if (g_coordinator->HasComponent<Name>(entity))
        {
            s_renameState.renamingEntity = entity;
            std::string currentName = g_coordinator->GetComponent<Name>(entity).name;
            std::strncpy(s_renameState.renameBuffer, currentName.c_str(), sizeof(s_renameState.renameBuffer) - 1);
            s_renameState.renameBuffer[sizeof(s_renameState.renameBuffer) - 1] = '\0';
            s_renameState.justStartedRenaming = true;
        }
    }

    // Helper to finish renaming
    static void FinishRenaming(bool save)
    {
        if (save && s_renameState.renamingEntity != INVALID_ENTITY)
        {
            auto* g_coordinator = Coordinator::GetInstance();
            if (g_coordinator && g_coordinator->HasComponent<Name>(s_renameState.renamingEntity))
            {
                std::string newName = s_renameState.renameBuffer;
                if (!newName.empty())
                {
                    g_coordinator->GetComponent<Name>(s_renameState.renamingEntity).name = newName;
                }
            }
        }

        s_renameState.renamingEntity = INVALID_ENTITY;
        s_renameState.renameBuffer[0] = '\0';
        s_renameState.justStartedRenaming = false;
    }

    // -------------------------------------------------------------------------
    // MAIN HIERARCHY WINDOW
    // -------------------------------------------------------------------------

    void DrawHierarchyWindow(ImGuiID dockID)
    {

        if (dockID)
            ImGui::SetNextWindowDockID(dockID, ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Hierarchy"))
        {
            ImGui::End();
            return;
        }

        auto* g_coordinator = Coordinator::GetInstance();
        if (!g_coordinator)
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Coordinator not found!");
            ImGui::End();
            return;
        }

        // STICK WITH ONLY EMPTY ENTITY FROM NOW ON

        //static std::vector<EntityPrefab> entityPrefabs = {
        //    { "Empty Entity",  nullptr,                                   true, 200.0f, 200.0f },
        //    { "Hero",          "../../MyEngine/JSON/HeroPrefab.json",     false, 0.0f,  0.0f   },
        //    { "Monster",       "../../MyEngine/JSON/MonsterPrefab.json",  false, 0.0f,  0.0f   },
        //    { "Background",    nullptr,                                   true, 800.0f, 600.0f }
        //};

        //// which entity type to create (index into prefabs array)
        //static int selectedType = 0;

        //// Build display names for combo box
        //std::vector<const char*> displayNames;
        //displayNames.reserve(entityPrefabs.size());
        //for (const auto& prefab : entityPrefabs)
        //    displayNames.push_back(prefab.displayName);

        // ============================================
        // TOOLBAR - CREATE & DELETE & REORDER
        // ============================================

        // Disable all editing when playing
        bool isPlaying = IsPlaying();

        // Track scene changes during play mode
        static std::string lastSceneDuringPlay = "";

        if (isPlaying)
        {
            // Get current scene from SceneManager
            auto sceneManager = g_coordinator->GetSystem<SceneManager>();
            std::string currentScene = sceneManager ? sceneManager->GrabCurrentScene() : "";

            // If scene changed while playing, deselect
            if (!lastSceneDuringPlay.empty() && currentScene != lastSceneDuringPlay)
            {
                selectedEntity = INVALID_ENTITY;

                // Also cancel any renaming in progress
                if (s_renameState.renamingEntity != INVALID_ENTITY)
                {
                    FinishRenaming(false);
                }
            }

            lastSceneDuringPlay = currentScene;
        }
        else
        {
            // Reset tracking when not playing
            lastSceneDuringPlay = "";
        }

        if (isPlaying)
        {
            ImGui::BeginDisabled();
        }

        // [+] Create Empty Entity at end
        // Undo
        if (ImGui::Button("+"))
        {

            // Remember what was selected BEFORE creating the new object
           Entity prevSelected = selectedEntity;

            Factory factory;
            Entity newEntity = factory.DefaultObj();

            if (newEntity != INVALID_ENTITY)
            {
                // add to hierarchy list
                entityDisplayOrder.push_back(newEntity);
                //select the new entity
                selectedEntity = newEntity;
                // ------------------------------------------------------------
                // UNDO: delete the entity again and restore previous selection
                // ------------------------------------------------------------
                //c
                mUndo.Push([newEntity,prevSelected]() {
                    // Remove from display order if still present
                    //find new entity in the container entityDisplayOrder
                    Coordinator* coord = Coordinator::GetInstance();
                    if (!coord)
                        return;

                    // Remove ALL occurrences of this entity from the hierarchy list
                    entityDisplayOrder.erase(
                        std::remove(entityDisplayOrder.begin(),
                            entityDisplayOrder.end(),
                            newEntity),
                        entityDisplayOrder.end());

                    // Destroy the entity for real (no factory)
                    coord->DestroyEntity(newEntity);

                    // Restore previous selection
                    selectedEntity = prevSelected;
                    });
            }
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Create Empty GameObject");

        ImGui::SameLine();

        // Buttons that require selection
        ImGui::BeginDisabled(selectedEntity == INVALID_ENTITY);

        // [X] Delete Selected Entity
        // Undo
        if (ImGui::Button("X"))
        {
            if (selectedEntity != INVALID_ENTITY)
            {
                Entity toDelete = selectedEntity;

                EntitySnapshot snap = MakeEntitySnapshot(toDelete);
                int index = -1;

                auto it = std::find(entityDisplayOrder.begin(),
                    entityDisplayOrder.end(), 
                    selectedEntity);

                //delete the entity
                if (it != entityDisplayOrder.end())
                    entityDisplayOrder.erase(it);

                Factory factory;
                factory.DeleteEntity(selectedEntity);

                selectedEntity = INVALID_ENTITY; //clear entity

                // --- UNDO COMMAND ---
                mUndo.Push([snap, index]()
                    {
                        // Recreate the entity with all its components
                        Entity restored = CreateEntityFromEntitySnapshot(snap);
                        if (restored == INVALID_ENTITY) {
                            return;
                        }
                           
                        // Reinsert into hierarchy at original position
                        int insertIndex = index;
                        if (insertIndex < 0 || insertIndex > static_cast<int>(entityDisplayOrder.size()))
                            insertIndex = static_cast<int>(entityDisplayOrder.size());

                        entityDisplayOrder.insert(entityDisplayOrder.begin() + insertIndex, restored);

                        // Select the restored entity
                        selectedEntity = restored;
                    });
            }
        }

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Delete Selected GameObject");

        ImGui::EndDisabled();

        ImGui::Separator();

        // ============================================
        // ENTITY LIST WITH DRAG-DROP REORDERING & PREFAB CREATION
        // ============================================

        ImGui::BeginChild("HierarchyList",
            ImVec2(-FLT_MIN, -FLT_MIN),
            true,
            ImGuiWindowFlags_AlwaysUseWindowPadding);

        if (ImGui::IsWindowHovered())
            ImGui::SetWindowFocus();

        bool anySelectableClicked = false;

        // Sync display order with actual entities
        auto allEntities = g_coordinator->GetAllEntities();
        std::set<Entity> entitySet(allEntities.begin(), allEntities.end());

        // Remove deleted entities from display order
        entityDisplayOrder.erase(
            std::remove_if(entityDisplayOrder.begin(), entityDisplayOrder.end(), [&entitySet](Entity e) { return entitySet.find(e) == entitySet.end(); }), entityDisplayOrder.end()
        );

        // Add any new entities not in display order
        for (auto entity : allEntities)
        {
            if (std::find(entityDisplayOrder.begin(), entityDisplayOrder.end(), entity) == entityDisplayOrder.end())
            {
                entityDisplayOrder.push_back(entity);
            }
        }

        // Display entities in tracked order WITH DRAG-DROP
        static int draggedIndex = -1;

        for (int i = 0; i < static_cast<int>(entityDisplayOrder.size()); ++i)
        {
            Entity entity = entityDisplayOrder[i];
            // Get display name
            std::string display;
            if (g_coordinator->HasComponent<Name>(entity))
            {
                display = g_coordinator->GetComponent<Name>(entity).name;
            }
            else
            {
                // Fallback only for legacy entities without Name component
                display = "Entity " + std::to_string(static_cast<int>(entity));
            }

            // Check if prefab instance
            bool isPrefabInstance = g_coordinator->HasComponent<PrefabInstance>(entity);

            // Add prefix for prefab instances
            if (isPrefabInstance)
            {
                display = "[P] " + display;
            }

            bool isSelected = (selectedEntity == entity);
            std::string uniqueLabel = display + "##" + std::to_string(static_cast<int>(entity));

            // =================================================================
            // INLINE RENAMING
            // =================================================================

            bool isRenaming = (s_renameState.renamingEntity == entity);

            if (isRenaming)
            {
                // Show text input for renaming
                ImGui::SetNextItemWidth(-FLT_MIN);

                if (s_renameState.justStartedRenaming)
                {
                    ImGui::SetKeyboardFocusHere();
                    s_renameState.justStartedRenaming = false;
                }

                ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue |
                    ImGuiInputTextFlags_AutoSelectAll;

                if (ImGui::InputText(uniqueLabel.c_str(), s_renameState.renameBuffer,
                    sizeof(s_renameState.renameBuffer), flags))
                {
                    // Enter pressed - save and finish
                    FinishRenaming(true);
                }

                // Escape pressed - cancel
                if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                {
                    FinishRenaming(false);
                }

                // Clicked outside - save and finish
                if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0))
                {
                    FinishRenaming(true);
                }
            }
            else
            {
                // Blue color for prefab instances
                if (isPrefabInstance)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.0f));
                }

                if (ImGui::Selectable(uniqueLabel.c_str(), isSelected))
                {
                    selectedEntity = entity;
                    anySelectableClicked = true;
                }

                if (isPrefabInstance)
                {
                    ImGui::PopStyleColor();
                }

                // =================================================================
                // DROP TARGET - Must be RIGHT AFTER Selectable!
                // =================================================================

                if (ImGui::BeginDragDropTarget())
                {
                    // Visual feedback: Draw a blue line to show where the entity will be inserted
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    ImVec2 p_min = ImGui::GetItemRectMin();
                    ImVec2 p_max = ImGui::GetItemRectMax();

                    // Draw a blue horizontal line above the hovered entity
                    float line_y = p_min.y;
                    draw_list->AddLine(
                        ImVec2(p_min.x, line_y),
                        ImVec2(p_max.x, line_y),
                        IM_COL32(0, 150, 255, 255),
                        3.0f
                    );

                    // Optional: Add small arrows/indicators at the ends
                    float arrow_size = 4.0f;
                    draw_list->AddTriangleFilled(
                        ImVec2(p_min.x, line_y),
                        ImVec2(p_min.x + arrow_size, line_y - arrow_size),
                        ImVec2(p_min.x + arrow_size, line_y + arrow_size),
                        IM_COL32(0, 150, 255, 255)
                    );
                    draw_list->AddTriangleFilled(
                        ImVec2(p_max.x, line_y),
                        ImVec2(p_max.x - arrow_size, line_y - arrow_size),
                        ImVec2(p_max.x - arrow_size, line_y + arrow_size),
                        IM_COL32(0, 150, 255, 255)
                    );

                    // Accept drop with flag to disable the default yellow rectangle
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY_REORDER", ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
                    {
                        // Extract data from struct
                        struct HierarchyDragData {
                            int index;
                            Entity entity;
                        };

                        const HierarchyDragData* dragData = (const HierarchyDragData*)payload->Data;
                        int sourceIndex = dragData->index;
                        int targetIndex = i;

                        std::cout << "[Hierarchy] DROP ACCEPTED! Moving index " << sourceIndex << " to " << targetIndex << std::endl;

                        if (sourceIndex != targetIndex &&
                            sourceIndex >= 0 && sourceIndex < static_cast<int>(entityDisplayOrder.size()) &&
                            targetIndex >= 0 && targetIndex < static_cast<int>(entityDisplayOrder.size()))
                        {
                            std::cout << "[Hierarchy] Performing reorder..." << std::endl;

                            // Reorder: move source entity to target position
                            Entity movedEntity = entityDisplayOrder[sourceIndex];
                            entityDisplayOrder.erase(entityDisplayOrder.begin() + sourceIndex);

                            // Adjust target index if we removed an element before it
                            if (sourceIndex < targetIndex)
                                targetIndex--;

                            entityDisplayOrder.insert(entityDisplayOrder.begin() + targetIndex, movedEntity);

                            std::cout << "[Hierarchy] Reorder complete!" << std::endl;
                        }
                        else
                        {
                            std::cout << "[Hierarchy] Reorder BLOCKED - sourceIndex=" << sourceIndex << " targetIndex=" << targetIndex << " size=" << entityDisplayOrder.size() << std::endl;
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                else
                {
                    // BeginDragDropTarget returned false - not currently dragging anything
                }

                // =================================================================
                // DRAG SOURCE - After drop target to not interfere
                // =================================================================

                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    draggedIndex = i;

                    // Store both index and entity data in a struct
                    struct HierarchyDragData {
                        int index;
                        Entity entity;
                    };

                    static HierarchyDragData s_dragData;
                    s_dragData.index = i;
                    s_dragData.entity = entity;

                    // Set payload for hierarchy reordering
                    ImGui::SetDragDropPayload("HIERARCHY_ENTITY_REORDER", &s_dragData, sizeof(HierarchyDragData));

                    ImGui::Text("%s", display.c_str());
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Drag to reorder");
                    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Drop in Project to create Prefab");
                    ImGui::EndDragDropSource();
                }

                // =================================================================
                // DOUBLE CLICK TO RENAME - Check last to not interfere with drag/drop
                // =================================================================

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                {
                    StartRenaming(entity);
                }
            }
        }

        // Deselect if clicked on empty space
        if (ImGui::IsWindowHovered() &&
            ImGui::IsMouseClicked(0) &&
            !anySelectableClicked)
        {
            selectedEntity = INVALID_ENTITY;

            // Also cancel renaming if clicking empty space
            if (s_renameState.renamingEntity != INVALID_ENTITY)
            {
                FinishRenaming(true);
            }
        }

        ImGui::EndChild();

        if (isPlaying)
        {
            ImGui::EndDisabled();
        }

        ImGui::End();
    }


}// end of namespace
