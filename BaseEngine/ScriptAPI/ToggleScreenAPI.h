#pragma once
using namespace System;

// =========================================================
// ToggleScreenAPI
// - Simple global (static) API for toggling fullscreen
// - Uses CoreEngine::ToggleFullscreen() + IsFullscreen()
// =========================================================

namespace ScriptAPI
{
    public ref class ToggleScreenAPI abstract sealed
    {
    public:
        // Toggle fullscreen <-> windowed
        static void ToggleFullscreen();

        // Convenience: ensure a specific state
        static void SetFullscreen(bool fullscreen);

        // Read current state
        static bool IsFullscreen();
    };
}
