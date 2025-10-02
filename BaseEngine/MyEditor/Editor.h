#pragma once
struct GLFWwindow;

namespace MyEditor {
    void Init(GLFWwindow* window);   // call once after GL context is created
    void NewFrame();                 // per-frame (no-op when disabled)
    void Draw();                     // per-frame (no-op when disabled)
    void Render();                   // per-frame (no-op when disabled)
    void Shutdown();                 // call once on exit

    // --- Toggle API ---
    void SetEnabled(bool enabled);
    bool IsEnabled();
}
