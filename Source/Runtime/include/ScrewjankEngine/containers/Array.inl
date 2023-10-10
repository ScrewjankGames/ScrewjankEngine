#include <ScrewjankEngine/containers/Array.hpp>

namespace sj
{
	    /**
     * Equality comparison operator
     */
    template <class T, size_t N>
    inline bool operator==(const array<T, N>& lhs, const array<T, N>& rhs)
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
    inline constexpr array<T, N>::array(Args... list) : m_array {list...}
    {
    }

    template <class T, size_t N>
    constexpr typename array<T, N>::reference array<T, N>::operator[](array<T, N>::size_type index)
    {
        return m_array[index];
    }

    template <class T, size_t N>
    constexpr typename array<T, N>::const_reference
    array<T, N>::operator[](array<T, N>::size_type index) const
    {
        return m_array[index];
    }

    template <class T, size_t N>
    inline constexpr auto&& array<T, N>::at(this auto&& self, size_type index)
    {
        SJ_ASSERT(index >= 0 && index < N, "Array out of bounds!");
        return self.m_array[index];
    }

    template <class T, size_t N>
    inline constexpr auto&& array<T, N>::front(this auto&& self)
    {
        return self.m_array[0];
    }

    template <class T, size_t N>
    inline constexpr auto&& array<T, N>::back(this auto&& self)
    {
        return self.m_array[N - 1];

    }

    template <class T, size_t N>
    inline constexpr typename array<T, N>::size_type array<T, N>::capacity() const
    {
        return N;
    }

    template <class T, size_t N>
    inline constexpr auto&& array<T, N>::data(this auto&& self)
    {
        return self.m_array;
    }

    template <class T, size_t N>
    inline constexpr auto&& array<T, N>::begin(this auto&& self)
    {
        return std::begin(self.m_array);
    }

    template <class T, size_t N>
    inline constexpr auto&& array<T, N>::end(this auto&& self)
    {
        return std::end(self.m_array);
    }
}