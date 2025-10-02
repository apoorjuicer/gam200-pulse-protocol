/******************************************************************************/
/**
 * @file            Message.h
 * @project			Pulse Protocol
 * @author          Leu Jun Yong
 * @brief			Update the brief of this file
 * @copyright		Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#pragma once

namespace Editor
{
	enum MessageIdType
	{
		NONE,
		QUIT,
		COLLIDE,
		DEBUGTOGGLE,
		CHARAKEY,
		MOUSECLICK,
		MOVEMOVE,
		FILEDROP
	};
		
	class Message
	{
	public:
		Message(MessageIdType id) : messageId(id) {};
		MessageIdType messageId;
		virtual ~Message() {};
	};

}