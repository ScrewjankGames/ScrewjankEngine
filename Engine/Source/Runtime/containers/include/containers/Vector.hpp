#pragma once

// STD Headers
#include <utility>

// Library Headers

// Scewjank Headers
#include "core/Assert.hpp"
#include "core/MemorySystem.hpp"

namespace Screwjank {

    template <class T>
    class Vector
    {
      public:
        // STL type aliases
        using value_type = T;
        using size_type = size_t;
        using difference_type = std::ptrdiff_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator_concept = std::contiguous_iterator_tag;

        Vector(Allocator* allocator = MemorySystem::GetDefaultAllocator(),
               size_t capacity_hint = 0);

        Vector(std::initializer_list<T> list,
               Allocator* allocator = MemorySystem::GetDefaultAllocator());

        ~Vector();

        /** Array Index Operator */
        T& operator[](size_t index);

        /** Array Index Operator */
        const T& operator[](size_t index) const;

        /** Bounds-checked element access */
        T& At(size_t index);

        /** Bounds-checked element access */
        const T& At(size_t index) const;

        /**
         * @return The first element in the vector
         */
        T& Front();

        /**
         * @return The first element in the vector
         */
        const T& Front() const;

        /**
         * @return The first element in the vector
         */
        T& Back();

        /**
         * @return The first element in the vector
         */
        const T& Back() const;

        /**
         * Inserts a new element onto the end of the array
         * @note Iterators are invalidated if the array is resized
         */
        void PushBack(const T& value);

        /**
         * Constructs an element into the array
         * @return A reference to the element inserted
         */
        template <class... Args>
        T& EmplaceBack(Args&&... args);

        /**
         * @return The number of elements actively stored in the array
         */
        size_t Size() const;

        /**
         * @return The number of elements the vector can currently contain
         */
        size_t Capacity() const;

      private:
        size_t m_Size;
        size_t m_Capacity;
        Allocator* m_Allocator;
        T* m_Data;

        /** Doubles the size of the data buffer, and copies the old data over */
        void GrowVector();

      public:
        /**
         * Function to allow use in ranged based for loops
         */
        T* begin();

        /**
         * Function to allow use in ranged based for loops
         */
        T* end();
    };

    template <class T>
    inline Vector<T>::Vector(Allocator* allocator, size_t capacity_hint)
        : m_Data(nullptr), m_Allocator(allocator), m_Size(0), m_Capacity(capacity_hint)
    {
        if (capacity_hint != 0) {
            m_Data = (T*)(m_Allocator->Allocate(sizeof(T) * capacity_hint, alignof(T)));
        }
    }

    template <class T>
    inline Vector<T>::Vector(std::initializer_list<T> list, Allocator* allocator)
        : Vector(allocator, list.size())
    {
        std::copy(list.begin(), list.end(), m_Data);
        m_Size = list.size();
    }

    template <class T>
    inline Vector<T>::~Vector()
    {
        // Please don't leak memory
        m_Allocator->Free(m_Data);
    }

    template <class T>
    inline T& Vector<T>::operator[](size_t index)
    {
        return m_Data[index];
    }

    template <class T>
    inline const T& Vector<T>::operator[](size_t index) const
    {
        return m_Data[index];
    }

    template <class T>
    inline T& Vector<T>::At(size_t index)
    {
        SJ_ASSERT(index >= 0 && index < m_Size, "Index out of bounds");

        return m_Data[index];
    }

    template <class T>
    inline const T& Vector<T>::At(size_t index) const
    {
        SJ_ASSERT(index >= 0 && index < m_Size, "Index out of bounds");

        return m_Data[index];
    }

    template <class T>
    inline T& Vector<T>::Front()
    {
        SJ_ASSERT(m_Size > 0, "Cannot access front of an empty vector");
        return m_Data[0];
    }

    template <class T>
    inline const T& Vector<T>::Front() const
    {
        SJ_ASSERT(m_Size > 0, "Cannot access front of an empty vector");
        return m_Data[0];
    }

    template <class T>
    inline T& Vector<T>::Back()
    {
        SJ_ASSERT(m_Size > 0, "Cannot access back of empty vector");

        return m_Data[m_Size - 1];
    }

    template <class T>
    inline const T& Vector<T>::Back() const
    {
        SJ_ASSERT(m_Size > 0, "Cannot access back of empty vector");

        return m_Data[m_Size - 1];
    }

    template <class T>
    inline void Vector<T>::PushBack(const T& value)
    {

        if (m_Size >= m_Capacity) {
            GrowVector();
        }

        // Copy element into array
        auto index = m_Size;
        new (&m_Data[index]) T(value);

        // Increment size of vector
        ++m_Size;
    }

    template <class T>
    inline size_t Vector<T>::Size() const
    {
        return m_Size;
    }

    template <class T>
    inline size_t Vector<T>::Capacity() const
    {
        return m_Capacity;
    }

    template <class T>
    template <class... Args>
    inline T& Vector<T>::EmplaceBack(Args&&... args)
    {
        if (m_Size >= m_Capacity) {
            GrowVector();
        }

        // Construct element in allocated space
        auto index = m_Size;
        auto element = new (&m_Data[index]) T(std::forward<Args>(args)...);

        // Increment size of vector
        ++m_Size;

        return *element;
    }

    template <class T>
    inline void Vector<T>::GrowVector()
    {
        // Array needs to grow
        size_t new_capacity = (m_Capacity != 0) ? m_Capacity * 2 : 1;

        // Allocate a new buffer
        T* new_buffer = (T*)(m_Allocator->Allocate(sizeof(T) * new_capacity, alignof(T)));

        // Move old buffer into new buffer
        for (size_t i = 0; i < m_Capacity; ++i) {
            new (&new_buffer[i]) T(std::move(m_Data[i]));
        }

        // Release old buffer
        if (m_Data != nullptr) {
            m_Allocator->Free(m_Data);
        }

        // Update capacity variable
        m_Capacity = new_capacity;

        // Allocate new buffer
        m_Data = new_buffer;
    }

    template <class T>
    inline T* Vector<T>::begin()
    {
        return m_Data;
    }

    template <class T>
    inline T* Vector<T>::end()
    {
        // return one past the last element
        return m_Data + m_Size;
    }

} // namespace Screwjank
