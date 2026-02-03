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

#if (_DEBUG)
#define _CRTDBG_MAP_ALLOC //to get more details
#include <cstdlib>  
#include <stdlib.h>
#include <crtdbg.h>   //for malloc and free
//#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include "pch/pch.h"     // this already pulls in CoreEngine.h and Editor.h (CHANGE THIS TO PCH_TEMP ISTFG)
#include "Editor.h"

#if defined(_DEBUG)
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

int main()
{
#if defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(125706); // to find where the leak is 
#endif
    // Memeory leak testing, comment the delete only
    //int* MemLeakTest = new int[100]; 
    // delete[] MemLeakTest; 

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
