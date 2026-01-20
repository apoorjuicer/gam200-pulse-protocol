/******************************************************************************/
/**
* @file        Scene.h
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
	// abstract sealed make it global I think? 
	public value class Scene // abstract sealed
	{
	public:
		static void LoadScene(System::String^ Filename);
	};
}