#include "pch/pch.h"            // same PCH as engine/editor

int main()
{
    CoreEngine engine;
    Engine = &engine;           // global pointer declared in CoreEngine.h,
                                // defined once in CoreEngine.cpp

    engine.Setup();             // register ECS components + systems
    engine.Init();              // window, GL, FMOD, assets, etc.
    engine.GameLoop();          // pure game loop – no MyEditor calls
    engine.Exit();              // shutdown systems, destroy window

    Engine = nullptr;
    return 0;
}