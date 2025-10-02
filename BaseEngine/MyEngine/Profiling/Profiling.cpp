// ==================================================================================
// Profiling.cpp
// Author: Chloe Lau Rey En
// Brief: Implementation of performance profiling system with high-precision timing
//        and percentage-based reporting for system performance analysis.
// ==================================================================================
#include "pch/pch.h"

ProfilingManager* ProfilingManager::instance = nullptr;

ProfilingManager* ProfilingManager::GetInstance() {
    if (!instance) {
        instance = new ProfilingManager();
    }
    return instance;
}

void ProfilingManager::StartFrame() {
    frameStartTime = std::chrono::high_resolution_clock::now();
}

void ProfilingManager::EndFrame() {
    auto frameEndTime = std::chrono::high_resolution_clock::now();
    totalFrameTime = std::chrono::duration<double>(frameEndTime - frameStartTime).count();

    if (totalFrameTime > 0.0) {
        fps = 1.0 / totalFrameTime;

        // Calculate percentages
        for (auto& [name, profile] : profiles) {
            profile.percentage = (profile.totalTime / totalFrameTime) * 100.0;
        }
    }
}

void ProfilingManager::StartSystem(const std::string& systemName) {
    startTimes[systemName] = std::chrono::high_resolution_clock::now();
}

void ProfilingManager::EndSystem(const std::string& systemName) {
    auto endTime = std::chrono::high_resolution_clock::now();
    auto it = startTimes.find(systemName);

    if (it != startTimes.end()) {
        double elapsed = std::chrono::duration<double>(endTime - it->second).count();
        profiles[systemName].totalTime += elapsed;
        profiles[systemName].callCount++;
        startTimes.erase(it);
    }
}

const std::unordered_map<std::string, ProfilingManager::SystemProfile>&
ProfilingManager::GetProfiles() const {
    return profiles;
}

void ProfilingManager::Reset() {
    for (auto& [name, profile] : profiles) {
        profile.totalTime = 0.0;
        profile.percentage = 0.0;
        profile.callCount = 0;
    }
}

void ProfilingManager::PrintStats() {
    std::cout << "\n========== PERFORMANCE PROFILE ==========\n";
    std::cout << "FPS: " << std::fixed << std::setprecision(1) << fps << " | ";
    std::cout << "Frame Time: " << std::setprecision(3)
        << totalFrameTime * 1000.0 << " ms\n";
    std::cout << "-----------------------------------------\n";

    for (const auto& [name, profile] : profiles) {
        std::cout << std::setw(20) << name << ": "
            << std::setw(6) << std::setprecision(2)
            << profile.percentage << "%  ("
            << std::setprecision(3) << profile.totalTime * 1000.0 << " ms)\n";
    }
    std::cout << "=========================================\n\n";
}

void ProfilingManager::DestroyInstance() {
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}