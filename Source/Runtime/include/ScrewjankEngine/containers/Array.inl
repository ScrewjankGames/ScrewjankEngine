#pragma once
#include <ScrewjankEngine/containers/Array.hpp>

namespace sj
{

    template <class T, class size_type>
    inline dynamic_array<T, size_type>::dynamic_array(size_type capacity, IMemSpace* zone)
        : m_MemSpace(zone), m_capacity(capacity)
    {
        if(capacity > 0)
        {
            m_array = m_MemSpace->AllocateType<T>(capacity);
        }

        if constexpr (!std::is_trivially_constructible_v<T>)
        {
            // Default construct all elements
            for(size_type i = 0; i < capacity; i++)
            {
                new(std::addressof(m_array[i])) T();
            }
        }
    }

    template <class T, class size_type>
    inline dynamic_array<T, size_type>::dynamic_array(std::initializer_list<T> elems) 
        : m_MemSpace(MemorySystem::GetCurrentMemSpace()),
          m_capacity(static_cast<size_type>( elems.size() ) )
    {
        if(m_capacity > 0)
        {
            m_array = m_MemSpace->AllocateType<T>(m_capacity);
        }

        // Construct all elements
        size_type i = 0;
        for(const T& elem : elems)
        {
            new(std::addressof(m_array[i])) T(elem);
            i++;
        }
    }

    template <class T, class size_type>
    inline dynamic_array<T, size_type>::dynamic_array(const dynamic_array<T, size_type>& other) 
        : dynamic_array(other.size(), MemorySystem::GetCurrentMemSpace())
    {
        if constexpr (std::is_trivially_constructible_v<T>)
        {
            memcpy(m_array, other.m_array, sizeof(T) * m_capacity);
        }
        else
        {
            int i = 0;
            for(const T& elem : other)
            {
                m_array[i] = elem;
                i++;
            }
        }
    }

    template <class T, class size_type>
    inline dynamic_array<T, size_type>::dynamic_array(dynamic_array<T, size_type>&& other) 
        : m_MemSpace(other.m_MemSpace), m_array(other.m_array), m_capacity(other.m_capacity)
    {
        other.m_array = nullptr;
        other.m_capacity = 0;
    }

    template <class T, class size_type>
    inline dynamic_array<T, size_type>::~dynamic_array()
    {
        if(m_array == nullptr)
        {
            return;
        }

        if constexpr(!std::is_trivially_destructible_v<T>)
        {
            for(T& element : *this)
            {
                element.~T();
            }
        }

        m_MemSpace->Free(m_array);
    }

    template <class T, class size_type>
    typename dynamic_array<T, size_type>::reference dynamic_array<T, size_type>::operator[](size_type index)
    {
        return m_array[index];
    }

    template <class T, class size_type>
    typename dynamic_array<T, size_type>::const_reference
    dynamic_array<T, size_type>::operator[](size_type index) const
    {
        return m_array[index];
    }

    template <class T, class size_type>
    inline auto&& dynamic_array<T, size_type>::at(this auto&& self, size_type index)
    {
        SJ_ASSERT(index >= 0 && index < self.m_capacity, "dynamic_array out of bounds!");
        return self.m_array[index];
    }

    template <class T, class size_type>
    inline auto&& dynamic_array<T, size_type>::front(this auto&& self)
    {
        return self.m_array[0];
    }

    template <class T, class size_type>
    inline auto&& dynamic_array<T, size_type>::back(this auto&& self)
    {
        return self.m_array[self.m_capacity - 1];
    }

    template <class T, class size_type>
    inline size_type dynamic_array<T, size_type>::size() const
    {
        return m_capacity;
    }

    template <class T, class size_type>
    inline auto&& dynamic_array<T, size_type>::data(this auto&& self)
    {
        return self.m_array;
    }

    template <class T, class size_type>
    inline auto dynamic_array<T, size_type>::begin(this auto&& self)
    {
        return self.m_array;
    }

    template <class T, class size_type>
    inline auto dynamic_array<T, size_type>::end(this auto&& self)
    {
        return self.m_array + self.m_capacity;
    }

    template <class T, class size_type>
    inline void dynamic_array<T, size_type>::resize(size_type newCapacity)
    {
        T* newArray = m_MemSpace->AllocateType<T>(newCapacity);
        if constexpr(std::is_trivially_constructible_v<T>)
        {
            memcpy(newArray, m_array, sizeof(T) * m_capacity);
        }
        else
        {
            // Move over everything that will fit
            for(size_type i = 0; i < std::min( m_capacity, newCapacity); i++)
            {
                new(std::addressof(newArray[i])) T(std::move(m_array[i]));
            }

            if constexpr(!std::is_trivially_destructible_v<T>)
            {
                // Destroy anything that doesn't fit ( when newCapacity < m_capacity)
                for(size_type i = newCapacity; i < m_capacity; i++)
                {
                    m_array[i].~T();
                }
            }

            // Default construct empty spaces
            for(size_type i = m_capacity; i < newCapacity; i++)
            {
                new(std::addressof(newArray[i])) T();
            }
        }

        m_array = newArray;
        m_capacity = newCapacity;
    }
} // namespace sj