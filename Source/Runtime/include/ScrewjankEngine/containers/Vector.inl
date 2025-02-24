#pragma once
#include <ScrewjankEngine/containers/Vector.hpp>

namespace sj
{
    template <class T>
    inline dynamic_vector<T>::dynamic_vector() : dynamic_vector(MemorySystem::GetCurrentMemSpace())
    {
    }

    template <class T>
    inline dynamic_vector<T>::dynamic_vector(size_t count) : dynamic_vector(MemorySystem::GetCurrentMemSpace(), count)
    {
    }

    template <class T>
    inline dynamic_vector<T>::dynamic_vector(size_t count, const T& value)
        : dynamic_vector(MemorySystem::GetCurrentMemSpace(), count, value)
    {
    }

    template <class T>
    inline dynamic_vector<T>::dynamic_vector(std::initializer_list<T> list)
        : dynamic_vector(MemorySystem::GetCurrentMemSpace(), list)
    {
    }

    template <class T>
    inline dynamic_vector<T>::dynamic_vector(IMemSpace* mem_space, T* buffer, size_t count) : dynamic_vector(mem_space)
    {
        m_Data = buffer;
        m_Capacity = count;
        m_Size = count;
    }

    template <class T>
    inline dynamic_vector<T>::dynamic_vector(IMemSpace* mem_space)
        : m_Data(nullptr), m_BackingZone(mem_space), m_Size(0), m_Capacity(0)
    {
    }

    template <class T>
    inline dynamic_vector<T>::dynamic_vector(IMemSpace* mem_space, size_t count) : dynamic_vector(mem_space)
    {
        resize(count);
    }

    template <class T>
    inline dynamic_vector<T>::dynamic_vector(IMemSpace* mem_space, size_t count, const T& value) : dynamic_vector(mem_space)
    {
        resize(count, value);
    }

    template <class T>
    inline dynamic_vector<T>::dynamic_vector(IMemSpace* mem_space, std::initializer_list<T> list) 
        : dynamic_vector(mem_space, list, std::from_range_t{})
    {

    }

    template <class T>
    template<class Range> requires std::ranges::range<Range>
    inline dynamic_vector<T>::dynamic_vector(IMemSpace* mem_space, Range&& inputRange, std::from_range_t _) 
        : dynamic_vector(mem_space)
    {
        // Make sure vector has space for size() elements
        reserve(inputRange.size());

        // Copy elements into m_Data
        size_t i = 0;
        for (auto&& element : inputRange)
        {
            new (&m_Data[i]) T(element);
            i++;
        }

        m_Size = inputRange.size();
    }

    template <class T>
    inline dynamic_vector<T>::dynamic_vector(const dynamic_vector<T>& other)
        : m_Data(nullptr), m_BackingZone(other.m_BackingZone), m_Size(0), m_Capacity(0)
    {
        reserve(other.m_Capacity);

        size_t i = 0;
        for (auto& elem : other)
        {
            new (&m_Data[i]) T(elem);
            i++;
        }

        m_Size = other.size();
    }

    template <class T>
    inline dynamic_vector<T>::dynamic_vector(dynamic_vector<T>&& other) noexcept
    {
        m_BackingZone = other.m_BackingZone;
        m_Data = other.m_Data;
        m_Size = other.m_Size;
        m_Capacity = other.m_Capacity;

        // The "other" vector was trashed by the move operations. Reduce it to an empty state
        other.m_Data = nullptr;
        other.m_Size = 0;
        other.m_Capacity = 0;
    }

    template <class T>
    inline dynamic_vector<T>::~dynamic_vector()
    {
        // Call the destructor of each contained object
        clear();

        // Please don't leak memory
        if (m_Data != nullptr)
        {
            m_BackingZone->Free(m_Data);
        }
    }

    template <class T>
    inline dynamic_vector<T>& dynamic_vector<T>::operator=(const dynamic_vector<T>& other)
    {
        clear();

        if (other.size() > m_Capacity)
        {
            resize(other.size());
        }

        size_t i = 0;
        for (auto& element : other)
        {
            new (&m_Data[i]) T(element);
            i++;
        }

        m_Size = other.size();

        return *this;
    }

