// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Engine Headers
#include <ScrewjankEngine/system/memory/Allocator.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/system/memory/allocators/StackAllocator.hpp>
#include <ScrewjankEngine/system/memory/allocators/LinearAllocator.hpp>

// Shared Headers
#include <ScrewjankShared/utils/MemUtils.hpp>

using namespace sj;

namespace system_tests {
    bool dummy_dtor_called = false;
    class DummyClass
    {
      public:
        DummyClass(int num, double num2)
        {
            m_num = num;
            m_double = num2;
        }

        ~DummyClass()
        {
            dummy_dtor_called = true;
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

    TEST(MemoryTests, CustomNewDeleteTest)
    {
        auto num_ptr = New<int>(1);
        ASSERT_EQ(1, *num_ptr);
        Delete<int>(num_ptr);
    }

    TEST(MemoryTests, UnmanagedAllocatorNewDeleteTest)
    {
        auto dummy = new DummyClass(5, 5.0);
        ASSERT_NE(nullptr, dummy);
        ASSERT_EQ(5, dummy->m_num);

        delete dummy;
        ASSERT_TRUE(dummy_dtor_called);
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

        sj_aligned = AlignMemory(alignof(DummyClass),
                                 sizeof(DummyClass),
                                 sj_aligned,
                                 (sizeof(DummyClass) * 2) - 1);

        ASSERT_NE(unaligned, sj_aligned);
        ASSERT_EQ(std_aligned, sj_aligned);
    }

    TEST(MemoryTests, IsMemoryAlignedTest)
    {

        uintptr_t memory_location = 0;
        ASSERT_TRUE(IsMemoryAligned((void*)memory_location, alignof(std::max_align_t)));

        memory_location = 2;
        ASSERT_TRUE(IsMemoryAligned((void*)memory_location, alignof(int16_t)));
        ASSERT_FALSE(IsMemoryAligned((void*)memory_location, alignof(int32_t)));
        ASSERT_FALSE(IsMemoryAligned((void*)memory_location, alignof(int64_t)));

        memory_location = 4;
        ASSERT_TRUE(IsMemoryAligned((void*)memory_location, alignof(int16_t)));
        ASSERT_TRUE(IsMemoryAligned((void*)memory_location, alignof(int32_t)));
        ASSERT_FALSE(IsMemoryAligned((void*)memory_location, alignof(int64_t)));

        memory_location = 8;
        ASSERT_TRUE(IsMemoryAligned((void*)memory_location, alignof(int16_t)));
        ASSERT_TRUE(IsMemoryAligned((void*)memory_location, alignof(int32_t)));
        ASSERT_TRUE(IsMemoryAligned((void*)memory_location, alignof(int64_t)));
    }

    TEST(MemoryTests, GetAlignmentAdjustmentTest)
    {

        uintptr_t memory_location = 0;
        ASSERT_EQ(0, GetAlignmentAdjustment(alignof(double), (void*)(memory_location)));

        memory_location = 3;
        ASSERT_EQ(1, GetAlignmentAdjustment(alignof(int32_t), (void*)(memory_location)));
        ASSERT_EQ(5, GetAlignmentAdjustment(alignof(int64_t), (void*)(memory_location)));
    }

} // namespace system_tests
