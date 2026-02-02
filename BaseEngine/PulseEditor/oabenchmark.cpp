
/******************************************************************************/
/**
 * @file        oabenchmark.cpp
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin
 * @brief       This file manages
 *
 *
 * @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#pragma message("ENGINE compiling oabenchmark.cpp")
#include "pch/pch_temp.h"
#include "../../PulseEngine/ObjectAllocator/ObjectAllocator.h"
#include "../../PulseEngine/CoreEngine/ECS/Coordinator.h"
#include "../../PulseEngine/CoreEngine/Factory/Factory.h"
#include "oabenchmark.h"

#include <new>
#include <cstdlib>
#include <cstdio>
#include <windows.h>


namespace Benchmark {

    const Entity INVALID_ENTITY = static_cast<Entity>(-1);

    static Entity SpawnViaFactory()
    {
        Factory factory;
        return factory.DefaultObj();
    }

    ECSBenchmarkResult RunECSBenchmark(int N)
    {
        ECSBenchmarkResult r{};
        r.N = N;

        std::vector<Entity> created;
        created.reserve(N);

        // -----------------------
        // Spawn only
        // -----------------------
        r.spawn_us = time_us([&]() {
            created.clear();
            for (int i = 0; i < N; ++i)
            {
                Entity e = SpawnViaFactory();
                if (e != INVALID_ENTITY)
                    created.push_back(e);
            }
            });

        // -----------------------
        // Destroy only
        // -----------------------
        r.destroy_us = time_us([&]() {
            Coordinator* coord = Coordinator::GetInstance();
            for (Entity e : created)
                coord->DestroyEntity(e);
            created.clear();
            });

        // -----------------------
        // Spawn + destroy
        // -----------------------
        r.spawn_destroy_us = time_us([&]() {
            created.clear();
            Coordinator* coord = Coordinator::GetInstance();

            for (int i = 0; i < N; ++i)
            {
                Entity e = SpawnViaFactory();
                if (e != INVALID_ENTITY)
                    created.push_back(e);
            }

            for (Entity e : created)
                coord->DestroyEntity(e);
            });

        return r;
    }


    long long benchmark_heap_unique_ptr(int N)
    {
        return time_us([&]() {
            //vector of unique_ptrs
            std::vector<std::unique_ptr<Student>> u_ptrs;
            u_ptrs.reserve(N);

            for (int i = 0; i < N; ++i) {
                //Student* p = new Student();   // alloc + ctor
                std::unique_ptr<Student> p = std::make_unique<Student>(); // alloc + ctor
                p->Age = i;                   // small side-effect
                //ptrs.push_back(p);      // cannot be copied
                u_ptrs.push_back(std::move(p)); // transfer ownership
            }


            u_ptrs.clear();

            });
    }

    //need to do reserve?
    long long benchmark_oa_allocate_free_unique_ptr(int N, ObjectAllocator& oa)
    {
        return time_us([&]() { //lambda
            std::vector<std::unique_ptr<Student, OADeleter>> u_ptrs;
            u_ptrs.reserve(N);

            for (int i = 0; i < N; ++i) {
                //void* mem = oa.Allocate();        // raw block
                //Student* p = new (mem) Student(); // placement-new ctor
                std::unique_ptr<Student, OADeleter> p = oa_make_unique<Student>(oa);
                p->Age = i;
                //ptrs.push_back(p);
                u_ptrs.push_back(std::move(p));
            }

            // for (Student* p : ptrs) {
            //   p->~Student();                    // manual dtor
            //   oa.Free(p);                       // return block to OA
            // }
            // Force destruction 
            u_ptrs.clear(); // force deleter calls now
            });
    }

    OABenchmarkResult RunOABenchmark(int N,
        unsigned objectsPerPage,
        unsigned maxPages,
        bool debugOn)
    {
        // Heap From OS
        long long heap = benchmark_heap_unique_ptr(N);

        // OA config (adjust constructor params to match OAConfig)
        // (UseCPPMemManager, ObjectsPerPage, MaxPages, DebugOn, PadBytes, HeaderBlocks, Alignment)
        OAConfig cfg(
            false,                     // UseCPPMemManager_
            objectsPerPage,            // ObjectsPerPage_
            maxPages,                  // MaxPages_ (0 = unlimited)
            debugOn,                   // DebugOn_
            0,                         // PadBytes_
            0,                         // HeaderBlocks_
            alignof(std::max_align_t)  // Alignment_
        );

        ObjectAllocator oa(sizeof(Student), cfg);

        // OUR OA
        long long oa_time = benchmark_oa_allocate_free_unique_ptr(N, oa);

        return OABenchmarkResult{ N, heap, oa_time };
    }

    // convenience printer
    std::string PrintOABenchmark( OABenchmarkResult const& r)
    {
        return
            std::string("[OA Benchmark]\n") +  
            "N = " + std::to_string(r.N) + "\n" +
            "Heap make_unique: " + std::to_string(r.heap_us) + " us\n" +
            "OA  oa_make_unique: " + std::to_string(r.oa_us) + " us\n";
    }

    std::string Benchmark::PrintECSBenchmark(ECSBenchmarkResult const& r)
    {
        return
            "N = " + std::to_string(r.N) + "\n" +
            "spawn_us = " + std::to_string(r.spawn_us) + "\n" +
            "destroy_us = " + std::to_string(r.destroy_us) + "\n" +
            "spawn_destroy_us = " + std::to_string(r.spawn_destroy_us) + "\n";
    }

    /*
    * Usage
    * i f (ImGui::Button("Run OA Benchmark"))
        {
             Only N needs to be defined explicitly
            auto result = RunOABenchmark(100000);
            PrintOABenchmark(result);

            
        }
    */

    //void* Student::operator new(std::size_t size)
    //{
    //    std::cout << "[Student] new " << size << " bytes\n";
    //    return ::operator new(size);
    //}

    //void Student::operator delete(void* ptr) noexcept
    //{
    //    std::cout << "[Student] delete\n";
    //    ::operator delete(ptr);
    //}
}// end of namespace

