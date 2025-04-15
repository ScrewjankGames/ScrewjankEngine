// STD Headers
#include <array>
#include <ranges>
// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include <ScrewjankEngine/containers/UnmanagedList.hpp>

using namespace sj;

namespace container_tests
{
    struct FwdListDummy
    {
        FwdListDummy* GetNext()
        {
            return next;
        }

        void SetNext(FwdListDummy* next)
        {
            this->next = next;
        }

        int val;
        FwdListDummy* next;
    };

    struct BiDiDummy
    {
        BiDiDummy* GetNext()
        {
            return next;
        }

        void SetNext(BiDiDummy* next)
        {
            this->next = next;
        }

        BiDiDummy* GetPrev()
        {
            return prev;
        }

        void SetPrev(BiDiDummy* newPrev)
        {
            prev = newPrev;
        }

        BiDiDummy* prev;
        BiDiDummy* next;
        int val;
    };

    TEST(UnmanagedListTests, IterationTest)
    {
        constexpr int kCount = 5;
        std::array<BiDiDummy, kCount> dummiesStorage;
        for(int i = 0; i < kCount; i++)
        {
            dummiesStorage[i].val = i;
        }

        sj::unmanaged_list<BiDiDummy> dummies;
        for(BiDiDummy& dummy : dummiesStorage)
        {
            dummies.push_back(&dummy);
        }

        int i = 0;
        for(BiDiDummy& dummy : dummies)
        {
            ASSERT_EQ(dummy.val, i);
            i++;
        }

    }

    TEST(UnmanagedListTests, ForwardListPushPopTest)
    {
        // Sequential Push Pop
        {
            unmanaged_list<FwdListDummy> fwdList;

            for(int i = 0; i < 10000; i++)
            {
                fwdList.push_front(new FwdListDummy {i});
            }

            for(int i = 10000; i > 0; i--)
            {
                ASSERT_EQ(i, fwdList.size());
                ASSERT_EQ(i-1, fwdList.front().val);
                FwdListDummy& dummy = fwdList.front();
                fwdList.pop_front();

                delete &(dummy);
            }
        }

        // Random push pop
        {
            unmanaged_list<FwdListDummy> fwdList;
            int seed = 123456;
            srand(seed);
        
            int pushed = 0;
            int popped = 0;

            for(int i = 0; i < 10000; i++)
            {
                int pop = rand() % 2;

                if( pop && fwdList.size() > 0 )
                {
                    FwdListDummy& dummy = fwdList.front();
                    popped += dummy.val;
                    fwdList.pop_front();
                    delete &(dummy);
                }
                else
                {
                    pushed += i;
                    fwdList.push_front(new FwdListDummy {i});
                }
            }

            while(fwdList.size() > 0)
            {
                popped += fwdList.front().val;
                delete &(fwdList.front());
                fwdList.pop_front();
            }

            ASSERT_EQ(pushed, popped);
        }
    }
}