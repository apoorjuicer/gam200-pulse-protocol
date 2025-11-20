#pragma once

#include "Editor.h"
#include "../CoreEngine/ECS/Types.h"

#include <string>
#include <vector>

namespace MyEditor
{
    // One-time setup (called from MyEditor::Init)
    void InitProjectPanel();

    // Draw the assets browser
    void ProjectPanel();



    // OS drag-and-drop from the platform layer
    void EnqueueExternalDrop(const std::vector<std::string>& paths);
    void PushOSDropPath(const char* path);
}