    template <class T>
    inline dynamic_vector<T>& dynamic_vector<T>::operator=(dynamic_vector<T>&& other) noexcept
    {
        clear();

        // Steal the other object's member variables
        m_Size = other.m_Size;
        m_Capacity = other.m_Capacity;

        if (m_BackingZone != other.m_BackingZone)
        {
            SJ_ENGINE_LOG_WARN("Move assignment is changing vector allocator!");
        }

        m_BackingZone = other.m_BackingZone;
        m_Data = other.m_Data;

        // Reset state of other vector
        other.m_Size = 0;
        other.m_Capacity = 0;
        other.m_Data = nullptr;

        return *this;
    }

    template <class T>
    inline dynamic_vector<T>& dynamic_vector<T>::operator=(std::initializer_list<T> list)
    {
        // Destroy all the elements currently in the list
        clear();

        // Ensure the vector is large enough to fit the new list
        if (list.size() > m_Capacity)
        {
            reserve(list.size());
        }

        size_t i = 0;
        for (auto& element : list)
        {
            new (&m_Data[i]) T(element);
            i++;
        }

        m_Size = list.size();

        return *this;
    }

    template <class T>
    inline T& dynamic_vector<T>::operator[](const size_t index)
    {
        return m_Data[index];
    }

    template <class T>
    inline const T& dynamic_vector<T>::operator[](const size_t index) const
    {
        return m_Data[index];
    }

    template <class T>
    inline auto&& dynamic_vector<T>::at(this auto&& self, size_t index)
    {
        SJ_ASSERT(index < self.capacity(), "Cannot access front of an empty vector");
        return self.m_Data[index];
    }

    template <class T>
    inline auto&& dynamic_vector<T>::front(this auto&& self)
    {
        SJ_ASSERT(self.m_Size > 0, "Cannot access front of an empty vector");
        return self.m_Data[0];
    }
    
    template <class T>
    inline auto&& dynamic_vector<T>::back(this auto&& self)
    {
        SJ_ASSERT(self.m_Size > 0, "Cannot access back of empty vector");

        return self.m_Data[self.m_Size - 1];
    }

    template <class T>
    inline void dynamic_vector<T>::push_back(const T& value)
    {
        if (m_Size >= m_Capacity)
        {
            // Array needs to grow
            size_t new_capacity = (m_Capacity != 0) ? m_Capacity * 2 : 1;
            reserve(new_capacity);
        }

        // Copy element into array
        auto index = m_Size;
        new (&m_Data[index]) T(value);

        // Increment size of vector
        ++m_Size;
    }

    template <class T>
    inline void dynamic_vector<T>::push_back(const dynamic_vector<T>& other)
    {
        // insert vector at end of this vector
        insert(m_Size, other);
    }

    template <class T>
    inline void dynamic_vector<T>::pop_back()
    {
        SJ_ASSERT(m_Size > 0, "Cannot pop empty vector");

        // Destruct the object at the back of the array and decrement size
        m_Data[m_Size - 1].~T();
        m_Size--;
    }

