// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Engine Headers
#include <ScrewjankShared/Math/Vec4.hpp>


using namespace sj;

namespace math_tests {

    TEST(Vec4Tests, MagnitudeTest)
    {
        Vec4 unit(0, 1, 0, 0);
        Vec4 nonUnit(1, 1, 0, 0);
        Vec4 twonit(2, 0, 0, 0);
        
        ASSERT_EQ(1, Magnitude(unit));
        ASSERT_NE(1, Magnitude(nonUnit));
        ASSERT_EQ(2, Magnitude(twonit));
    }

    TEST(Vec4Tests, CrossProductTest)
    {
        Vec4 z = Vec4::UnitX.Cross(Vec4::UnitY); 
        Vec4 y = Vec4::UnitZ.Cross(Vec4::UnitX);
        Vec4 x = Vec4::UnitY.Cross(Vec4::UnitZ);

        ASSERT_EQ(Vec4::UnitZ, z);
        ASSERT_EQ(Vec4::UnitY, y);
        ASSERT_EQ(Vec4::UnitX, x);
    }

} // namespace math_tests
