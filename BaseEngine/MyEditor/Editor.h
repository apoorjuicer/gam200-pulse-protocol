// Editor.h
#pragma once
struct GLFWwindow;
#include <algorithm>
#include <cmath> 


namespace MyEditor {
    void Init(GLFWwindow* window);
    void NewFrame();
    void Draw();
    void Render();
    void Shutdown();
    void SetEnabled(bool enabled);
    bool IsEnabled();

    struct GameViewport { int x, y, w, h; };
    // Returns the middle "game" rectangle in FRAMEBUFFER PIXELS, computed from current panel sizes
    GameViewport ComputeGameViewportPixels(GLFWwindow* window, bool keep_16x9 = true);
}
