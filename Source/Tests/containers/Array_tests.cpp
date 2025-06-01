// STD Headers
#include <algorithm>

// Library Headers
#include <gtest/gtest.h>

import sj.std.containers;

using namespace sj;

namespace container_tests
{

    TEST(ArrayTests, ElementAccessTest)
    {
        dynamic_array<int> arr = {1, 2, 3};
        ASSERT_EQ(arr.front(), arr[0]);
        ASSERT_EQ(arr.back(), arr[2]);

        ASSERT_EQ(1, arr[0]);
        ASSERT_EQ(2, arr[1]);
        ASSERT_EQ(3, arr[2]);
    }

    TEST(ArrayTests, ModifyElementTest)
    {
        dynamic_array<int> arr = {1, 2, 3, 4, 5};
        arr[0] = 2;

        ASSERT_EQ(2, arr[0]);

        arr.at(3) = 16;
        ASSERT_EQ(16, arr[3]);
    }

    TEST(ArrayTests, IterationTest)
    {
        dynamic_array<int> arr = {1, 1, 1, 1, 1};
        int i = 0;

        for(int element : arr)
        {
            ASSERT_EQ(1, element);
            i++;
        }

        ASSERT_EQ(5, i);
    }

    TEST(ArrayTests, CopyConstructionTest)
    {
        dynamic_array<int> arr({1, 2, 3});
        dynamic_array<int> arr2(arr);

        for(uint32_t i = 0; i < arr.size(); i++)
        {
            ASSERT_EQ(i + 1, arr[i]);
            ASSERT_EQ(i + 1, arr2[i]);
        }
    }

    TEST(ArrayTests, CopyAssignmentOperatorTest)
    {
        dynamic_array<int> arr({1, 2, 3});
        dynamic_array<int> arr2 = arr;

        for(uint32_t i = 0; i < arr.size(); i++)
        {
            ASSERT_EQ(i + 1, arr[i]);
            ASSERT_EQ(i + 1, arr2[i]);
        }
    }

    TEST(ArrayTests, MoveConstructionTest)
    {
        dynamic_array<int> arr(dynamic_array<int>({1, 2, 3}));

        for(uint32_t i = 0; i < arr.size(); i++)
        {
            ASSERT_EQ(i + 1, arr[i]);
        }
    }

    TEST(ArrayTests, MoveAssignmentOperatorTest)
    {
        dynamic_array<int> arr = dynamic_array<int>({1, 2, 3});

        for(uint32_t i = 0; i < arr.size(); i++)
        {
            ASSERT_EQ(i + 1, arr[i]);
        }
    }

    TEST(ArrayTests, EqualityComparisonTest)
    {
        dynamic_array<int> arr1 = {1, 2, 3};
        dynamic_array<int> arr2 = {4, 5, 6};
        dynamic_array<float> arr3 = {1.0f, 2.0f, 3.0f};
        dynamic_array<int> arr4 = {1, 2, 3};

        ASSERT_FALSE(arr1 == arr2);
        ASSERT_TRUE(arr1 == arr4);
        ASSERT_TRUE(arr1 != arr2);
    }

    TEST(ArrayTests, FindTest)
    {
        dynamic_array<int> arr = {1, 2, 3, 4, 5};

        auto res = std::ranges::find(arr, 6);

        ASSERT_TRUE(res == arr.end());

        res = std::find(arr.begin(), arr.end(), 2);
        ASSERT_TRUE(res != arr.end());
        ASSERT_EQ(arr[1], *res);
    }

    TEST(ArrayTests, ResizeTest)
    {
        dynamic_array<int> arr = {1, 2, 3, 4, 5};
        
        arr.resize(10);
        ASSERT_EQ(10, arr.size());

        for(uint32_t i = 0; i < 5; i++)
        {
            ASSERT_EQ(i + 1, arr[i]);
        }

        dynamic_array<std::string> arr2 = {"foo", "bar"};
        arr2.resize(4);
        ASSERT_EQ(4, arr2.size());
        ASSERT_EQ("bar", arr2[1]);

        arr2.resize(1);
        ASSERT_EQ(1, arr2.size());
        ASSERT_EQ("foo", arr2[0]);
    }


} // namespace container_tests