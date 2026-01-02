// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Engine Headers

import sj.std.string_hash;

using namespace sj;

namespace string_tests
{

    TEST(StringTests, ComparisonTest)
    {
        string_hash str("TEST");
        ASSERT_EQ(str, "TEST"_strhash);
    }

} // namespace math_tests