    template <class T>
    inline typename dynamic_vector<T>::iterator dynamic_vector<T>::insert(const size_t index, const T& value)
    {
        SJ_ASSERT(index >= 0 && index <= m_Size, "Insertion index is invalid.");

        // If you're pushing into the final spot, just resort to push_back
        if (index == m_Size)
        {
            push_back(value);
            return end() - 1;
        }

        dynamic_vector<T>::iterator it(nullptr);

        if (m_Size < m_Capacity)
        {

            // Move last element into the end spot (end may be unitialized data)

            // Move every element to the right
            for (auto i = m_Size; i >= index; i--)
            {
                new (&m_Data[i]) T(m_Data[i - 1]);
            }

            // insert value into buffer
            m_Data[index] = value;
            it = &m_Data[index];
        }
        else
        {
            // Array needs to grow, place new element in during buffer copy process

            // if size was zero, we'd have allready fallen back to push_back
            size_t new_capacity = m_Capacity * 2;

            // Allocate a new buffer
            T* new_buffer = (T*)(m_BackingZone->Allocate(sizeof(T) * new_capacity, alignof(T)));

            // Move old buffer into new buffer, up to index
            for (size_t i = 0; i < index; ++i)
            {
                new (&new_buffer[i]) T(m_Data[i]);
            }

            // Copy new element in to index position
            new (&new_buffer[index]) T(value);
            it = &m_Data[index];

            // Copy the rest of the old buffer into the new buffer
            for (auto i = index + 1; i < new_capacity; i++)
            {
                new (&new_buffer[i]) T(m_Data[i - 1]);
            }

            // Release old buffer
            if (m_Data != nullptr)
            {
                m_BackingZone->Free(m_Data);
            }

            // Update capacity variable
            m_Capacity = new_capacity;

            // Allocate new buffer
            m_Data = new_buffer;
        }

        m_Size++;
        return it;
    }

    template <class T>
    inline void dynamic_vector<T>::insert(const size_t index, const dynamic_vector& other)
    {
        SJ_ASSERT(index >= 0 && index <= m_Size, "Insertion index is invalid.");

        size_t shift = other.size();
        if (m_Capacity >= m_Size + other.size())
        {
            // There is enough space in the vector to insert other without reallocating

            // Move current data out of the way
            for (size_t i = m_Size - 1; i >= index; i--)
            {
                new (&m_Data[i + shift]) T(m_Data[i]);
            }

            // Move new data i
            for (size_t i = 0; i < other.size(); i++)
            {
                new (&m_Data[i + index]) T(other[i]);
            }

            m_Size += other.size();
        }
        else
        {
            // Allocate a new buffer with extra space
            size_t new_capacity = (m_Capacity + other.capacity()) * 2;

            T* new_buffer = (T*)m_BackingZone->Allocate(sizeof(T) * new_capacity, alignof(T));

            // Move old values into new buffer
            size_t i = 0;
            for (/***/; i < index; i++)
            {
                new (&new_buffer[i]) T(m_Data[i]);
            }

            // Copy new values into new buffer
            for (size_t j = 0; j < other.size(); i++, j++)
            {
                new (&new_buffer[i]) T(other[j]);
            }

            // Copy remainder of old buffer into new buffer
            for (size_t k = index; k < m_Size; i++, k++)
            {
                new (&new_buffer[i]) T(m_Data[k]);
            }

            // Update container data
            if (m_Data != nullptr)
            {
                m_BackingZone->Free(m_Data);
            }

            m_Data = new_buffer;
            m_Size += other.size();
            m_Capacity = new_capacity;
        }
    }

    template <class T>
    template <class... Args>
    inline void dynamic_vector<T>::emplace(const size_t index, Args&&... args)
    {
        SJ_ASSERT(index >= 0 && index <= m_Size, "Emplacement index is invalid.");

        // If you're pushing into the final spot, just resort to push_back
        if (index == m_Size)
        {
            emplace_back(std::forward<Args>(args)...);
            return;
        }

        if (m_Size < m_Capacity)
        {
            // Move every element to the right
            for (auto i = m_Size; i >= index; i--)
            {
                new (&m_Data[i]) T(std::move(m_Data[i - 1]));
            }

            // insert value into buffer
            new (&m_Data[index]) T(std::forward<Args>(args)...);
        }
        else
        {
            // Array needs to grow, place new element in during buffer copy process

            // if size was zero, we'd have allready fallen back to push_back
            size_t new_capacity = m_Capacity * 2;

            // Allocate a new buffer
            T* new_buffer = (T*)(m_BackingZone->Allocate(sizeof(T) * new_capacity, alignof(T)));

            // Move old buffer into new buffer, up to index
            for (size_t i = 0; i < index; ++i)
            {
                new (&new_buffer[i]) T(std::move(m_Data[i]));
            }

            // Copy new element in to index position
            new (&new_buffer[index]) T(std::forward<Args>(args)...);

            // Copy the rest of the old buffer into the new buffer
            for (auto i = index + 1; i < new_capacity; i++)
            {
                new (&new_buffer[i]) T(std::move(m_Data[i - 1]));
            }

            // Release old buffer
            if (m_Data != nullptr)
            {
                m_BackingZone->Free(m_Data);
            }

            // Update capacity variable
            m_Capacity = new_capacity;

            // Allocate new buffer
            m_Data = new_buffer;
        }

        m_Size++;
    }

