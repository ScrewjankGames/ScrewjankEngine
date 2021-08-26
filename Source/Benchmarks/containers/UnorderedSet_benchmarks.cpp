// STD Header
#include <functional>
#include <unordered_set>
#include <random>

// Library Headers
#include <benchmark/benchmark.h>

// Screwjank Headers
#include <ScrewjankEngine/containers/UnorderedSet.hpp>

static void BM_StdUnorderedSetInsert(benchmark::State& state)
{
    std::unordered_set<int64_t> set;
    auto count = state.range();

    std::mt19937_64 random_number_engine(0);
    std::uniform_int_distribution<int64_t> distribution;

    auto random_number_generator = std::bind(distribution, random_number_engine);

    while (state.KeepRunning()) {
        for (auto i = 0; i < state.range(); i++) {
            set.insert(random_number_generator());
        }
    }
}

static void BM_SjUnorderedSetInsert(benchmark::State& state)
{
    sj::UnorderedSet<int64_t> set(sj::MemorySystem::GetRootHeapZone());
    auto count = state.range();

    std::mt19937_64 random_number_engine(0);
    std::uniform_int_distribution<int64_t> distribution;

    auto random_number_generator = std::bind(distribution, random_number_engine);

    while (state.KeepRunning()) {
        for (auto i = 0; i < state.range(); i++) {
            set.Insert(random_number_generator());
        }
    }
}

static void BM_StdUnorderedSetFind(benchmark::State& state)
{
    std::unordered_set<int64_t> set;
    auto count = state.range();

    std::mt19937_64 random_number_engine(0);
    std::uniform_int_distribution<int64_t> distribution;

    auto random_number_generator = std::bind(distribution, random_number_engine);

    for (auto i = 0; i < state.range(); i++) {
        set.insert(random_number_generator());
    }

    while (state.KeepRunning()) {
        for (auto i = 0; i < state.range(); i++) {
            auto found = set.find(random_number_generator());
        }
    }
}

static void BM_SjUnorderedSetFind(benchmark::State& state)
{
    sj::UnorderedSet<int64_t> set(sj::MemorySystem::GetRootHeapZone());
    auto count = state.range();

    std::mt19937_64 random_number_engine(0);
    std::uniform_int_distribution<int64_t> distribution;

    auto random_number_generator = std::bind(distribution, random_number_engine);

    for (auto i = 0; i < state.range(); i++) {
        set.Insert(random_number_generator());
    }

    while (state.KeepRunning()) {
        for (auto i = 0; i < state.range(); i++) {
            auto found = set.Find(random_number_generator());
        }
    }
}

// Run the benchmark with element count up to 70312500 (approx 4.5GB)
BENCHMARK(BM_StdUnorderedSetInsert)->Range(0, 70312500);
BENCHMARK(BM_SjUnorderedSetInsert)->Range(0, 70312500);
BENCHMARK(BM_StdUnorderedSetFind)->Range(0, 70312500);
BENCHMARK(BM_SjUnorderedSetFind)->Range(0, 70312500);
BENCHMARK_MAIN();
