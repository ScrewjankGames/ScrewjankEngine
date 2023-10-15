#pragma once
// STD Headers
#include <cstddef>
#include <concepts>

// Screwjank Headers
#include <ScrewjankEngine/core/Assert.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/utils/Concepts.hpp>

namespace sj {

    template <class T, size_t N, class SizeType = uint32_t>
    class array
    {
      public:
        // STL type aliases
        using value_type = T;
        using size_type = SizeType;
        using difference_type = std::ptrdiff_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator_concept = std::contiguous_iterator_tag;

        /**
         * Constructor
         */
        constexpr array() = default;

        /**
         * Constructor
         * Allows Arrays to be braced-list initialized
         */
        template <class... Args>
        constexpr array(Args... list);

        /**
         * Array index operator
         * @note Complexity is O(1)
         * @return reference to element at spot index in array
         */
        constexpr reference operator[](size_type index);

        /**
         * Array index operator
         * @note Complexity is O(1)
         * @return reference to element at spot index in array
         */
        constexpr const_reference operator[](size_type index) const;

        /**
         * Equality comparison operator
         */
        friend inline bool operator==(const array<T, N>& lhs, const array<T, N>& rhs)
        {
            for(size_type i = 0; i < N; i++)
            {
                if(lhs[i] != rhs[i])
                {
                    return false;
                }
            }

            return true;
        }

        /**
         * Inequality comparison operator
         */
        friend inline bool operator!=(const array<T, N>& lhs, const array<T, N>& rhs)
        {
            return !(lhs == rhs);
        }

        /**
         * Bounds-checked element index operator
         */
        constexpr auto&& at(this auto&& self, size_type index); // -> (const?) T&

        /**
         * Returns reference to element at index 0
         */
        constexpr auto&& front(this auto&& self); // -> (const?) T&

        /**
         * Returns reference to last index of array
         */
        constexpr auto&& back(this auto&& self); // -> (const?) T&

        /**
         * @return Size of the array
         */
        constexpr size_type capacity() const;

        /**
         * Returns pointer to backing raw C array, allows modification of elements
         */
        constexpr auto data(this auto&& self); // -> (const?) T*

        /**
         * Function to allow use in ranged based for loops
         */
        constexpr auto begin(this auto&& self); // -> (const?) T*
        constexpr auto end(this auto&& self); // -> (const?) T*

      private:
        /** C style array this datastructure encapsulates */
        T m_array[N];
    };
} // namespace sj

// Include inlines
#include <ScrewjankEngine/containers/Array.inl>
