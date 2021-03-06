// STD Header

// Library Headers
#include <benchmark/benchmark.h>

// Screwjank Headers
#include "system/allocators/LinearAllocator.hpp"
#include "system/allocators/StackAllocator.hpp"
#include "system/allocators/PoolAllocator.hpp"
#include "system/allocators/FreeListAllocator.hpp"

static void BM_Malloc(benchmark::State& state)
{
    // Reserve a vector to store memory addresses so things can be freed properly
    std::vector<void*> vec(state.range(1));

    while (state.KeepRunning()) {
        for (int i = 0; i < state.range(1); i++) {
            auto memory = malloc(state.range(0));

            // state.PauseTiming();
            vec[i] = memory;
            // state.ResumeTiming();
        }

        for (auto address : vec) {
            free(address);
        }
    }
}

static void BM_LinearAllocator(benchmark::State& state)
{
    // Reserve a buffer that is alloc_size * num_allocs large
    sj::LinearAllocator allocator(state.range(0) * state.range(1),
                                  sj::MemorySystem::GetUnmanagedAllocator());
    while (state.KeepRunning()) {
        for (int i = 0; i < state.range(1); i++) {
            auto memory = allocator.Allocate(state.range(0));
        }
        allocator.Reset();
    }
}

static void BM_StackAllocator(benchmark::State& state)
{
    // Reserve a buffer that is alloc_size * num_allocs large
    sj::StackAllocator allocator(state.range(0) * (size_t)std::ceil(state.range(1) * 1.5),
                                 sj::MemorySystem::GetUnmanagedAllocator());
    while (state.KeepRunning()) {
        int i = 0;
        for (; i < state.range(1); i++) {
            auto memory = allocator.Allocate(state.range(0));
        }

        for (int j = 0; j < i; j++) {
            allocator.Pop();
        }
    }
}

template <size_t kBlockSize>
void BM_PoolAllocator(benchmark::State& state)
{
    // Reserve a buffer that is alloc_size * num_allocs large
    sj::PoolAllocator<kBlockSize> allocator(state.range(1),
                                            sj::MemorySystem::GetUnmanagedAllocator());

    // Reserve a vector to store memory addresses so things can be freed properly
    std::vector<void*> vec(state.range(1));

    while (state.KeepRunning()) {
        for (int i = 0; i < state.range(1); i++) {
            vec[i] = allocator.Allocate(state.range(0));
        }

        for (auto address : vec) {
            allocator.Free(address);
        }
    }
}

static void BM_FreeListAllocator(benchmark::State& state)
{
    // Reserve a buffer that is alloc_size * num_allocs large
    sj::FreeListAllocator allocator(state.range(0) * (size_t)std::ceil(state.range(1) * 1.5),
                                    sj::MemorySystem::GetUnmanagedAllocator());

    // Reserve a vector to store memory addresses so things can be freed properly
    std::vector<void*> vec(state.range(1));

    while (state.KeepRunning()) {
        for (int i = 0; i < state.range(1); i++) {
            auto memory = allocator.Allocate(state.range(0));

            vec[i] = memory;
        }

        for (auto address : vec) {
            allocator.Free(address);
        }
    }
}

// Run the benchmark with allocation sizes from 32 bytes to 16 MB, 1 to 256 times
BENCHMARK(BM_Malloc)->Ranges({{32ull, 1ull << 24}, {1, 256}});
BENCHMARK(BM_LinearAllocator)->Ranges({{32ull, 1ull << 24}, {1, 256}});
BENCHMARK(BM_StackAllocator)->Ranges({{32ull, 1ull << 24}, {1, 256}});

// Pool allocators are templated on block size
BENCHMARK_TEMPLATE(BM_PoolAllocator, 32)->Ranges({{32ull, 32ull}, {1, 256}});
BENCHMARK_TEMPLATE(BM_PoolAllocator, 64)->Ranges({{64, 64}, {1, 256}});
BENCHMARK_TEMPLATE(BM_PoolAllocator, 512)->Ranges({{512, 512}, {1, 256}});
BENCHMARK_TEMPLATE(BM_PoolAllocator, 512)->Ranges({{4096, 4096}, {1, 256}});
BENCHMARK_TEMPLATE(BM_PoolAllocator, 32768)->Ranges({{32768, 32768}, {1, 256}});
BENCHMARK_TEMPLATE(BM_PoolAllocator, 262144)->Ranges({{262144, 262144}, {1, 256}});
BENCHMARK_TEMPLATE(BM_PoolAllocator, 2097152)->Ranges({{2097152, 2097152}, {1, 256}});
BENCHMARK_TEMPLATE(BM_PoolAllocator, 16777216)->Ranges({{16777216, 16777216}, {1, 256}});

BENCHMARK(BM_FreeListAllocator)->Ranges({{32ull, 1ull << 24}, {1, 256}});

BENCHMARK_MAIN();
