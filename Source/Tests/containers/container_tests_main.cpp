// Library Headers
#include <gtest/gtest.h>

#include <print>

// Screwjank Headers

int main(int argc, char** argv)
{
    // Use debug heap

    std::println("Running main() from {}", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}