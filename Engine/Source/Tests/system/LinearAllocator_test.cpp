// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include "platform/PlatformDetection.hpp"
#include "system/Allocator.hpp"
#include "system/Memory.hpp"
#include "system/allocators/StackAllocator.hpp"
#include "system/allocators/LinearAllocator.hpp"

using namespace Screwjank;

namespace system_tests {
    class LinearAllocatorDummy
    {
      public:
        LinearAllocatorDummy(int num, double num2)
        {
            m_num = num;
            m_double = num2;
        }

        int m_num;
        double m_double;
    };

    struct SBS // Single Byte Struct
    {
        char byte;
    };

    TEST(LinearAllocatorTests, MemoryAlignmentTest)
    {
        // Create a stack allocator with a 128 byte buffer
        LinearAllocator allocator(128, MemorySystem::GetUnmanagedAllocator());

        void* dummyMemory = allocator.AllocateType<LinearAllocatorDummy>();
        ASSERT_NE(nullptr, dummyMemory);

        void* alignedDummyMemory = dummyMemory;
        size_t space = sizeof(LinearAllocatorDummy) + alignof(LinearAllocatorDummy) - 1;
        std::align(alignof(LinearAllocatorDummy),
                   sizeof(LinearAllocatorDummy),
                   alignedDummyMemory,
                   space);

        ASSERT_EQ(dummyMemory, alignedDummyMemory);

        allocator.Reset();
    }

    TEST(LinearAllocatorTests, ValidAllocationTest)
    {

        LinearAllocator packingTest(sizeof(SBS) * 10, MemorySystem::GetUnmanagedAllocator());

        auto mem = packingTest.Allocate(sizeof(SBS) * 10);
        packingTest.Reset();

        // You should be able to fit ten individual single byte structures into a linear allocator
        // of buffer size 10
        for (int i = 0; i < 10; i++) {
            mem = packingTest.AllocateType<SBS>();
        }
    }

    TEST(LinearAllocatorTests, OutOfMemoryTest)
    {
        LinearAllocator allocator(sizeof(SBS) * 10, MemorySystem::GetUnmanagedAllocator());
        auto mem = allocator.Allocate(sizeof(SBS) * 10);
        ASSERT_EQ(nullptr, allocator.AllocateType<SBS>());
    }

    TEST(LinearAllocatorTests, InsufficientMemoryTest)
    {
        LinearAllocator allocator(sizeof(SBS) * 10, MemorySystem::GetUnmanagedAllocator());
        auto mem = allocator.Allocate(sizeof(SBS) * 9);
        ASSERT_EQ(nullptr, allocator.Allocate(sizeof(SBS) * 2));
    }

    TEST(LinearAllocatorTests, MemoryStompTest)
    {
        // Ensure allocations don't stomp each other's memory
        // Reserve 256 bytes
        LinearAllocator allocator(256, MemorySystem::GetUnmanagedAllocator());

        LinearAllocatorDummy* dummy1 = New<LinearAllocatorDummy>(allocator, 1, 1.0);
        auto dummy2 = New<LinearAllocatorDummy>(allocator, 2, 2.0);
        auto dummy3 = New<LinearAllocatorDummy>(allocator, 3, 3.0);

        ASSERT_EQ(1, dummy1->m_num);
        ASSERT_EQ(1.0, dummy1->m_double);
        ASSERT_EQ(2, dummy2->m_num);
        ASSERT_EQ(2.0, dummy2->m_double);
        ASSERT_EQ(3, dummy3->m_num);
        ASSERT_EQ(3.0, dummy3->m_double);
    }
} // namespace system_tests
