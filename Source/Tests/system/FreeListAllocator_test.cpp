// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Shared Headers
#include <ScrewjankShared/utils/PlatformDetection.hpp>
#include <memory_resource>

import sj.engine.system.memory;

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
        std::pmr::memory_resource* heap = MemorySystem::GetUnmanagedMemoryResource();
        size_t alloc_size = sizeof(FreeListDummy) * 16;
        void* test_memory = heap->allocate(alloc_size);

        FreeListAllocator resource;
        resource.init(alloc_size, test_memory);
        std::pmr::polymorphic_allocator<FreeListDummy> allocator(&resource);

        // Allocate and construct three dummies sequentially
        auto mem_loc1 = allocator.allocate(1);

        ASSERT_NE(nullptr, mem_loc1);
        ASSERT_TRUE(IsMemoryAligned(mem_loc1, alignof(FreeListDummy)));

        auto Dummy1 = new (mem_loc1) FreeListDummy {.Label='a', .Value=3.14};

        FreeListDummy* Dummy2 = allocator.new_object<FreeListDummy>('b', 3.14);
        ASSERT_NE(nullptr, Dummy2);
        ASSERT_TRUE(IsMemoryAligned(Dummy2, alignof(FreeListDummy)));

        FreeListDummy* Dummy3 = allocator.new_object<FreeListDummy>('c', 3.14);
        ASSERT_NE(nullptr, Dummy3);
        ASSERT_TRUE(IsMemoryAligned(Dummy3, alignof(FreeListDummy)));

        // Make sure no memory was stomped by the subsequent allocations
        ASSERT_EQ('a', Dummy1->Label);
        ASSERT_EQ('b', Dummy2->Label);
        ASSERT_EQ('c', Dummy3->Label);
        ASSERT_EQ(Dummy1->Value, Dummy2->Value);
        ASSERT_EQ(Dummy2->Value, Dummy3->Value);

        allocator.deallocate(Dummy1, sizeof(FreeListDummy));

        auto mem_loc4 = allocator.allocate(1);

        // The allocation should end up in the same place as mem_loc1 in this instance (make sure
        // space isn't being leaked to padding)
        ASSERT_EQ(mem_loc1, mem_loc4);

        auto Dummy4 = new (mem_loc4) FreeListDummy {.Label='d', .Value=3.14};

        // Ensure Dummy2's memory was not stomped on
        ASSERT_EQ('b', Dummy2->Label);
        ASSERT_EQ(Dummy4->Value, Dummy2->Value);

        allocator.deallocate(Dummy2, sizeof(FreeListDummy));
        allocator.deallocate(Dummy3, sizeof(FreeListDummy));
        allocator.deallocate(Dummy4, sizeof(FreeListDummy));

        heap->deallocate(test_memory, alloc_size);
    }

    TEST(FreeListAllocatorTests, AdvancedAllocationTest)
    {
        // This test attempts to test block coalescing behaviors, but is sensitive to
        // implementation details of the Allocator (such as allocation header size)

        // This allocator should have enough room for exactly two individual allocations of size
        // sizeof(FreeListDummy)

        std::pmr::memory_resource* heap = MemorySystem::Get()->GetUnmanagedMemoryResource();
        size_t alloc_size = sizeof(FreeListDummy) * 4;
        void* test_memory = heap->allocate(alloc_size);

        FreeListAllocator resource;
        resource.init(alloc_size, test_memory);
        std::pmr::polymorphic_allocator<FreeListDummy> allocator(&resource);

        auto mem_loc1 = allocator.allocate(1);
        auto mem_loc2 = allocator.allocate(1);
        ASSERT_NE(nullptr, mem_loc1);
        ASSERT_NE(nullptr, mem_loc2);

        // Assert that capacity is not lost when resolving allocation headers
        allocator.deallocate(mem_loc1, sizeof(FreeListDummy));
        mem_loc1 = allocator.allocate(1);
        ASSERT_NE(nullptr, mem_loc1);

        // Free the first block, and try to allocate again from head
        allocator.deallocate(mem_loc1, sizeof(FreeListDummy));
        mem_loc1 = allocator.allocate(1);
        ASSERT_NE(nullptr, mem_loc1);
        allocator.deallocate(mem_loc1, sizeof(FreeListDummy));

        // Free the second block, the two blocks in the allocator should coaselce and allow for a
        // single larger allocation
        allocator.deallocate(mem_loc2, sizeof(FreeListDummy));

        // Allocator should be able to handle an allocation larger than the original allocation.
        // This implies that the memory blocks of the free list were coalesced correctly.
        mem_loc1 = allocator.allocate_bytes(sizeof(FreeListDummy) + 4, alignof(FreeListDummy));
        ASSERT_NE(nullptr, mem_loc1);

        // The allocator should be empty again
        allocator.deallocate(mem_loc1, sizeof(FreeListDummy) + 4);

        // Make sure we can still make the original two allocations
        mem_loc1 = allocator.allocate(1);
        mem_loc2 = allocator.allocate(1);
        ASSERT_NE(nullptr, mem_loc1);
        ASSERT_NE(nullptr, mem_loc2);

        allocator.deallocate(mem_loc1, sizeof(FreeListDummy));
        allocator.deallocate(mem_loc2, sizeof(FreeListDummy));

        heap->deallocate(test_memory, alloc_size);
    }

    TEST(FreeListAllocatorTests, MixedTypeAllocationTest)
    {
        std::pmr::memory_resource* heap = MemorySystem::Get()->GetUnmanagedMemoryResource();
        size_t alloc_size = 128;
        void* test_memory = heap->allocate(alloc_size);

        FreeListAllocator resource;
        resource.init(alloc_size, test_memory);
        std::pmr::polymorphic_allocator allocator(&resource);

        auto mem_loc1 = allocator.allocate_bytes(sizeof(int), alignof(int));
        auto mem_loc2 = allocator.allocate_bytes(sizeof(char), alignof(char));
        auto mem_loc3 = allocator.allocate_bytes(sizeof(double), alignof(double));

        allocator.deallocate_bytes(mem_loc1, sizeof(int));
        allocator.deallocate_bytes(mem_loc2, sizeof(char));
        allocator.deallocate_bytes(mem_loc3, sizeof(double));

        heap->deallocate(test_memory, alloc_size);
    }
} // namespace system_tests
