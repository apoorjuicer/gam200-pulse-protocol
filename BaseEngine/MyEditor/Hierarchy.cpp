// -----------------------------------------------------------------------------
// Hierarchy.cpp  -  Scene hierarchy panel (entity list + toolbar)
// -----------------------------------------------------------------------------

#include "pch/pch_temp.h"

#include "Hierarchy.h"
#include "Editor.h"   // selectedEntity, INVALID_ENTITY, entityDisplayOrder, Game/ECS helpers

// ImGui
#include <imgui.h>

// Everything else (Factory, Name, GUIButton, Renderable, Transform, Animation)
// is already included via Editor.h

namespace MyEditor
{
    // -------------------------------------------------------------------------
    // Internal helpers / types (local to this file)
    // -------------------------------------------------------------------------

    struct EntityPrefab {
        const char* displayName;
        const char* jsonPath;
        bool isDefault;
        float defaultScaleX;
        float defaultScaleY;
    };



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

        // --- Entity Creation with Type Selection -----------------------------------
        static std::vector<EntityPrefab> entityPrefabs = {
            { "Empty Entity",  nullptr,                                   true, 200.0f, 200.0f },
            { "Hero",          "../../MyEngine/JSON/HeroPrefab.json",     false, 0.0f,  0.0f   },
            { "Monster",       "../../MyEngine/JSON/MonsterPrefab.json",  false, 0.0f,  0.0f   },
            { "Background",    nullptr,                                   true, 800.0f, 600.0f }
        };

        // which entity type to create (index into prefabs array)
        static int selectedType = 0;

        // Build display names for combo box
        std::vector<const char*> displayNames;
        displayNames.reserve(entityPrefabs.size());
        for (const auto& prefab : entityPrefabs)
            displayNames.push_back(prefab.displayName);

        // ============================================
        // UNITY-STYLE TOOLBAR
        // ============================================
        float windowWidth = ImGui::GetContentRegionAvail().x;
        float dropdownWidth = 120.0f;

        bool createClicked = ImGui::SmallButton("+");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Create at End");

        ImGui::SameLine();

        ImGui::BeginDisabled(selectedEntity == INVALID_ENTITY);

        bool createAboveClicked = ImGui::SmallButton("^");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Create Above Selected");

        ImGui::SameLine();
        bool createBelowClicked = ImGui::SmallButton("v");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Create Below Selected");

        ImGui::SameLine();
        bool deleteClicked = ImGui::SmallButton("X");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Delete Selected");

        ImGui::EndDisabled();

        ImGui::SameLine(windowWidth - dropdownWidth + 5.0f);
        ImGui::PushItemWidth(dropdownWidth);
        ImGui::Combo("##Type", &selectedType, displayNames.data(),
                     static_cast<int>(displayNames.size()));
        ImGui::PopItemWidth();

        ImGui::Separator();

