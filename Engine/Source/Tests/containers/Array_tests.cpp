// STD Headers
#include <array>

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include "containers/Array.hpp"

using namespace Screwjank;

namespace container_tests {

    TEST(ArrayTest, InitializerListConstructionTest)
    {
        // Array<int, 5> test = {1, 2, 3, 4, 5};
    }

    TEST(ArrayTest, StdAlgorithmSupport)
    {
        // Array<int, 5> test = {1, 2, 3, 4, 5};
        // std::find(test.begin(), test.end(), item);
    }

    TEST(ArrayTests, AddTest)
    {
        std::array<int, 3> test;
    }

    TEST(ArrayTests, IterationTest)
    {
        ASSERT_TRUE(false);
    }

} // namespace container_tests
