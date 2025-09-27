/*********************************************************************************************
 \file      FontObj.h
 \par		Bloop
 \author	Goh Pin Kai (pinkai.g@digipen.edu)
			Leu Jun Yong (l.junyong)

\copyright All content   2025 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/
#pragma once
#include "pch/pch.h"

namespace Framework
{
	class Font {
		std::string FontID;
		std::string FontPath;
	
		virtual void Initialize() {
		}

		void SetFont(std::string t) {
			FontID = t;
		}
		void SetFontPath(std::string t) {
			FontPath = t;
		}
		std::string GetFontID() const
		{
			return FontID;
		}
		std::string GetFontPath() const
		{
			return FontPath;
		}

		//void Serialize(Value& data, Document::AllocatorType& alloc) override
		//{
		//	data.SetObject();
		//	data.AddMember("FontID", StringRef(FontID.c_str()), alloc);
		//}

		//void Deserialize(Value& data)
		//{
		//	SetFont(data["FontID"].GetString());
		//}
	};
}