        // ============================================
        // HANDLE BUTTON CLICKS - Create Entity
        // ============================================
        if (createClicked || createAboveClicked || createBelowClicked)
        {
            Factory defaultEntity;
            Entity newEntity = INVALID_ENTITY;

            const EntityPrefab& selectedPrefab = entityPrefabs[selectedType];

            if (selectedPrefab.isDefault)
            {
                newEntity = defaultEntity.DefaultObj();

                if (g_coordinator->HasComponent<Framework::Transform>(newEntity)) {
                    auto& t = g_coordinator->GetComponent<Framework::Transform>(newEntity);
                    t.Scale = { selectedPrefab.defaultScaleX, selectedPrefab.defaultScaleY };
                }
            }
            else if (selectedPrefab.jsonPath != nullptr)
            {
                auto result = defaultEntity.CreateEntityFromJson(selectedPrefab.jsonPath);
                if (result.has_value())
                    newEntity = result.value();
                else
                    newEntity = defaultEntity.DefaultObj();
            }
            else
            {
                newEntity = defaultEntity.DefaultObj();
            }

            if (newEntity != INVALID_ENTITY)
            {
                if (createClicked)
                {
                    // [+] add to end of list
                    entityDisplayOrder.push_back(newEntity);
                }
                else if (createAboveClicked && selectedEntity != INVALID_ENTITY)
                {
                    auto it = std::find(entityDisplayOrder.begin(),
                                        entityDisplayOrder.end(), selectedEntity);
                    if (it != entityDisplayOrder.end())
                        entityDisplayOrder.insert(it, newEntity);
                    else
                        entityDisplayOrder.push_back(newEntity);
                }
                else if (createBelowClicked && selectedEntity != INVALID_ENTITY)
                {
                    auto it = std::find(entityDisplayOrder.begin(),
                                        entityDisplayOrder.end(), selectedEntity);
                    if (it != entityDisplayOrder.end())
                        entityDisplayOrder.insert(it + 1, newEntity);
                    else
                        entityDisplayOrder.push_back(newEntity);
                }

                selectedEntity = newEntity;
            }
        }

        // ============================================
        // HANDLE BUTTON CLICKS - Delete Entity
        // ============================================
        if (deleteClicked && selectedEntity != INVALID_ENTITY)
        {
            auto it = std::find(entityDisplayOrder.begin(),
                                entityDisplayOrder.end(), selectedEntity);
            if (it != entityDisplayOrder.end())
                entityDisplayOrder.erase(it);

            Factory defaultEntity;
            defaultEntity.DeleteEntity(selectedEntity);

            selectedEntity = INVALID_ENTITY;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Scrollable list
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

        entityDisplayOrder.erase(
            std::remove_if(entityDisplayOrder.begin(), entityDisplayOrder.end(),
                           [&entitySet](Entity e) { return entitySet.find(e) == entitySet.end(); }),
            entityDisplayOrder.end()
        );

        for (auto entity : allEntities)
        {
            if (std::find(entityDisplayOrder.begin(), entityDisplayOrder.end(), entity) ==
                entityDisplayOrder.end())
            {
                entityDisplayOrder.push_back(entity);
            }
        }

        // Display in tracked order
        for (auto entity : entityDisplayOrder)
        {
            std::string display;

            if (g_coordinator->HasComponent<Name>(entity))
            {
                display = g_coordinator->GetComponent<Name>(entity).name;
            }
            else if (g_coordinator->HasComponent<GUIButton>(entity))
            {
                display = g_coordinator->GetComponent<GUIButton>(entity).buttonID;
            }
            else if (g_coordinator->HasComponent<Framework::Renderable>(entity))
            {
                auto& rend = g_coordinator->GetComponent<Framework::Renderable>(entity);
                if (!rend.spriteName.empty()) {
                    size_t lastSlash = rend.spriteName.find_last_of("/\\");
                    if (lastSlash != std::string::npos)
                        display = rend.spriteName.substr(lastSlash + 1);
                    else
                        display = rend.spriteName;

                    size_t jsonPos = display.find(".json");
                    if (jsonPos != std::string::npos)
                        display = display.substr(0, jsonPos);
                } else {
                    display = "New Entity";
                }
            }
            else
            {
                display = "Entity " + std::to_string(static_cast<int>(entity));
            }

            bool isSelected = (selectedEntity == entity);

            std::string uniqueLabel = display + "##" + std::to_string(static_cast<int>(entity));
            if (ImGui::Selectable(uniqueLabel.c_str(), isSelected))
            {
                selectedEntity = entity;
                anySelectableClicked = true;
            }
        }

        // Deselect if clicked empty space
        if (ImGui::IsWindowHovered() &&
            ImGui::IsMouseClicked(0) &&
            !anySelectableClicked)
        {
            selectedEntity = INVALID_ENTITY;
        }

        ImGui::EndChild();
        ImGui::End();


    }



}