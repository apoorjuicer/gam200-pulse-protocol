#include "pch/pch.h"
#include <windows.h>

int main()
{
    CoreEngine engine;
    Engine = &engine;

    engine.Setup();
    engine.SetStartupFullscreen(true);
    engine.Init();
    engine.PlayGame();

    engine.GameLoop();
    engine.Exit();

    Engine = nullptr;
    return 0;
}
