// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include "system/Allocator.hpp"

using namespace Screwjank;

namespace system_tests {
    class DummyClass
    {
      public:
        DummyClass(int num)
        {
            m_num = num;
        }

        int m_num;
    };

    TEST(MemoryTests, GlobalNewDeleteTest)
    {
        auto dummy = new DummyClass(5);
        ASSERT_NE(nullptr, dummy);
        ASSERT_EQ(5, dummy->m_num);

        delete dummy;
    }
} // namespace system_tests
