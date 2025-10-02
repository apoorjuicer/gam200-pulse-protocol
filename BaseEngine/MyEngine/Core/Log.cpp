/*********************************************************************************************
 \file      Log.cpp
 \par		Bloop
 \author	Goh Pin Kai (pinkai.g@digipen.edu)
			Leu Jun Yong (l.junyong)

\copyright All content   2025 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/

#include "Log.h"

#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"

namespace GAM200graphics {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init()
	{ 
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("GAM200graphics");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("game");
		s_ClientLogger->set_level(spdlog::level::trace);
	}

}