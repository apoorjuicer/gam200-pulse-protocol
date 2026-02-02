/******************************************************************************/
/**
 * @file        main.cpp
 * @project     Pulse Protocol
 * @author      Goh Pin Kai
 * @brief       Main

 * @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "pch/pch.h"     // this already pulls in CoreEngine.h and Editor.h (CHANGE THIS TO PCH_TEMP ISTFG)
#include "Editor.h"

int main()
{
    Engine = new CoreEngine();
    Engine->SetStartupFullscreen(false);
    Engine->Setup();
    Engine->Init();                     

    // Hide debug display in Release mode
#ifndef _DEBUG
    auto* coord = Coordinator::GetInstance();
    auto sceneManager = coord->GetSystem<SceneManager>();
    if (coord && sceneManager) {
        const auto& entities = sceneManager->GetSceneEntities();
        for (auto entity : entities) {
            if (coord->HasComponent<Name>(entity)) {
                auto& name = coord->GetComponent<Name>(entity);
                if (name.name == "Debug Display Text" && coord->HasComponent<TextComponent>(entity)) {
                    auto& txt = coord->GetComponent<TextComponent>(entity);
                    txt.visible = false;
                }
            }
        }
    }
#endif

    GLFWwindow* win = Engine->GetWindow(); 

    PulseEditor::Init(win);           


    Engine->SetEditorFrameCallback(PulseEditor::Editor_FrameCallback);
    Engine->GameLoop();

    PulseEditor::Shutdown();
    Engine->Exit();
    delete Engine;
    return 0;
}
