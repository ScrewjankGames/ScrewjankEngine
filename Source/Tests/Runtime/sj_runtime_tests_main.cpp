// Library Headers
#include <gtest/gtest.h>
#include "ScrewjankStd/Log.hpp"

import sj.std;
import sj.engine;

int main(int argc, char** argv)
{
    sj::Engine engine(1_MiB);

    SJ_ENGINE_LOG_INFO("Running main() from {}\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}