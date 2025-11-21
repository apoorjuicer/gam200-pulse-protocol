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
    // Create engine
    Engine = new CoreEngine();

    // Register components/systems
    Engine->Setup();

    // Init window, OpenGL, FMOD, systems, etc.
    //Engine->Init();

    //try
    //{
        Engine->Init();
    //}
    /*catch (const std::exception& e)
    {
        std::cerr << "[MAIN] Exception during Engine->Init(): " << e.what() << std::endl;
        MessageBoxA(nullptr, e.what(), "Engine Init Failed", MB_OK | MB_ICONERROR);
        return -1;
    }*/

    // Hook ImGui editor to the GLFW window
    MyEditor::Init(Engine->GetWindow());

    // Main loop
    Engine->GameLoop();

    // Shutdown editor + engine
    MyEditor::Shutdown();
    Engine->Exit();

    delete Engine;
    Engine = nullptr;

    return 0;
}