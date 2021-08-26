// STD Headers


// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include <containers/Optional.hpp>

using namespace sj;

namespace container_tests
{
    template <class T>
    struct SizeTestStruct
    {
        T a;
        bool b;
    };

    TEST(OptionalTests, SizeTest)
    {
        // The maximum size of an optional should always be:
        // Sizeof(T) + padding + Sizeof(bool)

        ASSERT_EQ(sizeof(SizeTestStruct<double>), sizeof(Optional<double>));
    }

    TEST(OptionalTests, ConstructionTest)
    {
        // Default construction
        Optional<double> a;
        ASSERT_FALSE(a.HasValue());

        // NullOpt construction
        Optional<double> b(NullOpt); 
        ASSERT_FALSE(a.HasValue());


    }

    TEST(OptionalTests, AssignmentOperatorTest)
    {
        // Value assignment
        Optional<double> a;
        a = 5;
        ASSERT_TRUE(a.HasValue());
        ASSERT_EQ(5, a.Value());

        // Copy assignment
        Optional<double> b = a;
        ASSERT_TRUE(b.HasValue());
        ASSERT_EQ(a.Value(), b.Value());

        // Move assignment
        Optional<double> c;
        c = 3.14;

        Optional<double> d = std::move(c);
        ASSERT_TRUE(d.HasValue());
        ASSERT_EQ(3.14, d.Value());

        // Nullopt assignment
        d = NullOpt;
        ASSERT_FALSE(d.HasValue());
    }

    TEST(OptionalTests, ReferenceAndDereferenceOperators)
    {
        struct test_struct
        {
            int a;
            int b;
        };

        // Test arrow operator
        Optional<test_struct> test = test_struct{5, 7};
        ASSERT_TRUE(test.HasValue());
        ASSERT_EQ(5, test->a);

        // Dereference test
        Optional<int> test2 = 5;
        ASSERT_EQ(5, *test2);

        // Like a std::optional, you can "take" the contained value by calling operator* 
        // on a rvalue to optional

        Optional<std::string> test3 = std::string("abc");
        auto stolen_value = *std::move(test3);
        ASSERT_TRUE(test3.HasValue());
        ASSERT_EQ(3, stolen_value.length());
        ASSERT_EQ(0, test3->length());
    }

    TEST(OptionalTests, ComparisonOperatorsTest)
    {
        // Optional Equality
        Optional<int> a = 5;
        Optional<int> b;

        ASSERT_FALSE(a == b);
        ASSERT_TRUE(a != b);

        // Nullopt equality
        ASSERT_TRUE(b == NullOpt);
        ASSERT_TRUE(NullOpt == b);
        ASSERT_FALSE(b != NullOpt);
        ASSERT_FALSE(NullOpt != b);

        ASSERT_FALSE(a == NullOpt);
        ASSERT_FALSE(NullOpt == a);
        ASSERT_TRUE(a != NullOpt);
        ASSERT_TRUE(NullOpt != a);

        // Value equality
        ASSERT_TRUE(a == 5);
        ASSERT_TRUE(5 == a);
        ASSERT_FALSE(a != 5);
        ASSERT_FALSE(5 != a);

        ASSERT_FALSE(b == 5);
        ASSERT_FALSE(5 == b);
        ASSERT_TRUE(b != 5);
        ASSERT_TRUE(5 != b);

        // Test relational operators
        b = 6;
        auto c = 6;

        ASSERT_EQ(5 <=> 6, a <=> b);
        ASSERT_EQ(6 <=> 6, c <=> b);
        ASSERT_TRUE(a < b);
        ASSERT_TRUE(a < c);
        ASSERT_FALSE(a > b);
        ASSERT_FALSE(b < a);
        ASSERT_TRUE(b > a);

        ASSERT_TRUE(b <= c);
        ASSERT_TRUE(b >= c);
    }

    TEST(OptionalTests, BoolConverstionTest)
    {
        Optional<int> a;

        ASSERT_FALSE(a);
        a = 5;
        ASSERT_TRUE(a);
    }

    TEST(OptionalTests, ValueOrTest)
    {
        Optional<double> a;
        ASSERT_EQ(1, a.ValueOr(1));
        
        a = 5;

        ASSERT_EQ(5, a.ValueOr(1000));
    }

    TEST(OptionalTests, SwapTest)
    {
        Optional<double> a;
        Optional<double> b;

        a.Swap(b);
        ASSERT_FALSE(a.HasValue());
        ASSERT_FALSE(b.HasValue());

        a = 5;
        a.Swap(b);
        ASSERT_FALSE(a.HasValue());
        ASSERT_TRUE(b.HasValue());
        ASSERT_EQ(5, b.Value());

        a.Swap(b);
        ASSERT_TRUE(a.HasValue());
        ASSERT_FALSE(b.HasValue());
        ASSERT_EQ(5, a.Value());

        b = 1000;
        a.Swap(b);
        ASSERT_TRUE(a.HasValue());
        ASSERT_TRUE(b.HasValue());
        ASSERT_EQ(1000, a.Value());
        ASSERT_EQ(5, b.Value());
    }


} // namespace container_tests
