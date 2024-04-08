// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include <ScrewjankEngine/platform/PlatformDetection.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/system/memory/allocators/FreeListAllocator.hpp>
#include <ScrewjankEngine/system/memory/Utils.hpp>

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
        IMemSpace* heap = MemorySystem::GetRootMemSpace();
        size_t alloc_size = sizeof(FreeListDummy) * 16;
        void* test_memory = heap->Allocate(alloc_size);

        FreeListAllocator allocator;
        allocator.Init(alloc_size, test_memory);

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

        allocator.Delete(Dummy1);

        auto mem_loc4 = allocator.AllocateType<FreeListDummy>();

        // The allocation should end up in the same place as mem_loc1 in this instance (make sure
        // space isn't being leaked to padding)
        ASSERT_EQ(mem_loc1, mem_loc4);

        auto Dummy4 = new (mem_loc4) FreeListDummy {'d', 3.14};

        // Ensure Dummy2's memory was not stomped on
        ASSERT_EQ('b', Dummy2->Label);
        ASSERT_EQ(Dummy4->Value, Dummy2->Value);

        allocator.Delete(Dummy2);
        allocator.Delete(Dummy3);
        allocator.Delete(Dummy4);

        heap->Free(test_memory);
    }

    TEST(FreeListAllocatorTests, AdvancedAllocationTest)
    {
        // This test attempts to test block coalescing behaviors, but is sensitive to
        // implementation details of the Allocator (such as allocation header size)

        // This allocator should have enough room for exactly two individual allocations of size
        // sizeof(FreeListDummy)

        IMemSpace* heap = MemorySystem::Get()->GetRootMemSpace();
        size_t alloc_size = sizeof(FreeListDummy) * 4;
        void* test_memory = heap->Allocate(alloc_size);

        FreeListAllocator allocator;
        allocator.Init(alloc_size, test_memory);


        auto mem_loc1 = allocator.AllocateType<FreeListDummy>();
        auto mem_loc2 = allocator.AllocateType<FreeListDummy>();
        ASSERT_NE(nullptr, mem_loc1);
        ASSERT_NE(nullptr, mem_loc2);

        // Assert that capacity is not lost when resolving allocation headers
        allocator.Free(mem_loc1);
        mem_loc1 = allocator.AllocateType<FreeListDummy>();
        ASSERT_NE(nullptr, mem_loc1);

        // Free the first block, and try to allocate again from head
        allocator.Free(mem_loc1);
        mem_loc1 = allocator.AllocateType<FreeListDummy>();
        ASSERT_NE(nullptr, mem_loc1);
        allocator.Free(mem_loc1);

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

        heap->Free(test_memory);
    }

    TEST(FreeListAllocatorTests, MixedTypeAllocationTest)
    {
        IMemSpace* heap = MemorySystem::Get()->GetRootMemSpace();
        size_t alloc_size = 128;
        void* test_memory = heap->Allocate(alloc_size);

        FreeListAllocator allocator;
        allocator.Init(alloc_size, test_memory);

        auto mem_loc1 = allocator.AllocateType<int>();
        auto mem_loc2 = allocator.AllocateType<char>();
        auto mem_loc3 = allocator.AllocateType<double>();

        allocator.Free(mem_loc1);
        allocator.Free(mem_loc2);
        allocator.Free(mem_loc3);
    }
} // namespace system_tests
