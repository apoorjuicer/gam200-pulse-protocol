#pragma once

namespace MyEditor
{
    // Forward declaration; full definition is in Editor.h
    struct GameViewport;

    // Main gizmo function
    void Gizmo_DrawAndHandle(const GameViewport& vp);
}
