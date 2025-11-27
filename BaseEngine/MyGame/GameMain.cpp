/******************************************************************************/
/**
 * @file        GameMain.cpp
 * @project     Pulse Protocol
 * @author      Goh Pin Kai
 * @brief		
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include "pch/pch.h"            // same PCH as engine/editor

int main()
{
    CoreEngine engine;
    Engine = &engine;           // global pointer declared in CoreEngine.h,
                                // defined once in CoreEngine.cpp

    engine.Setup();             // register ECS components + systems
    engine.Init();              // window, GL, FMOD, assets, etc.

    engine.PlayGame(); // this auto-starts the game

    engine.GameLoop();          // pure game loop – no MyEditor calls
    engine.Exit();              // shutdown systems, destroy window

    Engine = nullptr;
    return 0;
}