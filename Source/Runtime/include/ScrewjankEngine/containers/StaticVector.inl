// Parent Include
#include <ScrewjankEngine/containers/StaticVector.hpp>

// Engine Includes
#include <ScrewjankEngine/core/Assert.hpp>

namespace sj
{
    template <class T, size_t N>
    inline static_vector<T, N>::static_vector(std::initializer_list<T> vals)
    {
        for(const T& val : vals)
        {
            add(val);
        }
    }

    template <class T, size_t N>
    inline static_vector<T, N>& static_vector<T, N>::operator=(std::initializer_list<T> vals)
    {
        clear();

        for(const T& val : vals)
        {
            add(val);
        }

        return *this;
    }

    template <class T, size_t N>
    inline T& static_vector<T, N>::operator[](const size_t index)
    {
        SJ_ASSERT(index >= 0 && index < N, "Error: Array Index Out of Bounds");
        return m_cArray[index];
    }

    template <class T, size_t N>
    inline const T& static_vector<T, N>::operator[](const size_t index) const
    {
        SM_ASSERT(index >= 0 && index < N, "Error: Array Index Out of Bounds");
        return m_cArray[index];
    }

    template <class T, size_t N>
    void static_vector<T, N>::add(const T& value)
    {
        SJ_ASSERT(m_Count < N, "Error! Trying to add to fixed size array that is full!");
        m_cArray[m_Count] = value;
        m_Count++;
    }

    template <class T, size_t N>
    void static_vector<T, N>::add(T&& value)
    {
        SJ_ASSERT(m_Count < N, "Error! Trying to add to fixed size array that is full!");
        new(&m_cArray[m_Count]) T(std::forward<T>(value));
        m_Count++;
    }

    template <class T, size_t N>
    inline void static_vector<T, N>::erase_element(const T& value)
    {
        for(size_t i = 0; i < N; i++)
        {
            if(m_cArray[i] == value)
            {
                erase(i);
            }
        }
    }

    template <class T, size_t N>
    inline void static_vector<T, N>::erase(size_t idx)
    {
        m_cArray[idx].~T();

        if(idx != m_Count - 1)
        {
            m_cArray[idx] = m_cArray[m_Count - 1];
        }

        m_Count--;
    }

    template <class T, size_t N>
    inline size_t static_vector<T, N>::count() const
    {
        return m_Count;
    }

    template <class T, size_t N>
    inline size_t static_vector<T, N>::capacity() const
    {
        return N;
    }

    template <class T, size_t N>
    inline T* static_vector<T, N>::data()
    {
        return m_cArray;
    }

    template <class T, size_t N>
    inline void static_vector<T, N>::clear()
    {
        if constexpr(!std::is_trivially_destructible<T>::value)
        {
            for(T& entry : *this)
            {
                entry.~T();
            }
        }

        m_Count = 0;
    }

    template <class T, size_t N>
    inline T* static_vector<T, N>::begin()
    {
        return std::begin(m_cArray);
    }

    template <class T, size_t N>
    inline T* static_vector<T, N>::end()
    {
        return std::begin(m_cArray) + m_Count;
    }
}