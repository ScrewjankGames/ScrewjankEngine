#include <ScrewjankEngine/containers/Deque.hpp>

namespace sj
{
    template <class T>
    inline Deque<T>::Deque(HeapZone* heap_zone, size_t capacity)
    {
        m_Capacity = capacity;

        if(capacity > 0)
        {
            Reserve(capacity);
        }
    }

    template <class T>
    inline void Deque<T>::Reserve(size_t newCapacity)
    {
        T* new_buffer = m_HeapZone->Allocate(sizeof(T) * newCapacity);

        // Destroy elements that won't make the cut
        if(newCapacity < m_Count)
        {
            size_t startIdx = (m_Offset + newCapacity) % m_Capacity;

            for(int i = 0; i < m_Count - newCapacity; i++)
            {
                size_t idx = (startIdx + i) % m_Capacity;
                m_Buffer[idx].~T();
            }
        }

        // Pop items off of old queue and place into new queue
        for(size_t i = 0; i < newCapacity; i++)
        {

        }

    }
    
    template <class T>
    inline bool Deque<T>::IsEmpty() const
    {
        return m_Count == 0;
    }
    
    template <class T>
    inline T& Deque<T>::Front()
    {
        return const_cast<T&>(const_cast<Deque*>(this)->Front());
    }

    template <class T>
    inline const T& Deque<T>::Front() const
    {
        SJ_ASSERT(!IsEmpty(), "Cannot access front element of empty deque");
        return m_Buffer[m_Offset];
    }
}