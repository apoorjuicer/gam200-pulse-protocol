#pragma once

struct GLFWwindow;

namespace MyEditor {
    void Init(GLFWwindow* window);  // call once after GL context is created
    void NewFrame();                // begin a new ImGui frame (per-frame)
    void Draw();                    // build simple UI (per-frame)
    void Render();                  // render ImGui draw data (per-frame, after your scene)
    void Shutdown();                // call once on exit (before destroying window/context)
}
