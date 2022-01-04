#include <ScrewjankEngine/containers/Vector.hpp>

namespace sj
{


    template <class T>
    inline Vector<T>::Vector() : Vector(MemorySystem::GetCurrentHeapZone())
    {
    }

    template <class T>
    inline Vector<T>::Vector(size_t count) : Vector(MemorySystem::GetCurrentHeapZone(), count)
    {
    }

    template <class T>
    inline Vector<T>::Vector(size_t count, const T& value)
        : Vector(MemorySystem::GetCurrentHeapZone(), count, value)
    {
    }

    template <class T>
    inline Vector<T>::Vector(std::initializer_list<T> list)
        : Vector(MemorySystem::GetCurrentHeapZone(), list)
    {
    }

    template <class T>
    inline Vector<T>::Vector(HeapZone* heap_zone)
        : m_Data(nullptr), m_BackingZone(heap_zone), m_Size(0), m_Capacity(0)
    {
    }

    template <class T>
    inline Vector<T>::Vector(HeapZone* heap_zone, size_t count) : Vector(heap_zone)
    {
        Resize(count);
    }

    template <class T>
    inline Vector<T>::Vector(HeapZone* heap_zone, size_t count, const T& value) : Vector(heap_zone)
    {
        Resize(count, value);
    }

    template <class T>
    inline Vector<T>::Vector(HeapZone* heap_zone, std::initializer_list<T> list) : Vector(heap_zone)
    {
        // Make sure vector has space for size() elements
        Reserve(list.size());

        // Copy elements into m_Data
        size_t i = 0;
        for (auto& element : list)
        {
            new (&m_Data[i]) T(element);
            i++;
        }

        m_Size = list.size();
    }

    template <class T>
    inline Vector<T>::Vector(const Vector<T>& other)
        : m_Data(nullptr), m_BackingZone(other.m_BackingZone), m_Size(0), m_Capacity(0)
    {
        Reserve(other.m_Capacity);

        size_t i = 0;
        for (auto& elem : other)
        {
            new (&m_Data[i]) T(elem);
            i++;
        }

        m_Size = other.Size();
    }

    template <class T>
    inline Vector<T>::Vector(Vector<T>&& other) noexcept
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
    inline Vector<T>::~Vector()
    {
        // Call the destructor of each contained object
        Clear();

        // Please don't leak memory
        if (m_Data != nullptr)
        {
            m_BackingZone->Free(m_Data);
        }
    }

    template <class T>
    inline Vector<T>& Vector<T>::operator=(const Vector<T>& other)
    {
        Clear();

        if (other.Size() > m_Capacity)
        {
            Resize(other.Size());
        }

        size_t i = 0;
        for (auto& element : other)
        {
            new (&m_Data[i]) T(element);
            i++;
        }

        m_Size = other.Size();

        return *this;
    }

