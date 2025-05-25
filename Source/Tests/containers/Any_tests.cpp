// STD Headers
#include <memory_resource>

// Library Headers
#include <gtest/gtest.h>
#include <vector>

// SJ Headers

import sj.shared.containers;
import sj.shared.core;

using namespace sj;

namespace container_tests
{
    static_assert(sj::is_instantiation_of_v<static_any<256>, static_any>, "Any isn't an any?");
    static_assert(!sj::is_instantiation_of_v<int, static_any>, "Int is an any?");
    static_assert(!sj::is_instantiation_of_v<std::vector<int>, static_any>, "Vector is an any?");
    static_assert(!sj::is_instantiation_of_v<std::array<int,15>, static_any>, "Array is an any?");

    TEST(AnyTests, DefaultConstructionTest)
    {
        sj::static_any<256> nothing; // default construction

        ASSERT_TRUE(true);
    }

    TEST(AnyTests, ConcreteSimpleCopyConstructionTest)
    {
        int test = 100;
        sj::static_any<8> lvalueTest(test);
        test = lvalueTest.get<int>();
        ASSERT_EQ(100, test);
    }

    TEST(AnyTests, ConcreteSimpleMoveConstructionTest)
    {
        sj::static_any<8> rvalueTest(5);
        int test = rvalueTest.get<int>();
        ASSERT_EQ(5, test);
    }

    TEST(AnyTests, AnyToAnySimpleCopyConstructionTest)
    {
        int test = 100;
        sj::static_any<8> lvalueTest(test);
        sj::static_any<8> anyLvalueTest(lvalueTest);
        
        // Make sure source wasn't changed
        test = lvalueTest.get<int>();
        ASSERT_EQ(100, test);

        // Make sure destination correct
        test = anyLvalueTest.get<int>();
        ASSERT_EQ(100, test);
    }

    TEST(AnyTests, AnyToAnySimpleMoveConstructionTest)
    {
        int test = 100;
        sj::static_any<8> lvalueTest(test);
        sj::static_any<8> anyRvalueTest(std::move(lvalueTest));
        test = anyRvalueTest.get<int>();
        ASSERT_EQ(100, test);
    }

    TEST(AnyTests, ConcreteComplexCopyConstructionTest)
    {
        using container_any = sj::static_any<sizeof(std::vector<int>), alignof(std::vector<int>)>;

        std::vector<int> testVec = {1, 2, 3, 4};

        container_any containerCopyTest(testVec);

        // Make sure original vector intact
        ASSERT_EQ(4, testVec.size());
        for(size_t i = 0; i < testVec.size(); i++)
            ASSERT_EQ(i+1, testVec[i]);

        // Make sure copied vector matches
        std::vector<int>& extractedTestVec1 = containerCopyTest.get<std::vector<int>>(); 
        ASSERT_EQ(4, extractedTestVec1.size());
        for(size_t i = 0; i < extractedTestVec1.size(); i++)
            ASSERT_EQ(i+1, extractedTestVec1[i]);

        // Make sure our reference really updates what's in the any
        extractedTestVec1.push_back(5);
        std::vector<int>& extractedTestVec2 = containerCopyTest.get<std::vector<int>>();
        ASSERT_EQ(5, extractedTestVec1.size());
        ASSERT_EQ(5, extractedTestVec2.size());
    }

    TEST(AnyTests, ConcreteComplexMoveConstructionTest)
    {
        using container_any = sj::static_any<sizeof(std::vector<int>), alignof(std::vector<int>)>;

        std::vector<int> testVec = {1, 2, 3, 4};
        container_any containerMoveTest(std::move(testVec));
        ASSERT_EQ(0, testVec.size()); //NOLINT(clang-analyzer-cplusplus.Move)

        std::vector<int>& extractedTestVec1 = containerMoveTest.get<std::vector<int>>(); 
        ASSERT_EQ(4, extractedTestVec1.size());
        for(size_t i = 0; i < extractedTestVec1.size(); i++)
            ASSERT_EQ(i+1, extractedTestVec1[i]);

        // Make sure our reference really updates what's in the any
        extractedTestVec1.push_back(5);
        std::vector<int>& extractedTestVec2 = containerMoveTest.get<std::vector<int>>();
        ASSERT_EQ(5, extractedTestVec1.size());
        ASSERT_EQ(5, extractedTestVec2.size());
    }

    TEST(AnyTests, AnyToAnyComplexCopyConstructionTest)
    {
        using container_any = sj::static_any<sizeof(std::vector<int>), alignof(std::vector<int>)>;
        container_any original(std::vector<int>{1,2,3,4,5});

        static_assert(is_instantiation_of_v< std::decay_t<container_any>, static_any>);

        // Copy from any -> any
        container_any anyToAnyCopyTest(original);
        std::vector<int>& originalVec = original.get<std::vector<int>>();
         ASSERT_EQ(5, originalVec.size());

        std::vector<int> copiedVec = anyToAnyCopyTest.get<std::vector<int>>();
        ASSERT_EQ(5, copiedVec.size());
        for(size_t i = 0; i < copiedVec.size(); i++)
        {
            ASSERT_EQ(i+1, originalVec[i]);
            ASSERT_EQ(i+1, copiedVec[i]);
        }
    }

    TEST(AnyTests, AnyToAnyComplexMoveConstructionTest)
    {
        using container_any = sj::static_any<sizeof(std::vector<int>), alignof(std::vector<int>)>;
        std::vector<int> originalVec = {1, 2, 3, 4, 5};
        container_any original(originalVec);

        // Copy from any -> any
        container_any anyToAnyCopyTest(std::move(original));
        ASSERT_EQ(5, originalVec.size());

        std::vector<int>& movedVec = anyToAnyCopyTest.get<std::vector<int>>();
        ASSERT_EQ(5, movedVec.size());
        for(size_t i = 0; i < movedVec.size(); i++)
        {
            ASSERT_EQ(i+1, movedVec[i]);
        }
    }

} // namespace container_tests