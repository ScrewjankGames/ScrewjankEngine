#pragma once
// STD Headers
#include <cstddef>

// Library Headers

// Screwjank Headers
#include "core/Assert.hpp"
#include "system/Memory.hpp"
#include "utils/Concepts.hpp"

namespace sj {

    template <class T, size_t N>
    class Array
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

        /**
         * Constructor
         */
        constexpr Array() = default;

        /**
         * Constructor
         * Allows Arrays to be braced-list initialized
         */
        template <class... Args>
        constexpr Array(Args... list);

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
         * Inequality comparison operator
         */
        friend inline bool operator!=(const Array<T, N>& lhs, const Array<T, N>& rhs)
        {
            return !(lhs == rhs);
        }

        /**
         * Bounds-checked element index operator
         */
        constexpr reference At(size_type index);

        /**
         * Bounds-checked element index operator
         */
        constexpr const_reference At(size_type index) const;

        /**
         * Returns reference to element at index 0
         */
        constexpr reference Front();

        /**
         * Returns const reference to the element at index 0
         */
        constexpr const_reference Front() const;

        /**
         * Returns reference to last index of array
         */
        constexpr reference Back();

        /**
         * Returns const reference to last index of array
         */
        constexpr const_reference Back() const;

        /**
         * @return Size of the array
         */
        constexpr size_type Size() const;

        /**
         * Returns pointer to backing raw C array, allows modification of elements
         */
        constexpr T* Data();

        /**
         * Returns pointer to backing raw C array, disallows modification of elements
         */
        constexpr const T* Data() const;

      private:
        /** C style array this datastructure encapsulates */
        T m_Array[N];

      public:
        /**
         * Function to allow use in ranged based for loops
         */
        auto begin() -> decltype(std::begin(m_Array));

        /**
         * Function to allow use in ranged based for loops
         */
        auto end() -> decltype(std::end(m_Array));
    };

    /**
     * Equality comparison operator
     */
    template <class T, size_t N>
    inline bool operator==(const Array<T, N>& lhs, const Array<T, N>& rhs)
    {
        for (size_t i = 0; i < N; i++) {
            if (lhs[i] != rhs[i]) {
                return false;
            }
        }

        return true;
    }

    template <class T, size_t N>
    template <class... Args>
    inline constexpr Array<T, N>::Array(Args... list) : m_Array {list...}
    {
    }

    template <class T, size_t N>
    constexpr typename Array<T, N>::reference Array<T, N>::operator[](Array<T, N>::size_type index)
    {
        return m_Array[index];
    }

    template <class T, size_t N>
    constexpr typename Array<T, N>::const_reference
    Array<T, N>::operator[](Array<T, N>::size_type index) const
    {
        return m_Array[index];
    }

    template <class T, size_t N>
    constexpr typename Array<T, N>::reference Array<T, N>::At(Array<T, N>::size_type index)
    {
        SJ_ASSERT(index >= 0 && index < N, "Array out of bounds!");
        return m_Array[index];
    }

    template <class T, size_t N>
    constexpr typename Array<T, N>::const_reference
    Array<T, N>::At(Array<T, N>::size_type index) const
    {
        SJ_ASSERT(index >= 0 && index < N, "Array out of bounds!");
        return m_Array[index];
    }

    template <class T, size_t N>
    constexpr typename Array<T, N>::reference Array<T, N>::Front()
    {
        return m_Array[0];
    }

    template <class T, size_t N>
    constexpr typename Array<T, N>::const_reference Array<T, N>::Front() const
    {
        return m_Array[0];
    }

    template <class T, size_t N>
    constexpr typename Array<T, N>::reference Array<T, N>::Back()
    {
        return m_Array[N - 1];
    }

    template <class T, size_t N>
    constexpr typename Array<T, N>::const_reference Array<T, N>::Back() const
    {
        return m_Array[N - 1];
    }

    template <class T, size_t N>
    inline constexpr typename Array<T, N>::size_type Array<T, N>::Size() const
    {
        return N;
    }

    template <class T, size_t N>
    constexpr T* Array<T, N>::Data()
    {
        return m_Array;
    }

    template <class T, size_t N>
    constexpr const T* Array<T, N>::Data() const
    {
        return m_Array;
    }

    template <class T, size_t N>
    inline auto Array<T, N>::begin() -> decltype(std::begin(m_Array))
    {
        return std::begin(m_Array);
    }

    template <class T, size_t N>
    inline auto Array<T, N>::end() -> decltype(std::end(m_Array))
    {
        return std::end(m_Array);
    }

} // namespace sj
