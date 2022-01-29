#pragma once
#include <ScrewjankEngine/system/HeapZone.hpp>

namespace sj
{
	template<class T>
    class Deque
    {
    public:
        /**
         * Default constructor 
         */
        Deque() = default;

        /**
         * Value Initialization Constructor
         * @param capacity The initial capacity for this queue
         */
        Deque(HeapZone* heap_zone, size_t capacity);

        /**
         * Change queue memory footprint, disards elements that don't fit
         */
        void Reserve(size_t newCapacity);

    private:
        /** Heap Zone used to manage this queue */
        HeapZone* m_HeapZone = nullptr;

        /** Memory buffer for queue's elements */
        T* m_Buffer = nullptr;
        
        /** Number of elements this queue can hold */
        size_t m_Capacity = 0;

        /** Number of elements in the queue */
        size_t m_Count = 0;

        /** Index of first element in buffer */
        size_t m_Offset = 0;
    };
}

// Include Inlines
#include <ScrewjankEngine/containers/Queue.inl>