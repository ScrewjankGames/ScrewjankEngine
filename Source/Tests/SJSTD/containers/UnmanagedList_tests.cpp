// STD Headers
#include <array>

// Library Headers
#include "gtest/gtest.h"

// SJ Headers
import sj.std.containers.unmanaged_list;

using namespace sj;

namespace container_tests
{
    struct FwdListDummy
    {
        FwdListDummy* get_next()
        {
            return next;
        }

        void set_next(FwdListDummy* nextNode)
        {
            this->next = nextNode;
        }

        int val;
        FwdListDummy* next;
    };

    struct BiDiDummy
    {
        BiDiDummy* get_next()
        {
            return next;
        }

        void set_next(BiDiDummy* nextNode)
        {
            this->next = nextNode;
        }

        BiDiDummy* get_prev()
        {
            return prev;
        }

        void set_prev(BiDiDummy* newPrev)
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
        std::array<BiDiDummy, kCount> dummiesStorage = {};
        for(unsigned i = 0; i < kCount; i++)
        {
            dummiesStorage.at(i).val = static_cast<int>(i);
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
                fwdList.push_front(new FwdListDummy {.val=i, .next=nullptr});
            }

            int count = 0;
            for(FwdListDummy& _ : fwdList)
                count++;

            ASSERT_EQ(count, 10000);

            
            for(int i = 10000; i > 0; i--)
            {
                ASSERT_EQ(i-1, fwdList.front().val);
                FwdListDummy& dummy = fwdList.front();
                fwdList.pop_front();

                delete &(dummy);
            }

            ASSERT_TRUE(fwdList.empty());

        }

        // Random push pop
        {
            unmanaged_list<FwdListDummy> fwdList;
            unsigned int seed = 123456;
            srand(seed);
        
            int pushed = 0;
            int popped = 0;

            for(int i = 0; i < 10000; i++)
            {
                int pop = rand() % 2;

                if( pop && !fwdList.empty() )
                {
                    FwdListDummy& dummy = fwdList.front();
                    popped += dummy.val;
                    fwdList.pop_front();
                    delete &(dummy);
                }
                else
                {
                    pushed += i;
                    fwdList.push_front(new FwdListDummy {.val=i, .next=nullptr});
                }
            }

            while(!fwdList.empty())
            {
                popped += fwdList.front().val;
                delete &(fwdList.front());
                fwdList.pop_front();
            }

            ASSERT_EQ(pushed, popped);
        }
    }
}