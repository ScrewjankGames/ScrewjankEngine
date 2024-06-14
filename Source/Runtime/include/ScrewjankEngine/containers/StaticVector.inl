// Parent Include
#include <ScrewjankEngine/containers/StaticVector.hpp>

// Engine Includes
#include <ScrewjankShared/utils/Assert.hpp>

namespace sj
{
    template <class T, size_t N, class SizeType>
    inline static_vector<T, N, SizeType>::static_vector(std::initializer_list<T> vals)
    {
        for(const T& val : vals)
        {
            push_back(val);
        }
    }

    template <class T, size_t N, class SizeType>
    inline static_vector<T, N, SizeType>& static_vector<T, N, SizeType>::operator=(std::initializer_list<T> vals)
    {
        clear();

        for(const T& val : vals)
        {
            push_back(val);
        }

        return *this;
    }

    template <class T, size_t N, class SizeType>
    inline T& static_vector<T, N, SizeType>::operator[](const SizeType index)
    {
        SJ_ASSERT(index >= 0 && index < N, "Error: Array Index Out of Bounds");
        return m_cArray[index];
    }

    template <class T, size_t N, class SizeType>
    inline const T& static_vector<T, N, SizeType>::operator[](const SizeType index) const
    {
        SM_ASSERT(index >= 0 && index < N, "Error: Array Index Out of Bounds");
        return m_cArray[index];
    }

    template <class T, size_t N, class SizeType>
    void static_vector<T, N, SizeType>::push_back(const T& value)
    {
        SJ_ASSERT(m_Count < N, "Error! Trying to add to fixed size array that is full!");
        m_cArray[m_Count] = value;
        m_Count++;
    }

    template <class T, size_t N, class SizeType>
    void static_vector<T, N, SizeType>::push_back(T&& value)
    {
        SJ_ASSERT(m_Count < N, "Error! Trying to add to fixed size array that is full!");
        new(&m_cArray[m_Count]) T(std::forward<T>(value));
        m_Count++;
    }

    template <class T, size_t N, class SizeType>
    template <class... Args>
    inline void static_vector<T, N, SizeType>::emplace_back(Args&&... args)
    {
        SJ_ASSERT(m_Count < N, "Error! Trying to add to fixed size array that is full!");

        new(&m_cArray[m_Count]) T(std::forward<Args>(args)...);
        m_Count++;
    }

    template <class T, size_t N, class SizeType>
    inline void static_vector<T, N, SizeType>::erase_element(const T& value)
    {
        for(SizeType i = 0; i < N; i++)
        {
            if(m_cArray[i] == value)
            {
                erase(i);
            }
        }
    }

    template <class T, size_t N, class SizeType>
    inline void static_vector<T, N, SizeType>::erase(SizeType idx)
    {
        SJ_ASSERT(idx < m_Count, "Erase index out of bounds");

        m_cArray[idx].~T();

        if(idx != m_Count - 1)
        {
            m_cArray[idx] = m_cArray[m_Count - 1];
        }

        m_Count--;
    }

    template <class T, size_t N, class SizeType>
    inline SizeType static_vector<T, N, SizeType>::size() const
    {
        return m_Count;
    }

    template <class T, size_t N, class SizeType>
    inline SizeType static_vector<T, N, SizeType>::capacity() const
    {
        return (SizeType)N;
    }

    template <class T, size_t N, class SizeType>
    inline T* static_vector<T, N, SizeType>::data()
    {
        return m_cArray;
    }

    template <class T, size_t N, class SizeType>
    inline void static_vector<T, N, SizeType>::clear()
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

    template <class T, size_t N, class SizeType>
    inline T* static_vector<T, N, SizeType>::begin()
    {
        return std::begin(m_cArray);
    }

    template <class T, size_t N, class SizeType>
    inline T* static_vector<T, N, SizeType>::end()
    {
        return std::begin(m_cArray) + m_Count;
    }
}