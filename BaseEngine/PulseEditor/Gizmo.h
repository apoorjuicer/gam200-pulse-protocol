/******************************************************************************/
/**
 * @file        Gizmo.h
 * @project     Pulse Protocol
 * @author      Leu Jun Yong
 * @brief       Provides gizmo drawing and interaction handling for the editor viewport
 * 
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#pragma once

namespace MyEditor
{
    // Forward declaration; full definition is in Editor.h
    struct GameViewport;

    // Main gizmo function
    void Gizmo_DrawAndHandle(const GameViewport& vp);
}
