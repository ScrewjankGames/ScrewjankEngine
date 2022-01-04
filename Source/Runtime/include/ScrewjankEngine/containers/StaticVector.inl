#include <ScrewjankEngine/containers/StaticVector.hpp>

namespace sj
{
    template <class T, size_t N>
    inline StaticVector<T, N>::StaticVector() : m_Count(0)
    {
    }

    template <class T, size_t N>
    inline T& StaticVector<T, N>::operator[](const size_t index)
    {
        SJ_ASSERT(index >= 0 && index < N, "Error: Array Index Out of Bounds");
        return m_cArray[index];
    }

    template <class T, size_t N>
    inline const T& StaticVector<T, N>::operator[](const size_t index) const
    {
        SM_ASSERT(index >= 0 && index < N, "Error: Array Index Out of Bounds");
        return m_cArray[index];
    }

    template <class T, size_t N>
    void StaticVector<T, N>::Add(const T& value)
    {
        SJ_ASSERT(m_Count < N, "Error! Trying to add to fixed size array that is full!");
        m_cArray[m_Count] = value;
        m_Count++;
    }

    template <class T, size_t N>
    void StaticVector<T, N>::Add(T&& value)
    {
        SJ_ASSERT(m_Count < N, "Error! Trying to add to fixed size array that is full!");
        new(&m_cArray[m_Count]) T(std::forward<T>(value));
        m_Count++;
    }

    template <class T, size_t N>
    inline void StaticVector<T, N>::EraseElement(const T& value)
    {
        for(size_t i = 0; i < N; i++)
        {
            if(m_cArray[i] == value)
            {
                Erase(i);
            }
        }
    }

    template <class T, size_t N>
    inline void StaticVector<T, N>::Erase(size_t idx)
    {
        m_cArray[idx].~T();

        if(idx != m_Count - 1)
        {
            m_cArray[idx] = m_cArray[m_Count - 1];
        }

        m_Count--;
    }

    template <class T, size_t N>
    inline size_t StaticVector<T, N>::Count() const
    {
        return m_Count;
    }

    template <class T, size_t N>
    inline size_t StaticVector<T, N>::Capacity() const
    {
        return N;
    }

    template <class T, size_t N>
    inline T* StaticVector<T, N>::begin()
    {
        return std::begin(m_cArray);
    }

    template <class T, size_t N>
    inline T* StaticVector<T, N>::end()
    {
        return std::begin(m_cArray) + m_Count;
    }
}