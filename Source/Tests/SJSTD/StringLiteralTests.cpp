// Library Headers
#include <gtest/gtest.h>

// SJ Headers

import sj.std.string_literal;

using namespace sj;

namespace container_tests
{

template <string_literal tStr>
size_t FunctionTemplateTest()
{
    return tStr.value.size();
}

TEST(StringLiteralTests, LiteralConstructionTest)
{
    string_literal test("Test");
    int expectedLength = strlen("Test") + 1;
    ASSERT_EQ(expectedLength, test.value.size());
    ASSERT_EQ(0, strcmp(test.value.data(), "Test"));
}

TEST(StringLiteralTests, TemplateParamTest)
{
    auto size = FunctionTemplateTest<"Test">();
    ASSERT_EQ(strlen("Test") + 1, size);
}

} // namespace container_tests