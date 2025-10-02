#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <cstdlib>  // for std::abort

namespace GAM200graphics {
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

    void Log::Init()
    {
        try {
            spdlog::set_pattern("%^[%T] %n: %v%$");

            // Core logger (engine logs)
            s_CoreLogger = spdlog::stdout_color_mt("GAM200graphics");
            s_CoreLogger->set_level(spdlog::level::trace);

            // Client logger (game logs)
            s_ClientLogger = spdlog::stdout_color_mt("Game");
            s_ClientLogger->set_level(spdlog::level::trace);

            // File sink for crash/error logs
            auto file_logger = spdlog::basic_logger_mt("FileLogger", "CrashLog.txt", true);
            file_logger->set_level(spdlog::level::trace);
            file_logger->flush_on(spdlog::level::err); // auto-flush on errors
        }
        catch (const spdlog::spdlog_ex& ex) {
            printf("Log initialization failed: %s\n", ex.what());
        }
    }

    void Log::CriticalAndAbort(const std::string& message)
    {
        // Log to console
        if (s_CoreLogger) s_CoreLogger->critical(message);
        if (s_ClientLogger) s_ClientLogger->critical(message);

        // Log to file
        auto file_logger = spdlog::get("FileLogger");
        if (file_logger) {
            file_logger->critical("CRASH LOG: {}", message);
            file_logger->flush(); // ensure it writes immediately
        }

        // Immediately terminate program
        std::abort();
    }

}