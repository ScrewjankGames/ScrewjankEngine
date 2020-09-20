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

        // Allocate and construct three dummies sequentially
        auto mem_loc1 = allocator.Allocate(sizeof(FreeListDummy), alignof(FreeListDummy));

        ASSERT_NE(nullptr, mem_loc1);
        ASSERT_TRUE(IsMemoryAligned(mem_loc1, alignof(FreeListDummy)));

        auto Dummy1 = new (mem_loc1) FreeListDummy {'a', 3.14};

        auto mem_loc2 = allocator.AllocateType<FreeListDummy>();
        ASSERT_NE(nullptr, mem_loc2);
        ASSERT_TRUE(IsMemoryAligned(mem_loc2, alignof(FreeListDummy)));

        auto Dummy2 = new (mem_loc2) FreeListDummy {'b', 3.14};

        auto mem_loc3 = allocator.AllocateType<FreeListDummy>();
        ASSERT_NE(nullptr, mem_loc3);
        ASSERT_TRUE(IsMemoryAligned(mem_loc3, alignof(FreeListDummy)));

        auto Dummy3 = new (mem_loc3) FreeListDummy {'c', 3.14};

        // Make sure no memory was stomped by the subsequent allocations
        ASSERT_EQ('a', Dummy1->Label);
        ASSERT_EQ('b', Dummy2->Label);
        ASSERT_EQ('c', Dummy3->Label);
        ASSERT_EQ(Dummy1->Value, Dummy2->Value);
        ASSERT_EQ(Dummy2->Value, Dummy3->Value);

        allocator.Delete<FreeListDummy>(Dummy1);

        auto mem_loc4 = allocator.AllocateType<FreeListDummy>();

        // The allocation should end up in the same place as mem_loc1 in this instance (make sure
        // space isn't being leaked to padding)
        ASSERT_EQ(mem_loc1, mem_loc4);

        auto Dummy4 = new (mem_loc4) FreeListDummy {'d', 3.14};

        // Ensure Dummy2's memory was not stomped on
        ASSERT_EQ('b', Dummy2->Label);
        ASSERT_EQ(Dummy4->Value, Dummy2->Value);

        // TODO (MrLever) Verify block coalescing
        ASSERT_TRUE(false);
    }

} // namespace system_tests
