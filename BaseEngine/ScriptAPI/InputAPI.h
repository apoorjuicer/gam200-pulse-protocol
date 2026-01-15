/******************************************************************************/
/**
* @file        InputAPI.h
* @project     Pulse Protocol
* @author      Chloe Lau Rey En
* @brief       Wrapper for mouse input access from C# scripts
*
* @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/
#pragma once

namespace ScriptAPI
{
	// Wrapper function for modifier and accessor for MouseInput
	public value struct InputComponent
	{
	public:
		property float MouseX
		{
			float get();
		}

		property float MouseY
		{
			float get();
		}

		property float WorldMouseX
		{
			float get();
		}

		property float WorldMouseY
		{
			float get();
		}

		property bool InGameViewport
		{
			bool get();
		}

		bool IsButtonPressed(int button);
		bool IsButtonTriggered(int button);
		bool IsButtonReleased(int button);

	internal:
		//Constructor when GetInputComponent() with Entity ID gets called
		InputComponent(unsigned int ID);
	private:
		// the ID that this instance has
		unsigned int entityID;
	};
}