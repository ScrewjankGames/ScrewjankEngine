// STD Headers

// Library Headers
#include <gtest/gtest.h>

// Void Engine Headers
#include <ScrewjankEngine/containers/Array.hpp>

using namespace sj;

namespace container_tests {

    TEST(ArrayTests, ElementAccessTest)
    {
        array<int, 3> arr = {1, 2, 3};
        ASSERT_EQ(arr.front(), arr[0]);
        ASSERT_EQ(arr.back(), arr[2]);

        ASSERT_EQ(1, arr[0]);
        ASSERT_EQ(2, arr[1]);
        ASSERT_EQ(3, arr[2]);

#ifdef SJ_DEBUG
        ASSERT_DEATH(arr.at(5), ".*");
#endif // SJ_DEBUG
    }

    TEST(ArrayTests, ModifyElementTest)
    {
        array<int, 5> arr = {1, 2, 3, 4, 5};
        arr[0] = 2;

        ASSERT_EQ(2, arr[0]);

        arr.at(3) = 16;
        ASSERT_EQ(16, arr[3]);
    }

    TEST(ArrayTests, IterationTest)
    {
        array<int, 5> arr = {1, 1, 1, 1, 1};
        int i = 0;

        for (int element : arr) {
            ASSERT_EQ(1, element);
            i++;
        }

        ASSERT_EQ(5, i);
    }

    TEST(ArrayTests, CopyConstructionTest)
    {
        array<int, 3> arr({1, 2, 3});
        array<int, 3> arr2(arr);

        for (uint32_t i = 0; i < arr.capacity(); i++) {
            ASSERT_EQ(i + 1, arr[i]);
            ASSERT_EQ(i + 1, arr2[i]);
        }
    }

    TEST(ArrayTests, CopyAssignmentOperatorTest)
    {
        array<int, 3> arr({1, 2, 3});
        array<int, 3> arr2 = arr;

        for(uint32_t i = 0; i < arr.capacity(); i++)
        {
            ASSERT_EQ(i + 1, arr[i]);
            ASSERT_EQ(i + 1, arr2[i]);
        }
    }

    TEST(ArrayTests, MoveConstructionTest)
    {
        array<int, 3> arr(array<int, 3>({1, 2, 3}));

        for(uint32_t i = 0; i < arr.capacity(); i++)
        {
            ASSERT_EQ(i + 1, arr[i]);
        }
    }

    TEST(ArrayTests, MoveAssignmentOperatorTest)
    {
        array<int, 3> arr = array<int, 3>({1, 2, 3});

        for(uint32_t i = 0; i < arr.capacity(); i++)
        {
            ASSERT_EQ(i + 1, arr[i]);
        }
    }

    TEST(ArrayTests, EqualityComparisonTest)
    {
        array<int, 3> arr1 = {1, 2, 3};
        array<int, 3> arr2 = {4, 5, 6};
        array<float, 3> arr3 = {1.0f, 2.0f, 3.0f};
        array<int, 3> arr4 = {1, 2, 3};

        ASSERT_FALSE(arr1 == arr2);
        ASSERT_TRUE(arr1 == arr4);
        ASSERT_TRUE(arr1 != arr2);
    }

    TEST(ArrayTests, FindTest)
    {
        array<int, 5> arr = {1, 2, 3, 4, 5};

        auto res = std::find(arr.begin(), arr.end(), 6);

        ASSERT_TRUE(res == arr.end());

        res = std::find(arr.begin(), arr.end(), 2);
        ASSERT_TRUE(res != arr.end());
        ASSERT_EQ(arr[1], *res);
    }

} // namespace container_tests
