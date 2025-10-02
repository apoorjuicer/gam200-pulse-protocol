/*********************************************************************************************
 \file      Message.h
 \par		Cat Feeders
 \author	Michelle Han Jian Ping (m.han@digipen.edu) 100%


\copyright All content   2024 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/
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