    template <class T>
    inline Vector<T>& Vector<T>::operator=(Vector<T>&& other) noexcept
    {
        Clear();

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
    inline Vector<T>& Vector<T>::operator=(std::initializer_list<T> list)
    {
        // Destroy all the elements currently in the list
        Clear();

        // Ensure the vector is large enough to fit the new list
        if (list.size() > m_Capacity)
        {
            Reserve(list.size());
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
    inline T& Vector<T>::operator[](const size_t index)
    {
        return m_Data[index];
    }

    template <class T>
    inline const T& Vector<T>::operator[](const size_t index) const
    {
        return m_Data[index];
    }

    template <class T>
    inline T& Vector<T>::At(const size_t index)
    {
        SJ_ASSERT(index >= 0 && index < m_Size, "Index out of bounds");

        return m_Data[index];
    }

    template <class T>
    inline const T& Vector<T>::At(const size_t index) const
    {
        SJ_ASSERT(index >= 0 && index < m_Size, "Index out of bounds");

        return m_Data[index];
    }

    template <class T>
    inline T& Vector<T>::Front()
    {
        SJ_ASSERT(m_Size > 0, "Cannot access front of an empty vector");
        return m_Data[0];
    }

    template <class T>
    inline const T& Vector<T>::Front() const
    {
        SJ_ASSERT(m_Size > 0, "Cannot access front of an empty vector");
        return m_Data[0];
    }

    template <class T>
    inline T& Vector<T>::Back()
    {
        SJ_ASSERT(m_Size > 0, "Cannot access back of empty vector");

        return m_Data[m_Size - 1];
    }

    template <class T>
    inline const T& Vector<T>::Back() const
    {
        SJ_ASSERT(m_Size > 0, "Cannot access back of empty vector");

        return m_Data[m_Size - 1];
    }

    template <class T>
    inline void Vector<T>::PushBack(const T& value)
    {
        if (m_Size >= m_Capacity)
        {
            // Array needs to grow
            size_t new_capacity = (m_Capacity != 0) ? m_Capacity * 2 : 1;
            Reserve(new_capacity);
        }

        // Copy element into array
        auto index = m_Size;
        new (&m_Data[index]) T(value);

        // Increment size of vector
        ++m_Size;
    }

    template <class T>
    inline void Vector<T>::PushBack(const Vector<T>& other)
    {
        // Insert vector at end of this vector
        Insert(m_Size, other);
    }

    template <class T>
    inline void Vector<T>::PopBack()
    {
        SJ_ASSERT(m_Size > 0, "Cannot pop empty vector");

        // Destruct the object at the back of the array and decrement size
        m_Data[m_Size - 1].~T();
        m_Size--;
    }

    template <class T>
    inline typename Vector<T>::iterator Vector<T>::Insert(const size_t index, const T& value)
    {
        SJ_ASSERT(index >= 0 && index <= m_Size, "Insertion index is invalid.");

        // If you're pushing into the final spot, just resort to PushBack
        if (index == m_Size)
        {
            PushBack(value);
            return end() - 1;
        }

        Vector<T>::iterator it(nullptr);

        if (m_Size < m_Capacity)
        {

            // Move last element into the end spot (end may be unitialized data)

            // Move every element to the right
            for (auto i = m_Size; i >= index; i--)
            {
                new (&m_Data[i]) T(m_Data[i - 1]);
            }

            // Insert value into buffer
            m_Data[index] = value;
            it = &m_Data[index];
        }
        else
        {
            // Array needs to grow, place new element in during buffer copy process

            // if size was zero, we'd have allready fallen back to PushBack
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
    inline void Vector<T>::Insert(const size_t index, const Vector& other)
    {
        SJ_ASSERT(index >= 0 && index <= m_Size, "Insertion index is invalid.");

        size_t shift = other.Size();
        if (m_Capacity >= m_Size + other.Size())
        {
            // There is enough space in the vector to insert other without reallocating

            // Move current data out of the way
            for (size_t i = m_Size - 1; i >= index; i--)
            {
                new (&m_Data[i + shift]) T(m_Data[i]);
            }

            // Move new data i
            for (size_t i = 0; i < other.Size(); i++)
            {
                new (&m_Data[i + index]) T(other[i]);
            }

            m_Size += other.Size();
        }
        else
        {
            // Allocate a new buffer with extra space
            size_t new_capacity = (m_Capacity + other.Capacity()) * 2;

            T* new_buffer = (T*)m_BackingZone->Allocate(sizeof(T) * new_capacity, alignof(T));

            // Move old values into new buffer
            size_t i = 0;
            for (/***/; i < index; i++)
            {
                new (&new_buffer[i]) T(m_Data[i]);
            }

            // Copy new values into new buffer
            for (size_t j = 0; j < other.Size(); i++, j++)
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
            m_Size += other.Size();
            m_Capacity = new_capacity;
        }
    }

    template <class T>
    template <class... Args>
    inline void Vector<T>::Emplace(const size_t index, Args&&... args)
    {
        SJ_ASSERT(index >= 0 && index <= m_Size, "Emplacement index is invalid.");

        // If you're pushing into the final spot, just resort to PushBack
        if (index == m_Size)
        {
            EmplaceBack(std::forward<Args>(args)...);
            return;
        }

        if (m_Size < m_Capacity)
        {
            // Move every element to the right
            for (auto i = m_Size; i >= index; i--)
            {
                new (&m_Data[i]) T(std::move(m_Data[i - 1]));
            }

            // Insert value into buffer
            new (&m_Data[index]) T(std::forward<Args>(args)...);
        }
        else
        {
            // Array needs to grow, place new element in during buffer copy process

            // if size was zero, we'd have allready fallen back to PushBack
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
    inline T& Vector<T>::EmplaceBack(Args&&... args)
    {
        if (m_Size >= m_Capacity)
        {
            size_t new_capacity = (m_Capacity != 0) ? m_Capacity * 2 : 1;
            Reserve(new_capacity);
        }

        // Construct element in allocated space
        auto index = m_Size;
        auto element = new (&m_Data[index]) T(std::forward<Args>(args)...);

        // Increment size of vector
        ++m_Size;

        return *element;
    }

    template <class T>
    inline typename Vector<T>::iterator Vector<T>::Erase(iterator pos)
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
    inline size_t Vector<T>::Size() const
    {
        return m_Size;
    }

    template <class T>
    inline void Vector<T>::Resize(size_t new_size)
    {
        Resize(new_size, T {});
    }

    template <class T>
    inline void Vector<T>::Resize(size_t new_size, const T& value)
    {
        Reserve(new_size);

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
    inline void Vector<T>::Reserve(size_t new_capacity)
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
    inline size_t Vector<T>::Capacity() const
    {
        return m_Capacity;
    }

    template <class T>
    inline void Vector<T>::Clear()
    {
        for (auto& element : *this)
        {
            element.~T();
        }

        m_Size = 0;
    }

    template <class T>
    inline bool Vector<T>::Empty()
    {
        return Size() != 0;
    }

    template <class T>
    inline T* Vector<T>::Data()
    {
        return m_Data;
    }

    template <class T>
    inline typename Vector<T>::iterator Vector<T>::begin()
    {
        return iterator(m_Data);
    }

    template <class T>
    inline typename Vector<T>::const_iterator Vector<T>::begin() const
    {
        return const_iterator(m_Data);
    }

    template <class T>
    inline typename Vector<T>::iterator Vector<T>::end()
    {
        // return one past the last element
        return iterator(m_Data + m_Size);
    }

    template <class T>
    inline typename Vector<T>::const_iterator Vector<T>::end() const
    {
        return const_iterator(m_Data + m_Size);
    }
}