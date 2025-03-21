// Library Headers
#include <gtest/gtest.h>

// Screwjank Headers

int main(int argc, char** argv)
{
    // Use debug heap

    printf("Running main() from %s\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}