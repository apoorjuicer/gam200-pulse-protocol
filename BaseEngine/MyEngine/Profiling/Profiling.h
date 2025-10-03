/******************************************************************************/
/**
 * @file        Profiling.h
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Standalone performance profiling system for measuring and
 *              displaying execution times of systems as percentages of total
 *              frame time. Provides ProfilingManager for per-frame tracking
 *              and ScopedProfiler (RAII) helper for automatic profiling of
 *              code regions.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without
 *              the prior written consent of DigiPen Institute of Technology
 *              is prohibited.
 */
 /******************************************************************************/

#pragma once
#include "pch/pch.h"

class ProfilingManager {
public:
    struct SystemProfile {
        double totalTime = 0.0;
        double percentage = 0.0;
        int callCount = 0;
    };

    static ProfilingManager* GetInstance();
    static void DestroyInstance();

    void StartFrame();
    void EndFrame();

    void StartSystem(const std::string& systemName);
    void EndSystem(const std::string& systemName);

    const std::unordered_map<std::string, SystemProfile>& GetProfiles() const;
    double GetTotalFrameTime() const { return totalFrameTime; }
    double GetFPS() const { return fps; }

    void Reset();
    void PrintStats();

private:
    ProfilingManager() = default;
    static ProfilingManager* instance;

    std::unordered_map<std::string, SystemProfile> profiles;
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> startTimes;

    std::chrono::high_resolution_clock::time_point frameStartTime;
    double totalFrameTime = 0.0;
    double fps = 0.0;
};

// RAII helper - use only in CoreEngine, NOT in individual systems
class ScopedProfiler {
public:
    ScopedProfiler(const std::string& systemName) : name(systemName) {
        ProfilingManager::GetInstance()->StartSystem(name);
    }
    ~ScopedProfiler() {
        ProfilingManager::GetInstance()->EndSystem(name);
    }
private:
    std::string name;
};

#define PROFILE_SCOPE(name) ScopedProfiler profiler_##__LINE__(name)