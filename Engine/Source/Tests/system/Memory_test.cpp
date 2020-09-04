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
    class DummyClass
    {
      public:
        DummyClass(int num, double num2)
        {
            m_num = num;
            m_double = num2;
        }

        int m_num;
        double m_double;
    };

    TEST(MemoryTests, GlobalNewDeleteTest)
    {
        auto dummy = new DummyClass(5, 5.0);
        ASSERT_NE(nullptr, dummy);
        ASSERT_EQ(5, dummy->m_num);
        ASSERT_EQ(5.0, dummy->m_double);

        delete dummy;
    }

    TEST(MemoryTests, AlignTest)
    {
        auto memory = malloc(sizeof(DummyClass) * 2);
        void* unaligned = (void*)((uintptr_t)memory + 1);

        auto space = (sizeof(DummyClass) * 2) - 1;
        void* std_aligned = unaligned;
        void* sj_aligned = unaligned;

        std_aligned = std::align(alignof(DummyClass), sizeof(DummyClass), std_aligned, space);
        ASSERT_NE(unaligned, std_aligned);

        sj_aligned = AlignMemory(
            alignof(DummyClass), sizeof(DummyClass), sj_aligned, (sizeof(DummyClass) * 2) - 1);
        ASSERT_NE(unaligned, sj_aligned);
        ASSERT_EQ(std_aligned, sj_aligned);
    }

} // namespace system_tests