    template <class T>
    template <class... Args>
    inline T& dynamic_vector<T>::emplace_back(Args&&... args)
    {
        if (m_Size >= m_Capacity)
        {
            size_t new_capacity = (m_Capacity != 0) ? m_Capacity * 2 : 1;
            reserve(new_capacity);
        }

        // Construct element in allocated space
        auto index = m_Size;
        auto element = new (&m_Data[index]) T(std::forward<Args>(args)...);

        // Increment size of vector
        ++m_Size;

        return *element;
    }

    template <class T>
    inline typename dynamic_vector<T>::iterator dynamic_vector<T>::erase(iterator pos)
    {
        // Destroy contained element
        auto& element = *pos;
        element.~T();

        // Move everything one to the left
        void* dest = &(*pos);
        void* first = &(*(pos + 1));
        void* last = &(*end());
        auto count = uintptr_t(last) - uintptr_t(first);

        std::memmove(dest, first, count);

        // Reduce size
        m_Size--;
        return iterator(pos);
    }

    template <class T>
    inline size_t dynamic_vector<T>::size() const
    {
        return m_Size;
    }

    template <class T>
    inline void dynamic_vector<T>::resize(size_t new_size)
    {
        resize(new_size, T {});
    }

    template <class T>
    inline void dynamic_vector<T>::resize(size_t new_size, const T& value)
    {
        reserve(new_size);

        if (new_size > m_Size)
        {
            for (auto i = m_Size; i < m_Capacity; i++)
            {
                new (&m_Data[i]) T(value);
            }
        }

        m_Size = new_size;
    }

    template <class T>
    inline void dynamic_vector<T>::reserve(size_t new_capacity)
    {
        if (new_capacity == m_Capacity)
        {
            return;
        }

        // Determine how many elements will be in the new vector
        size_t new_size = std::min(m_Size, new_capacity);

        if (new_size < m_Size)
        {
            // If the vector is shrinking, destroy the elements that aren't getting copied
            for (size_t i = new_size; i < m_Size; i++)
            {
                m_Data[i].~T();
            }
        }

        // Allocate a new buffer
        T* new_buffer = (T*)(m_BackingZone->Allocate(sizeof(T) * new_capacity, alignof(T)));

        // Move old buffer into new buffer
        for (size_t i = 0; i < new_size; i++)
        {
            new (&new_buffer[i]) T(std::move(m_Data[i]));
        }

        // Update capacity variable
        m_Capacity = new_capacity;

        // Update size of vector
        m_Size = new_size;

        // Swap buffer pointers
        if (m_Data != nullptr)
        {
            m_BackingZone->Free(m_Data);
        }

        m_Data = new_buffer;
    }

    template <class T>
    inline size_t dynamic_vector<T>::capacity() const
    {
        return m_Capacity;
    }

    template <class T>
    inline void dynamic_vector<T>::clear()
    {
        for (auto& element : *this)
        {
            element.~T();
        }

        m_Size = 0;
    }

    template <class T>
    inline bool dynamic_vector<T>::empty()
    {
        return size() == 0;
    }
    
    template <class T>
    inline auto dynamic_vector<T>::data(this auto&& self)
    {
        return self.m_Data;
    }

    template <class T>
    inline auto dynamic_vector<T>::begin(this auto&& self)
    {
        return self.m_Data;
    }

    template <class T>
    inline auto dynamic_vector<T>::end(this auto&& self)
    {
        return self.m_Data + self.m_Size;
    }
}