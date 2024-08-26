// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Engine Headers
#include <ScrewjankShared/String/StringHash.hpp>

using namespace sj;

namespace string_tests
{

    TEST(StringTests, ComparisonTest)
    {
        StringHash str("TEST");
        ASSERT_EQ(str, "TEST"_strhash);
    }

} // namespace math_tests
