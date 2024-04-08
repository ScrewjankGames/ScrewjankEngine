// Library Headers
#include <gtest/gtest.h>

// Screwjank Headers
#include <ScrewjankEngine/system/memory/Memory.hpp>

sj::MemSpace g_BenchmarkHeap(nullptr, sj::k1_GiB * 1, "Unit Test Heap");

int main(int argc, char** argv)
{
    // Use debug heap
    sj::MemSpaceScope testHz(&g_BenchmarkHeap);

    printf("Running main() from %s\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}