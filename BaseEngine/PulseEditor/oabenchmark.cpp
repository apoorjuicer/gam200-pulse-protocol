
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
#include "oabenchmark.h"
namespace Benchmark {
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
    std::string PrintOABenchmark(const OABenchmarkResult& r)
    {
        return
            std::string("[OA Benchmark]\n") +  
            "N = " + std::to_string(r.N) + "\n" +
            "Heap make_unique: " + std::to_string(r.heap_us) + " us\n" +
            "OA  oa_make_unique: " + std::to_string(r.oa_us) + " us\n";
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
}// end of namespace



