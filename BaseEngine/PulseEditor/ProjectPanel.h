/******************************************************************************/
/**
 * @file        ProjectPanel.h
 * @project     Pulse Protocol
 * @author      Leu Jun Yong
 * @brief       Initializes, updates, renders, and manages the ImGui
 *              editor interface with toggle support.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once

#include "Editor.h"
#include "CoreEngine/ECS/Types.h"

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

    // ?? Audio validation helpers ?????????????????????????????????
    bool IsAudioExt(const std::string& path);
    bool IsAudioFolder(const std::string& folderName);
}
