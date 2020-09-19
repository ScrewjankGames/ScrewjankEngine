// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include "platform/PlatformDetection.hpp"
#include "system/Memory.hpp"
#include "system/allocators/FreeListAllocator.hpp"

using namespace Screwjank;

namespace system_tests {

    struct FreeListDummy
    {
        char Label;
        double Value;
    };

    TEST(FreeListAllocatorTests, AllocationTest)
    {
        FreeListAllocator allocator(sizeof(FreeListDummy) * 16,
                                    MemorySystem::GetDefaultUnmanagedAllocator());

        auto mem_loc = allocator.Allocate(sizeof(FreeListDummy), alignof(FreeListDummy));

        ASSERT_NE(nullptr, mem_loc);
        ASSERT_TRUE(IsMemoryAligned(mem_loc, alignof(FreeListDummy)));
    }

} // namespace system_tests
