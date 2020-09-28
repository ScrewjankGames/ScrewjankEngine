// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include "platform/PlatformDetection.hpp"
#include "system/Memory.hpp"
#include "system/allocators/FreeListAllocator.hpp"

using namespace sj;

namespace system_tests {

    struct FreeListDummy
    {
        char Label;
        double Value;
    };

    TEST(FreeListAllocatorTests, BasicAllocationTest)
    {
        // Test basic allocations and frees that shouldn't hit many edge cases
        FreeListAllocator allocator(sizeof(FreeListDummy) * 16,
                                    MemorySystem::GetUnmanagedAllocator());

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

        Delete(allocator, Dummy1);

        auto mem_loc4 = allocator.AllocateType<FreeListDummy>();

        // The allocation should end up in the same place as mem_loc1 in this instance (make sure
        // space isn't being leaked to padding)
        ASSERT_EQ(mem_loc1, mem_loc4);

        auto Dummy4 = new (mem_loc4) FreeListDummy {'d', 3.14};

        // Ensure Dummy2's memory was not stomped on
        ASSERT_EQ('b', Dummy2->Label);
        ASSERT_EQ(Dummy4->Value, Dummy2->Value);

        Delete(allocator, Dummy2);
        Delete(allocator, Dummy3);
        Delete(allocator, Dummy4);
    }

    TEST(FreeListAllocatorTests, AdvancedAllocationTest)
    {
        // This test attempts to test block coalescing behaviors, but is sensitive to
        // implementation details of the Allocator (such as allocation header size)

        // This allocator should have enough room for exactly two individual allocations of size
        // sizeof(FreeListDummy)
        FreeListAllocator allocator(sizeof(FreeListDummy) * 4,
                                    MemorySystem::GetUnmanagedAllocator());

        auto mem_loc1 = allocator.AllocateType<FreeListDummy>();
        auto mem_loc2 = allocator.AllocateType<FreeListDummy>();
        ASSERT_NE(nullptr, mem_loc1);
        ASSERT_NE(nullptr, mem_loc2);

        // Assert that capacity is not lost when resolving allocation headers
        allocator.Free(mem_loc1);
        mem_loc1 = allocator.AllocateType<FreeListDummy>();
        ASSERT_NE(nullptr, mem_loc1);

        // Free the first block, and try to allocate something larger (should fail)
        allocator.Free(mem_loc1);
        mem_loc1 = allocator.Allocate(sizeof(FreeListDummy) + 4, alignof(FreeListDummy));
        ASSERT_EQ(nullptr, mem_loc1);

        // Free the second block, the two blocks in the allocator should coaselce and allow for a
        // single larger allocation
        allocator.Free(mem_loc2);

        // Allocator should be able to handle an allocation larger than the original allocation.
        // This implies that the memory blocks of the free list were coalesced correctly.
        mem_loc1 = allocator.Allocate(sizeof(FreeListDummy) + 4, alignof(FreeListDummy));
        ASSERT_NE(nullptr, mem_loc1);

        // The allocator should be empty again
        allocator.Free(mem_loc1);

        // Make sure we can still make the original two allocations
        mem_loc1 = allocator.AllocateType<FreeListDummy>();
        mem_loc2 = allocator.AllocateType<FreeListDummy>();
        ASSERT_NE(nullptr, mem_loc1);
        ASSERT_NE(nullptr, mem_loc2);

        allocator.Free(mem_loc1);
        allocator.Free(mem_loc2);
    }

    TEST(FreeListAllocatorTests, MixedTypeAllocationTest)
    {
        FreeListAllocator allocator(128, MemorySystem::GetUnmanagedAllocator());

        auto mem_loc1 = allocator.AllocateType<int>();
        auto mem_loc2 = allocator.AllocateType<char>();
        auto mem_loc3 = allocator.AllocateType<double>();

        allocator.Free(mem_loc1);
        allocator.Free(mem_loc2);
        allocator.Free(mem_loc3);
    }
} // namespace system_tests
