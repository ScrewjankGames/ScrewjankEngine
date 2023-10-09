// STD Headers
#include <string>

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include <ScrewjankEngine/containers/String.hpp>

using namespace sj;

namespace container_tests
{
    TEST(StringTests, sj_strncpyTest)
    {
        char foo[4];
        sj_strncpy(foo, "foo", sizeof(foo));

        ASSERT_EQ(0, strcmp(foo, "foo"));
        
        sj_strncpy(foo, "fuck", sizeof(foo));
        ASSERT_NE(0, strcmp(foo, "fuck"));
    }
}

