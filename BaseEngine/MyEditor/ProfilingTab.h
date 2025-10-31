/******************************************************************************/
/**
 * @file        ProfilingTab.h
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Simple profiling panel that displays the same info as the original
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#pragma once
#include <imgui.h>

class ProfilingPanel
{
public:
    ProfilingPanel();
    ~ProfilingPanel();

    // Render the profiling panel (matches original Editor.cpp profiler tab)
    void Draw();

private:
    // No additional state needed - just renders current profiling data
};