// Library Headers
#include <gtest/gtest.h>

// Screwjank Headers
import sj.engine.system.memory;

int main(int argc, char** argv)
{
    printf("Running main() from %s\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}