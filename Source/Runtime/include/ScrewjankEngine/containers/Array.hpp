#pragma once
// STD Headers
#include <cstddef>
#include <concepts>
#include <array>

// Screwjank Headers
#include <ScrewjankEngine/core/Assert.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/system/memory/HeapZone.hpp>
#include <ScrewjankEngine/utils/Concepts.hpp>

namespace sj 
{
    template <class T, std::size_t N> 
	using array = std::array<T, N>;
    
    template <class T, class size_type=uint32_t>
    class dynamic_array
    {
    public:
        // STL type aliases
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator_concept = std::contiguous_iterator_tag;

        /**
         * Constructor
         */
        dynamic_array(HeapZoneBase* zone, size_type capacity);
        dynamic_array(std::initializer_list<T> elems);

        /**
         * Copy Constructor 
         */
        dynamic_array(const dynamic_array<T, size_type>& other);

        /**
         * Move Constructor 
         */
        dynamic_array(dynamic_array<T, size_type>&& other);

        /**
         * Destructor 
         */
        ~dynamic_array();

        /**
         * Array index operator
         * @note Complexity is O(1)
         * @return reference to element at spot index in array
         */
        reference operator[](size_type index);

        /**
         * Array index operator
         * @note Complexity is O(1)
         * @return reference to element at spot index in array
         */
        const_reference operator[](size_type index) const;

        /**
         * Inequality comparison operator
         */
        friend bool operator==(const dynamic_array<T, size_type>& lhs,
                               const dynamic_array<T, size_type>& rhs)
        {
            if(lhs.m_capacity != rhs.m_capacity)
            {
                return false;
            }

            for(size_type i = 0; i < lhs.m_capacity; i++)
            {
                if(lhs[i] != rhs[i])
                {
                    return false;
                }
            }

            return true;
        }

        friend inline bool operator!=(const dynamic_array<T, size_type>& lhs,
                                      const dynamic_array<T, size_type>& rhs)
        {
            return !(lhs == rhs);
        }

        /**
         * Bounds-checked element index operator
         */
        auto&& at(this auto&& self, size_type index); // -> (const?) T&

        /**
         * Returns reference to element at index 0
         */
        auto&& front(this auto&& self); // -> (const?) T&

        /**
         * Returns reference to last index of array
         */
        auto&& back(this auto&& self); // -> (const?) T&

        /**
         * @return Size of the array
         */
        size_type capacity() const;

        /**
         * Returns pointer to backing raw C array, allows modification of elements
         */
        auto&& data(this auto&& self); // -> (const?) T*

        /**
         * Function to allow use in ranged based for loops
         */
        auto begin(this auto&& self); // -> (const?) T*
        auto end(this auto&& self); // -> (const?) T*

        void resize(size_type newCapacity);

    private:
        /** Heap zone we live in */
        HeapZoneBase* m_heapZone = nullptr;

        /** C style array this datastructure encapsulates */
        T* m_array = nullptr;
        size_type m_capacity = 0;
    };
} // namespace sj

#include <ScrewjankEngine/containers/Array.inl>
