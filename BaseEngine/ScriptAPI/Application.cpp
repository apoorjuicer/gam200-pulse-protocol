/******************************************************************************/
/**
* @file        Application.cpp
* @project     Pulse Protocol
* @author      Chloe Lau Rey En
* @brief       Provides application-level functions like quitting the game.
*
* @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/
#define generic generic_workaround
#include "Application.h"
#include "CoreEngine/Core/CoreEngine.h"
#include <GLFW/glfw3.h>
#undef generic

namespace ScriptAPI
{
    void Application::Quit()
    {
        if (Engine && Engine->GetWindow())
        {
            glfwSetWindowShouldClose(Engine->GetWindow(), GLFW_TRUE);
        }
    }

    bool Application::IsPlaying()
    {
        if (Engine)
        {
            return Engine->IsPlaying();
        }
        return false;
    }
}