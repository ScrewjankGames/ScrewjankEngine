// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
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
        LinearAllocator allocator(128);

        void* dummyMemory = allocator.Allocate<LinearAllocatorDummy>();
        ASSERT_NE(nullptr, dummyMemory);

        void* alignedDummyMemory = dummyMemory;
        size_t space = sizeof(LinearAllocatorDummy) + alignof(LinearAllocatorDummy) - 1;
        std::align(
            alignof(LinearAllocatorDummy), sizeof(LinearAllocatorDummy), alignedDummyMemory, space);

        ASSERT_EQ(dummyMemory, alignedDummyMemory);
    }

    TEST(LinearAllocatorTests, ValidAllocationTest)
    {

        LinearAllocator packingTest(sizeof(SBS) * 10);

        packingTest.Allocate(sizeof(SBS) * 10);
        packingTest.Reset();

        // You should be able to fit ten individual single byte structures into a linear allocator
        // of buffer size 10
        for (int i = 0; i < 10; i++) {
            packingTest.Allocate<SBS>();
        }
    }

    TEST(LinearAllocatorTests, OutOfMemoryTest)
    {
        LinearAllocator allocator(sizeof(SBS) * 10);
        allocator.Allocate(sizeof(SBS) * 10);
        allocator.Allocate<SBS>();
        ASSERT_DEATH(allocator.Allocate<SBS>(), ".");
    }

    TEST(LinearAllocatorTests, InsufficientMemoryTest)
    {
        LinearAllocator allocator(sizeof(SBS) * 10);
        allocator.Allocate(sizeof(SBS) * 9);

        ASSERT_DEATH(allocator.Allocate(sizeof(SBS) * 2);, ".");
    }
} // namespace system_tests
