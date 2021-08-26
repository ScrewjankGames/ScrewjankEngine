#include <ScrewjankEngine/containers/Array.hpp>

namespace sj
{
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
    inline auto Array<T, N>::begin() const -> decltype(std::begin(m_Array))
    {
        return std::begin(m_Array);
    }

    template <class T, size_t N>
    inline auto Array<T, N>::end() -> decltype(std::end(m_Array))
    {
        return std::end(m_Array);
    }

    template <class T, size_t N>
    inline auto Array<T, N>::end() const -> decltype(std::end(m_Array))
    {
        return std::end(m_Array);
    }
}