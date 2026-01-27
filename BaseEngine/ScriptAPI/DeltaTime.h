/******************************************************************************/
/**
* @file        DeltaTime.h
* @project     Pulse Protocol
* @author      Chia Wei Xuan Rachael - 100%
* @brief
*
* @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/
#pragma once

namespace ScriptAPI
{
	// abstract sealed make it global	
	public ref class Time abstract sealed
	{
	public:
        static property float DeltaTime
        {
            float get();
        }

        static property int FrameCount
        {
            int get();
        }

        // Timer for how long u played the game
        static property float GameTime
        {
            float get();
        }

	};
}