// Library Headers
#include <gtest/gtest.h>

// Screwjank Headers
#include <system/Memory.hpp>

int main(int argc, char** argv)
{
    // Use debug heap
    sj::ScopedHeapZone testHz(sj::MemorySystem::GetDebugHeapZone());

    printf("Running main() from %s\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}