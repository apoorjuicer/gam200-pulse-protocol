/******************************************************************************/
/**
 * @file        main.cpp
 * @project     Pulse Protocol
 * @author      Goh Pin Kai
 * @brief       Main

 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
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

    GLFWwindow* win = Engine->GetWindow(); 

    PulseEditor::Init(win);           


    Engine->SetEditorFrameCallback(PulseEditor::Editor_FrameCallback);
    Engine->GameLoop();

    PulseEditor::Shutdown();
    Engine->Exit();
    delete Engine;
    return 0;
}
