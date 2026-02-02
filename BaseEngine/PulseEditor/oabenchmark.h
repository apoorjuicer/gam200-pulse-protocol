#pragma once
#include "../../PulseEngine/CoreEngine/ECS/Types.h"
#include <string>
#include <chrono>
class ObjectAllocator; // forward declaration

// -----------------------------
// Benchmark type
// -----------------------------
namespace Benchmark {
    struct Student {
        int   Age;
        long  Year;
        float GPA;
        long  ID;

      //  static void* operator new(std::size_t size);
        //static void operator delete(void* ptr) noexcept;
    };

    struct ECSBenchmarkResult
    {
        int        N;
        long long  spawn_us;     // create entity + add components
        long long  destroy_us;   // destroy entity 
        long long  spawn_destroy_us; // spawn then destroy in one timed region
    };

    struct OABenchmarkResult
    {
        int        N;
        long long heap_us;
        long long oa_us;
    };

    // -----------------------------
    // ECS benchmark API
    // -----------------------------
    // Measures "Create GameObject" workload (entity + components).
    ECSBenchmarkResult RunECSBenchmark(int N);
    std::string PrintECSBenchmark(ECSBenchmarkResult const& r);



    // -----------------------------
    // OA benchmark API 
    // -----------------------------
    long long benchmark_heap_unique_ptr(int N);
    long long benchmark_oa_allocate_free_unique_ptr(int N, ObjectAllocator& oa);
    //// Runs the benchmark and returns results
    //// Safe to call from editor / tools / tests
    OABenchmarkResult RunOABenchmark(
        int N,
        unsigned objectsPerPage = 64,
        unsigned maxPages = 0,
        bool debugOn = false
    );
    std::string PrintOABenchmark( OABenchmarkResult const& r);
    // -----------------------------
    // timing helper, time in microsconds(us)
    // -----------------------------
    template <typename T>
    inline long long time_us(T&& fn, int trials = 7)
    {
        //init huge number
        long long best = (1LL << 62);
        for (int t = 0; t < trials; ++t)
        {
            auto start = std::chrono::steady_clock::now();
            fn();
            auto end = std::chrono::steady_clock::now();
            //convert duration to microsec
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            if (us < best) { best = us; } //keep best time
        }
        return best;
    }

   
}//end of namespace
