/*********************************************************************************************
 \file      Log.h
 \par		Bloop
 \author    Goh Pin Kai (pinkai.g@digipen.edu)
			Leu Jun Yong (l.junyong)
\copyright All content   2025 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/


#pragma once

#include <memory>
/*CANNOT TAKE Precompiled.h*/

namespace spdlog {
	class logger;
}

namespace GAM200graphics {

	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

// core log macros
#define GAM200_CORE_TRACE(...)   ::GAM200graphics::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define GAM200_CORE_INFO(...)    ::GAM200graphics::Log::GetCoreLogger()->info(__VA_ARGS__)
#define GAM200_CORE_WARN(...)    ::GAM200graphics::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define GAM200_CORE_ERROR(...)   ::GAM200graphics::Log::GetCoreLogger()->error(__VA_ARGS__)
#define GAM200_CORE_FATAL(...)   ::GAM200graphics::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// client log macros
#define GAM200_TRACE(...)   ::GAM200graphics::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define GAM200_INFO(...)    ::GAM200graphics::Log::GetCoreLogger()->info(__VA_ARGS__)
#define GAM200_WARN(...)    ::GAM200graphics::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define GAM200_ERROR(...)   ::GAM200graphics::Log::GetCoreLogger()->error(__VA_ARGS__)
#define GAM200_FATAL(...)   ::GAM200graphics::Log::GetCoreLogger()->fatal(__VA_ARGS__)