// Library Headers
#include <gtest/gtest.h>

// STD Includes
#include <print>

int main(int argc, char** argv)
{
    std::println("Running main() from {}\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}