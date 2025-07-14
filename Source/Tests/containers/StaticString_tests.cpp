// Library Headers
#include <gtest/gtest.h>

// SJ Headers

import sj.std.containers.static_string;

using namespace sj;

namespace container_tests
{
    TEST(StaticStringTests, DefaultConstructionTest)
    {
        sj::static_string<256> test; // default construction
        ASSERT_EQ(0, test.size());
    }

    TEST(StaticStringTests, StringLiteralConstructionTest)
    {
        sj::static_string test("test");
        ASSERT_EQ(4, test.size());
    }

    TEST(StaticStringTests, StringViewConstructionTest)
    {
        std::string_view nullTerminated = "test";
        sj::static_string test(nullTerminated);
        ASSERT_EQ(4, test.size());

        std::string_view notNullTerminated = std::string_view("test but oh no it keeps going!").substr(0,4);
        sj::static_string test2(notNullTerminated);
        ASSERT_EQ(4, test2.size());
    }

    TEST(StaticStringTests, CStringTest)
    {
        const char* testStr = "This is a test- do not be alarmed!";
        sj::static_string testStaticStr(testStr);
        ASSERT_EQ(0, strcmp(testStr, testStaticStr.c_str()));
    }

} // namespace container_tests