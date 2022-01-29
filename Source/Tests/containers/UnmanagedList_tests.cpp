// STD Headers

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

    struct BiDiDummy : public FwdListDummy
    {
        BiDiDummy* GetPrev()
        {
            return prev;
        }

        void SetPrev(BiDiDummy* newPrev)
        {
            prev = newPrev;
        }
        

        BiDiDummy* prev;
    };

    TEST(UnmanagedListTests, ForwardListPushPopTest)
    {
        // Sequential Push Pop
        {
            UnmanagedList<FwdListDummy> fwdList;

            for(int i = 0; i < 10000; i++)
            {
                fwdList.PushFront(new FwdListDummy {i});
            }

            for(int i = 10000; i > 0; i--)
            {
                ASSERT_EQ(i, fwdList.Size());
                ASSERT_EQ(i-1, fwdList.Front().val);
                delete &(fwdList.Front());
                fwdList.PopFront();
            }
        }

        // Random push pop
        {
            UnmanagedList<FwdListDummy> fwdList;
            int seed = 123456;
            srand(seed);
        
            int pushed = 0;
            int popped = 0;

            for(int i = 0; i < 10000; i++)
            {
                int pop = rand() % 2;

                if( pop && fwdList.Size() > 0 )
                {
                    popped += fwdList.Front().val;
                    delete &(fwdList.Front());
                    fwdList.PopFront();
                }
                else
                {
                    pushed += i;
                    fwdList.PushFront(new FwdListDummy {i});
                }
            }

            while(fwdList.Size() > 0)
            {
                popped += fwdList.Front().val;
                delete &(fwdList.Front());
                fwdList.PopFront();
            }

            ASSERT_EQ(pushed, popped);
        }
    }
}