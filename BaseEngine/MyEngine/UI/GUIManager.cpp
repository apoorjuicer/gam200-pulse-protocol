///******************************************************************************/
///**
// * @file        GUIManager.cpp
// * @project     Pulse Protocol
// * @author      Chloe Lau Rey En
// * @brief       Simple button system - hover detection and texture swapping.
// *              Detects buttons by name suffix "_Btn" or "Btn" on UI layer.
// *
// * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
// */
// /******************************************************************************/
//
//#include "pch/pch_temp.h"
//#include "../MyEngine/UI/GUIManager.h"
//#include "../MyEngine/CoreEngine/ECS/Coordinator.h"
//#include "../MyEngine/Input/InputManager.h"
//#include "../MyEngine/Graphics/Renderable.h"
//#include "../MyEngine/Graphics/Transform.h"
//#include "../MyEngine/Components/DisplayName.h"
//#include "../MyEngine/Graphics/Layer.h"
//#include "../MyEngine/CoreEngine/Scene/SceneManager.h"
//#include "../MyEngine/CoreEngine/Core/CoreEngine.h"
//
//GUIManager::GUIManager() {}
//GUIManager::~GUIManager() { Exit(); }
//
//void GUIManager::Init() {
//    std::cout << "[GUIManager] Initialized." << std::endl;
//}
//
//void GUIManager::Update(float) {
//    updateButtons();
//}
//
//void GUIManager::Exit() {
//    std::cout << "[GUIManager] Shutdown." << std::endl;
//}
//
//void GUIManager::updateButtons() {
//    auto* coord = Coordinator::GetInstance();
//    auto inputMgr = coord->GetSystem<InputManager>();
//    if (!inputMgr) return;
//
//    // Get mouse position
//    float mouseX = inputMgr->getMousePositionVirtual().x;
//    float mouseY = inputMgr->getMousePositionVirtual().y;
//    bool mouseInViewport = inputMgr->isMouseInGameViewport();
//
//    // Find all button entities on UI layer
//    for (auto entity : coord->GetAllEntities()) {
//        // Must have: Transform, Renderable, Name, LayerTag
//        if (!coord->HasComponent<Framework::Transform>(entity)) continue;
//        if (!coord->HasComponent<Framework::Renderable>(entity)) continue;
//        if (!coord->HasComponent<Name>(entity)) continue;
//        if (!coord->HasComponent<LayerTag>(entity)) continue;
//
//        auto& layer = coord->GetComponent<LayerTag>(entity);
//        if (layer.mask != LAYER_UI) continue; // Only process UI layer
//
//        auto& name = coord->GetComponent<Name>(entity);
//
//        // Check if it's a button (name ends with "Btn")
//        if (name.name.find("Btn") == std::string::npos) continue;
//
//        auto& transform = coord->GetComponent<Framework::Transform>(entity);
//        auto& renderable = coord->GetComponent<Framework::Renderable>(entity);
//
//        if (!transform.isVisible) continue;
//
//        // Calculate button bounds
//        float halfW = transform.Scale.x * 0.5f;
//        float halfH = transform.Scale.y * 0.5f;
//        float left = transform.Pos.x - halfW;
//        float right = transform.Pos.x + halfW;
//        float bottom = transform.Pos.y - halfH;
//        float top = transform.Pos.y + halfH;
//
//        // Check hover
//        bool isHovering = (mouseX >= left && mouseX <= right &&
//            mouseY >= bottom && mouseY <= top &&
//            mouseInViewport);
//
//        // Get base texture name (remove "_hover" if present)
//        std::string baseTexture = renderable.spriteName;
//        size_t hoverPos = baseTexture.find("_hover");
//        if (hoverPos != std::string::npos) {
//            baseTexture = baseTexture.substr(0, hoverPos) + baseTexture.substr(hoverPos + 6);
//        }
//
//        // Construct hover texture name
//        size_t extPos = baseTexture.find_last_of('.');
//        std::string hoverTexture;
//        if (extPos != std::string::npos) {
//            hoverTexture = baseTexture.substr(0, extPos) + "_hover" + baseTexture.substr(extPos);
//        }
//        else {
//            hoverTexture = baseTexture + "_hover";
//        }
//
//        if (isHovering) {
//            // Swap to hover texture
//            if (renderable.spriteName != hoverTexture) {
//                renderable.spriteName = hoverTexture;
//                renderable.needsTextureReload = true;
//                std::cout << "[GUIManager] Hover: " << name.name << std::endl;
//            }
//
//            // Check for click
//            if (inputMgr->isMouseButtonTriggered(0)) {
//                std::cout << "[GUIManager] Clicked: " << name.name << std::endl;
//                handleButtonClick(entity, name.name);
//            }
//        }
//        else {
//            // Swap back to normal texture
//            if (renderable.spriteName != baseTexture) {
//                renderable.spriteName = baseTexture;
//                renderable.needsTextureReload = true;
//            }
//        }
//    }
//}
//
//void GUIManager::handleButtonClick(Entity buttonEntity, const std::string& buttonName) {
//    auto* coord = Coordinator::GetInstance();
//    auto sceneMgr = coord->GetSystem<SceneManager>();
//
//    if (buttonName == "NewGameBtn") {
//        std::cout << "[GUIManager] Starting new game..." << std::endl;
//        sceneMgr->CurrentScene = "Level1";
//    }
//    //else if (buttonName == "SettingsBtn") {
//    //    std::cout << "[GUIManager] Opening settings..." << std::endl;
//    //    sceneMgr->CurrentScene = "SettingsMenu";
//    //}
//    else if (buttonName == "QuitBtn") {
//        std::cout << "[GUIManager] Quitting game..." << std::endl;
//        if (Engine) {
//            glfwSetWindowShouldClose(Engine->GetWindow(), GLFW_TRUE);
//        }
//    }
//}