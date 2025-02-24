#pragma once

// Parent Include
#include <ScrewjankEngine/containers/StaticVector.hpp>

// Engine Includes
#include <ScrewjankShared/utils/Assert.hpp>

namespace sj
{
    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline static_vector<T, N, tOpts, SizeType>::static_vector(SizeType count) noexcept
    {
        resize(count);
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline static_vector<T, N, tOpts, SizeType>::static_vector(SizeType count, const T& value) noexcept
    {
        resize(count, value);
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline static_vector<T, N, tOpts, SizeType>::static_vector(std::initializer_list<T> vals) noexcept
    {
        for(const T& val : vals)
        {
            push_back(val);
        }
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline static_vector<T, N, tOpts, SizeType>& static_vector<T, N, tOpts, SizeType>::operator=(std::initializer_list<T> vals) noexcept
    {
        clear();

        for(const T& val : vals)
        {
            push_back(val);
        }

        return *this;
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline auto&& static_vector<T, N, tOpts, SizeType>::operator[](this auto&& self,
                                                            const SizeType index) noexcept
    {
        SJ_ASSERT(index >= 0 && index < N, "Error: Array Index Out of Bounds");
        return self.m_cArray[index];
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    void static_vector<T, N, tOpts, SizeType>::push_back(const T& value) noexcept
    {
        SJ_ASSERT(m_Count < N, "Error! Trying to add to fixed size array that is full!");
        m_cArray[m_Count] = value;
        m_Count++;
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    void static_vector<T, N, tOpts, SizeType>::push_back(T&& value) noexcept
    {
        SJ_ASSERT(m_Count < N, "Error! Trying to add to fixed size array that is full!");
        new(&m_cArray[m_Count]) T(std::forward<T>(value));
        m_Count++;
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline static_vector<T, N, tOpts, SizeType>::iterator
    static_vector<T, N, tOpts, SizeType>::insert(const_iterator pos, const T& value)
    {
        return emplace(pos, value);
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline static_vector<T, N, tOpts, SizeType>::iterator
    static_vector<T, N, tOpts, SizeType>::insert(const_iterator pos, T&& value)
    {
        return emplace(pos, std::forward<T>(value));
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    template <class... Args>
    inline static_vector<T, N, tOpts, SizeType>::iterator
    static_vector<T, N, tOpts, SizeType>::emplace(const_iterator pos, Args&&... args) noexcept
    {
        SizeType idx = static_cast<SizeType>(pos - m_cArray);
        SJ_ASSERT(idx >= 0 && idx <= m_Count, "Emplace index out of bounds");
        SJ_ASSERT(m_Count < N, "Cannot emplace into full vector");

        if constexpr(tOpts.kStableOperations)
        {
            if(m_Count > 0)
            {
                for(SizeType i = m_Count - 1; i >= idx; i--)
                {
                    new(&m_cArray[i + 1]) T(std::move(m_cArray[i]));
                }
            }

            m_Count++;
        }
        else
        {
            emplace_back(std::move(m_cArray[idx]));
        }

        new(&m_cArray[idx]) T(std::forward<Args>(args)...);
        return &m_cArray[idx];
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    template <class... Args>
    inline void static_vector<T, N, tOpts, SizeType>::emplace_back(Args&&... args) noexcept
    {
        SJ_ASSERT(m_Count < N, "Error! Trying to add to fixed size array that is full!");

        new(&m_cArray[m_Count]) T(std::forward<Args>(args)...);
        m_Count++;
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline void static_vector<T, N, tOpts, SizeType>::erase_element(const T& value) noexcept
    {
        for(SizeType i = 0; i < m_Count; i++)
        {
            if(m_cArray[i] == value)
            {
                erase(i);
                break;
            }
        }
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline void static_vector<T, N, tOpts, SizeType>::erase(SizeType idx) noexcept
    {
        SJ_ASSERT(idx < m_Count, "Erase index out of bounds");

        m_cArray[idx].~T();

        if(idx != m_Count - 1)
        {
            m_cArray[idx] = m_cArray[m_Count - 1];
        }

        m_Count--;
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline SizeType static_vector<T, N, tOpts, SizeType>::size() const noexcept
    {
        return m_Count;
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline void static_vector<T, N, tOpts, SizeType>::resize(SizeType new_size) noexcept
    {
        resize(new_size, T {});
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline void static_vector<T, N, tOpts, SizeType>::resize(SizeType new_size, const T& value) noexcept
    {
        SJ_ASSERT(new_size <= capacity(), "Cannot resize static vector past it's own capacity");

        if(new_size < m_Count)
        {
            // If the vector is shrinking, destroy the elements that aren't getting copied
            for(size_t i = new_size; i < m_Count; i++)
            {
                m_cArray[i].~T();
            }
        }
        else if(new_size > m_Count)
        {
            for(auto i = m_Count; i < N; i++)
            {
                new(&m_cArray[i]) T(value);
            }
        }

        m_Count = new_size;
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline SizeType static_vector<T, N, tOpts, SizeType>::capacity() const noexcept
    {
        return (SizeType)N;
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline T* static_vector<T, N, tOpts, SizeType>::data() noexcept
    {
        return m_cArray;
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline void static_vector<T, N, tOpts, SizeType>::clear() noexcept
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

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline decltype(auto) static_vector<T, N, tOpts, SizeType>::begin(this auto&& self) noexcept
    {
        return &self.m_cArray[0];
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline decltype(auto) static_vector<T, N, tOpts, SizeType>::end(this auto&& self) noexcept
    {
        return self.m_cArray + self.m_Count;
    }

    template <class T, size_t N, VectorOptions tOpts, class SizeType>
    inline bool static_vector<T, N, tOpts, SizeType>::empty() const noexcept
    {
        return size() == 0;
    }
}