//static thread_local bool g_inNew = false;
//
//void* operator new(std::size_t sz)
//{
//    if (g_inNew) // re-entrancy: don't log, just malloc
//        return std::malloc(sz);
//
//    g_inNew = true;
//
//    char buf[64];
//    int n = std::snprintf(buf, sizeof(buf), "[GLOBAL NEW] %zu bytes\n", sz);
//    if (n > 0) OutputDebugStringA(buf);
//
//    g_inNew = false;
//
//    if (void* p = std::malloc(sz)) return p;
//    throw std::bad_alloc{};
//}
//
//void operator delete(void* p) noexcept
//{
//    std::free(p);
//}
//
//// Also add aligned overloads (MSVC will call these sometimes)
//void* operator new(std::size_t sz, std::align_val_t al)
//{
//    return operator new(sz); // simple for logging; or use _aligned_malloc
//}
//void operator delete(void* p, std::align_val_t) noexcept
//{
//    operator delete(p);
//}


//static std::atomic<int> g_newCount{ 0 };
//static constexpr int MAX_LOGS = 10;
//
//void* operator new(std::size_t sz)
//{
//    int count = g_newCount.fetch_add(1, std::memory_order_relaxed);
//
//    if (count < MAX_LOGS)
//    {
//        char buf[64];
//        std::snprintf(buf, sizeof(buf),
//            "[GLOBAL NEW] %zu bytes (count=%d)\n", sz, count);
//        OutputDebugStringA(buf);
//    }
//
//    if (void* p = std::malloc(sz)) return p;
//    throw std::bad_alloc{};
//}
//bool g_LogNew = false;
//void* operator new(std::size_t sz)
//{
//    if (g_LogNew)
//    {
//        char buf[64];
//        std::snprintf(buf, sizeof(buf),
//            "[GLOBAL NEW] %zu bytes\n", sz);
//        OutputDebugStringA(buf);
//    }
//
//    if (void* p = std::malloc(sz)) return p;
//    throw std::bad_alloc{};
//}

//static thread_local bool g_inNew = false;
//
//void* operator new(std::size_t sz)
//{
//    if (g_inNew) {
//        if (void* p = std::malloc(sz)) return p;
//        throw std::bad_alloc{};
//    }
//
//    g_inNew = true;
//    char buf[96];
//    std::snprintf(buf, sizeof(buf), "\n>>> [GLOBAL NEW] %zu bytes <<<\n", sz);
//    OutputDebugStringA(buf);
//    g_inNew = false;
//
//    if (void* p = std::malloc(sz)) return p;
//    throw std::bad_alloc{};
//}
//
//void operator delete(void* p) noexcept
//{
//    std::free(p);
//}
//
//// REQUIRED for MSVC / STL
//void* operator new(std::size_t sz, std::align_val_t)
//{
//    return ::operator new(sz);
//}
//
//void operator delete(void* p, std::align_val_t) noexcept
//{
//    ::operator delete(p);
//}

