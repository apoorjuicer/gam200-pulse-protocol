/******************************************************************************/
/**
 * @file        Editor.h
 * @project     Pulse Protocol
 * @author      Goh Pin Kai
 * @brief       Main

 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "pch/pch.h"     // this already pulls in CoreEngine.h and Editor.h

int main()
{
    Engine = new CoreEngine();
    Engine->Setup();
    Engine->Init();                     

    GLFWwindow* win = Engine->GetWindow(); 

    MyEditor::Init(win);           


    Engine->SetEditorFrameCallback(MyEditor::Editor_FrameCallback);
    Engine->GameLoop();

    MyEditor::Shutdown();
    Engine->Exit();
    delete Engine;
    return 0;
}
