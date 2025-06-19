// STD Headers

// Library Headers
#include "gtest/gtest.h"

import sj.std.math;

using namespace sj;

namespace math_tests {

    TEST(Vec4Tests, MagnitudeTest)
    {
        Vec4 unit(0, 1, 0, 0);
        Vec4 nonUnit(1, 1, 0, 0);
        Vec4 twonit(2, 0, 0, 0);
        
        ASSERT_EQ(1, unit.Magnitude());
        ASSERT_NE(1, nonUnit.Magnitude());
        ASSERT_EQ(2, twonit.Magnitude());
    }

    TEST(Vec4Tests, CrossProductTest)
    {
        Vec4 z = Vec4_UnitX.Cross(Vec4_UnitY); 
        Vec4 y = Vec4_UnitZ.Cross(Vec4_UnitX);
        Vec4 x = Vec4_UnitY.Cross(Vec4_UnitZ);

        ASSERT_EQ(Vec4_UnitZ, z);
        ASSERT_EQ(Vec4_UnitY, y);
        ASSERT_EQ(Vec4_UnitX, x);
    }

} // namespace math_tests
