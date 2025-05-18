// Library Headers
#include <gtest/gtest.h>

// Screwjank Headers
import sj.engine.system.memory;

sj::MemSpace<sj::FreeListAllocator> g_BenchmarkHeap(nullptr, 1_GiB, "Unit Test Heap");

int main(int argc, char** argv)
{
    // Use debug heap
    sj::MemSpaceScope testHz(&g_BenchmarkHeap);

    printf("Running main() from %s\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}