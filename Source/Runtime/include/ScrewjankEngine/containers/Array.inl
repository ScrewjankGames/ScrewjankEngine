#include <ScrewjankEngine/containers/Array.hpp>

namespace sj
{
	    /**
     * Equality comparison operator
     */
    template <class T, size_t N, class SizeType>
    inline bool operator==(const array<T, N>& lhs, const array<T, N>& rhs)
    {
        for (size_t i = 0; i < N; i++) {
            if (lhs[i] != rhs[i]) {
                return false;
            }
        }

        return true;
    }

    template <class T, size_t N, class SizeType>
    template <class... Args>
    inline constexpr array<T, N, SizeType>::array(Args... list) : m_array {list...}
    {
    }

    template <class T, size_t N, class SizeType>
    constexpr typename array<T, N, SizeType>::reference array<T, N, SizeType>::operator[](array<T, N, SizeType>::size_type index)
    {
        return m_array[index];
    }

    template <class T, size_t N, class SizeType>
    constexpr typename array<T, N, SizeType>::const_reference
    array<T, N, SizeType>::operator[](array<T, N, SizeType>::size_type index) const
    {
        return m_array[index];
    }

    template <class T, size_t N, class SizeType>
    inline constexpr auto&& array<T, N, SizeType>::at(this auto&& self, size_type index)
    {
        SJ_ASSERT(index >= 0 && index < N, "Array out of bounds!");
        return self.m_array[index];
    }

    template <class T, size_t N, class SizeType>
    inline constexpr auto&& array<T, N, SizeType>::front(this auto&& self)
    {
        return self.m_array[0];
    }

    template <class T, size_t N, class SizeType>
    inline constexpr auto&& array<T, N, SizeType>::back(this auto&& self)
    {
        return self.m_array[N - 1];
    }

    template <class T, size_t N, class SizeType>
    inline constexpr typename array<T, N, SizeType>::size_type array<T, N, SizeType>::capacity() const
    {
        return N;
    }

    template <class T, size_t N, class SizeType>
    inline constexpr auto array<T, N, SizeType>::data(this auto&& self)
    {
        return self.m_array;
    }

    template <class T, size_t N, class SizeType>
    inline constexpr auto array<T, N, SizeType>::begin(this auto&& self)
    {
        return self.m_array;
    }

    template <class T, size_t N, class SizeType>
    inline constexpr auto array<T, N, SizeType>::end(this auto&& self)
    {
        return &(self.m_array[N]);
    }
}