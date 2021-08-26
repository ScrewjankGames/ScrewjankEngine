#pragma once
#include <ScrewjankEngine/core/Assert.hpp>

namespace sj
{
    /**
     * Static stack structure
     */
    template <class T, size_t tNumElements>
    class StaticStack
    {
      public:
        StaticStack() : m_Elements(), m_Count(0)
        {
            static_assert(tNumElements > 0, "Cannot have stack of size zero");
        }

        void Push(const T& value)
        {
            SJ_ASSERT(m_Count < tNumElements, "Static Stack is full!");
            m_Elements[m_Count] = value;
            m_Count++;
        }

        void Push(T&& value)
        {
            SJ_ASSERT(m_Count < tNumElements, "Static Stack is full!");
            m_Elements[m_Count] = std::forward<T>(value);
            m_Count++;
        }

        void Pop() 
        {
            SJ_ASSERT(m_Count > 0, "Cannot pop empty stack!");
            m_Elements[m_Count - 1].~T();
            m_Count--;
        }

        T& Top()
        {
            return const_cast<T&>(const_cast<const StaticStack*>(this)->Top());
        }

        const T& Top() const
        {
            SJ_ASSERT(m_Count != 0, "Cannot peek at top of empty stack!");
            return m_Elements[m_Count - 1];
        }

        size_t Count() const
        {
            return m_Count;
        }

        bool IsEmpty() const
        {
            return Count() == 0;
        }

      private:
        T m_Elements[tNumElements];
        size_t m_Count;
    };
}