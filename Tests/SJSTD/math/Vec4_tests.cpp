// STD Headers

// Library Headers
#include "gtest/gtest.h"

import sj.std.math;

using namespace sj;

namespace math_tests {

    TEST(Vec4Tests, MagnitudeTest)
    {
        vec4 unit(0, 1, 0, 0);
        vec4 nonUnit(1, 1, 0, 0);
        vec4 twonit(2, 0, 0, 0);
        
        ASSERT_EQ(1, unit.magnitude());
        ASSERT_NE(1, nonUnit.magnitude());
        ASSERT_EQ(2, twonit.magnitude());
    }

    TEST(Vec4Tests, CrossProductTest)
    {
        vec4 z = Vec4_UnitX.cross(Vec4_UnitY); 
        vec4 y = Vec4_UnitZ.cross(Vec4_UnitX);
        vec4 x = Vec4_UnitY.cross(Vec4_UnitZ);

        ASSERT_EQ(Vec4_UnitZ, z);
        ASSERT_EQ(Vec4_UnitY, y);
        ASSERT_EQ(Vec4_UnitX, x);
    }

} // namespace math_tests
