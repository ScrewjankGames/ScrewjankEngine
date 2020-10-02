#pragma once

// STD Headers
#include <compare>
#include <utility>

// Library Headers

// Scewjank Headers
#include "core/Assert.hpp"
#include "core/MemorySystem.hpp"

namespace sj {

    /**
     * Class to define both const and non-const iterators over vectors
     *  @tparam Vector_t The cv-qualified type of vector being iterated over
     */
    template <class Vector_t>
    class VectorIterator_t
    {
      public:
        // Using declarations for STL compatibility
        using iterator_category = std::contiguous_iterator_tag;
        using value_type = std::conditional_t<std::is_const<Vector_t>::value,
                                              typename const Vector_t::value_type,
                                              typename Vector_t::value_type>;
        using pointer = std::conditional_t<std::is_const<Vector_t>::value,
                                           typename Vector_t::const_pointer,
                                           typename Vector_t::pointer>;
        using const_pointer = typename Vector_t::const_pointer;

        using reference = std::conditional_t<std::is_const<Vector_t>::value,
                                             typename Vector_t::const_reference,
                                             typename Vector_t::reference>;

        using const_reference = typename Vector_t::const_reference;

        using difference_type = typename Vector_t::difference_type;

      public:
        /** Constructor */
        VectorIterator_t(pointer element) noexcept : m_CurrElement(element)
        {
        }

        /** Dereference operator overload */
        [[nodiscard]] reference operator*() const
        {
            return *m_CurrElement;
        }

        /** Arrow operator overload */
        [[nodiscard]] pointer operator->() const
        {
            return m_CurrElement;
        }

        /** Equality comparison operator */
        bool operator==(const VectorIterator_t& other) const
        {
            return m_CurrElement == other.m_CurrElement;
        }

        /** Inequality comparison operator */
        bool operator!=(const VectorIterator_t& other) const
        {
            return !(*this == other);
        }

        auto operator<=>(const VectorIterator_t& other) const = default;

        /** Addition operator overload */
        VectorIterator_t& operator+(size_t num)
        {
            m_CurrElement = m_CurrElement + num;
            return *this;
        }

        /** Addition operator overload */
        VectorIterator_t& operator-(size_t num)
        {
            m_CurrElement = m_CurrElement - num;
            return *this;
        }

        /** Pre-increment operator overload */
        VectorIterator_t& operator++()
        {
            ++m_CurrElement;
            return *this;
        }

        /** Post-increment operator overload */
        VectorIterator_t operator++(int)
        {
            VectorIterator_t tmp(*this);
            this->operator++();
            return tmp;
        }

        /** Pre-decrement operator overload */
        VectorIterator_t& operator--()
        {
            --m_CurrElement;
            return *this;
        }

        /** Post-decrement operator overload */
        VectorIterator_t& operator--(int)
        {
            VectorIterator_t tmp(*this);
            --*this;
            return tmp;
        }

        /** Compound assignment operator overload */
        VectorIterator_t& operator+=(size_t val)
        {
            m_CurrElement += val;
            return *this;
        }

        /** Compound assignment operator overload */
        VectorIterator_t& operator-=(size_t val)
        {
            m_CurrElement -= val;
            return *this;
        }

      private:
        pointer m_CurrElement;
    };

    template <class T>
    class Vector
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

        // Iterator info
        using iterator_concept = std::contiguous_iterator_tag;
        using iterator = typename VectorIterator_t<Vector<T>>;
        using const_iterator = typename VectorIterator_t<const Vector<T>>;

        /**
         * Default Constructor
         *
         */
        Vector(Allocator* allocator = MemorySystem::GetDefaultAllocator(),
               size_t capacity_hint = 0);

        /**
         * List initialization Constructor
         */
        Vector(Allocator* allocator, std::initializer_list<T> list);

        /**
         * Copy Constructor
         */
        Vector(const Vector<T>& other);

        /**
         * Move Constructor
         */
        Vector(Vector<T>&& other) noexcept;

        /**
         * Destructor
         */
        ~Vector();

        /**
         * Copy assignment from initializer list
         */
        Vector<T>& operator=(const Vector<T>& other);

        /**
         * Copy assignment from other vector
         */
        Vector<T>& operator=(Vector<T>&& other);

        /**
         * Move assignment from initializer list
         */
        Vector<T>& operator=(std::initializer_list<T>&& list);

        /** Array Index Operator */
        T& operator[](const size_t index);

        /** Array Index Operator */
        const T& operator[](const size_t index) const;

        /** Bounds-checked element access */
        T& At(const size_t index);

        /** Bounds-checked element access */
        const T& At(const size_t index) const;

        /**
         * @return The first element in the vector
         */
        T& Front();

        /**
         * @return The first element in the vector
         */
        const T& Front() const;

        /**
         * @return The first element in the vector
         */
        T& Back();

        /**
         * @return The first element in the vector
         */
        const T& Back() const;

        /**
         * Inserts a new element onto the end of the array
         * @note Iterators are invalidated if the array is resized
         */
        void PushBack(const T& value);

        /**
         * Pushes another vector onto the back of this vector
         */
        void PushBack(const Vector<T>& other);

        /**
         * Constructs an element into the array
         * @return A reference to the element inserted
         */
        template <class... Args>
        T& EmplaceBack(Args&&... args);

        /**
         * Removes and destructs element at the back of the array
         */
        void PopBack();

        /**
         * Removes and destructs element at the back of the array
         */
        void Insert(const size_t index, const T& value);

        /**
         * Removes and destructs element at the back of the array
         */
        template <class... Args>
        void Emplace(const size_t index, Args&&... args);

        /**
         * Copies an entire other vector into this vector, starting at provided index
         */
        void Insert(const size_t index, const Vector& other);

        /**
         * Grows vector to requrested new_capacity and copies any current data over
         * @param new_capacity The new size of the vector
         */
        void Resize(size_t new_capacity);

        /**
         * @return The number of elements actively stored in the array
         */
        size_t Size() const;

        /**
         * @return The number of elements the vector can currently contain
         */
        size_t Capacity() const;

        /**
         * Clear the contents of this vector
         */
        void Clear();

      private:
        /** Current size of the dynamic array */
        size_t m_Size;

        /** Current capacity of the dynamic array */
        size_t m_Capacity;

        /** Allocator used to service this vector */
        Allocator* m_Allocator;

        /** Pointer to the data buffer */
        T* m_Data;

      public:
        /**
         * Function to allow use in ranged based for loops
         */
        iterator begin();

        /**
         * Function to allow use in ranged based for loops
         */
        const_iterator begin() const;

        /**
         * Function to allow use in ranged based for loops
         */
        iterator end();

        /**
         * Function to allow use in ranged based for loops
         */
        const_iterator end() const;
    };

    template <class T>
    inline Vector<T>::Vector(Allocator* allocator, size_t capacity_hint)
        : m_Data(nullptr), m_Allocator(allocator), m_Size(0), m_Capacity(capacity_hint)
    {
        if (capacity_hint != 0) {
            m_Data = (T*)(m_Allocator->Allocate(sizeof(T) * capacity_hint, alignof(T)));
        }
    }

    template <class T>
    inline Vector<T>::Vector(Allocator* allocator, std::initializer_list<T> list)
        : Vector(allocator, list.size())
    {
        size_t i = 0;
        for (auto& element : list) {
            new (&m_Data[i]) T(element);
            i++;
        }

        m_Size = list.size();
    }

    template <class T>
    inline Vector<T>::Vector(const Vector<T>& other)
    {
        m_Allocator = other.m_Allocator;
        m_Data = nullptr;
        m_Size = 0;
        Resize(other.m_Capacity);

        size_t i = 0;
        for (auto& elem : other) {
            new (&m_Data[i]) T(elem);
            i++;
        }

        m_Size = other.Size();
    }

    template <class T>
    inline Vector<T>::Vector(Vector<T>&& other) noexcept
    {
        m_Allocator = other.m_Allocator;
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
        if (m_Data != nullptr) {
            m_Allocator->Free(m_Data);
        }
    }

    template <class T>
    inline Vector<T>& Vector<T>::operator=(const Vector<T>& other)
    {
        Clear();

        if (other.Size() > m_Capacity) {
            Resize(other.Size());
        }

        size_t i = 0;
        for (auto& element : other) {
            new (&m_Data[i]) T(element);
            i++;
        }

        m_Size = other.Size();

        return *this;
    }

    template <class T>
    inline Vector<T>& Vector<T>::operator=(Vector<T>&& other)
    {
        Clear();

        if (other.Size() > m_Capacity) {
            Resize(other.Size());
        }

        size_t i = 0;
        for (auto& element : other) {
            new (&m_Data[i]) T(std::move(element));
            i++;
        }

        m_Size = other.Size();

        return *this;
    }

    template <class T>
    inline Vector<T>& Vector<T>::operator=(std::initializer_list<T>&& list)
    {
        // Destroy all the elements currently in the list
        Clear();

        // Ensure the vector is large enough to fit the new list
        if (list.size() > m_Capacity) {
            Resize(list.size());
        }

        size_t i = 0;
        for (auto& element : list) {
            new (&m_Data[i]) T(std::move(element));
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
        if (m_Size >= m_Capacity) {
            // Array needs to grow
            size_t new_capacity = (m_Capacity != 0) ? m_Capacity * 2 : 1;
            Resize(new_capacity);
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
    inline void Vector<T>::Insert(const size_t index, const T& value)
    {
        SJ_ASSERT(index >= 0 && index <= m_Size, "Insertion index is invalid.");

        // If you're pushing into the final spot, just resort to PushBack
        if (index == m_Size) {
            PushBack(value);
            return;
        }

        if (m_Size < m_Capacity) {

            // Move last element into the end spot (end may be unitialized data)

            // Move every element to the right
            for (auto i = m_Size; i >= index; i--) {
                new (&m_Data[i]) T(m_Data[i - 1]);
            }

            // Insert value into buffer
            m_Data[index] = value;
        } else {
            // Array needs to grow, place new element in during buffer copy process

            // if size was zero, we'd have allready fallen back to PushBack
            size_t new_capacity = m_Capacity * 2;

            // Allocate a new buffer
            T* new_buffer = (T*)(m_Allocator->Allocate(sizeof(T) * new_capacity, alignof(T)));

            // Move old buffer into new buffer, up to index
            for (size_t i = 0; i < index; ++i) {
                new (&new_buffer[i]) T(m_Data[i]);
            }

            // Copy new element in to index position
            new (&new_buffer[index]) T(value);

            // Copy the rest of the old buffer into the new buffer
            for (auto i = index + 1; i < new_capacity; i++) {
                new (&new_buffer[i]) T(m_Data[i - 1]);
            }

            // Release old buffer
            if (m_Data != nullptr) {
                m_Allocator->Free(m_Data);
            }

            // Update capacity variable
            m_Capacity = new_capacity;

            // Allocate new buffer
            m_Data = new_buffer;
        }

        m_Size++;
    }

    template <class T>
    inline void Vector<T>::Insert(const size_t index, const Vector& other)
    {
        SJ_ASSERT(index >= 0 && index <= m_Size, "Insertion index is invalid.");

        size_t shift = other.Size();
        if (m_Capacity >= m_Size + other.Size()) {
            // There is enough space in the vector to insert other without reallocating

            // Move current data out of the way
            for (size_t i = m_Size - 1; i >= index; i--) {
                new (&m_Data[i + shift]) T(m_Data[i]);
            }

            // Move new data i
            for (size_t i = 0; i < other.Size(); i++) {
                new (&m_Data[i + index]) T(other[i]);
            }

            m_Size += other.Size();
        } else {
            // Allocate a new buffer with extra space
            size_t new_capacity = (m_Capacity + other.Capacity()) * 2;

            T* new_buffer = (T*)m_Allocator->Allocate(sizeof(T) * new_capacity, alignof(T));

            // Move old values into new buffer
            size_t i = 0;
            for (/***/; i < index; i++) {
                new (&new_buffer[i]) T(m_Data[i]);
            }

            // Copy new values into new buffer
            for (size_t j = 0; j < other.Size(); i++, j++) {
                new (&new_buffer[i]) T(other[j]);
            }

            // Copy remainder of old buffer into new buffer
            for (size_t k = index; k < m_Size; i++, k++) {
                new (&new_buffer[i]) T(m_Data[k]);
            }

            // Update container data
            if (m_Data != nullptr) {
                m_Allocator->Free(m_Data);
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
        if (index == m_Size) {
            EmplaceBack(std::forward<Args>(args)...);
            return;
        }

        if (m_Size < m_Capacity) {
            // Move every element to the right
            for (auto i = m_Size; i >= index; i--) {
                new (&m_Data[i]) T(std::move(m_Data[i - 1]));
            }

            // Insert value into buffer
            new (&m_Data[index]) T(std::forward<Args>(args)...);

        } else {
            // Array needs to grow, place new element in during buffer copy process

            // if size was zero, we'd have allready fallen back to PushBack
            size_t new_capacity = m_Capacity * 2;

            // Allocate a new buffer
            T* new_buffer = (T*)(m_Allocator->Allocate(sizeof(T) * new_capacity, alignof(T)));

            // Move old buffer into new buffer, up to index
            for (size_t i = 0; i < index; ++i) {
                new (&new_buffer[i]) T(std::move(m_Data[i]));
            }

            // Copy new element in to index position
            new (&new_buffer[index]) T(std::forward<Args>(args)...);

            // Copy the rest of the old buffer into the new buffer
            for (auto i = index + 1; i < new_capacity; i++) {
                new (&new_buffer[i]) T(std::move(m_Data[i - 1]));
            }

            // Release old buffer
            if (m_Data != nullptr) {
                m_Allocator->Free(m_Data);
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
        if (m_Size >= m_Capacity) {
            size_t new_capacity = (m_Capacity != 0) ? m_Capacity * 2 : 1;
            Resize(new_capacity);
        }

        // Construct element in allocated space
        auto index = m_Size;
        auto element = new (&m_Data[index]) T(std::forward<Args>(args)...);

        // Increment size of vector
        ++m_Size;

        return *element;
    }

    template <class T>
    inline void Vector<T>::Resize(size_t new_capacity)
    {
        if (new_capacity == m_Capacity) {
            return;
        }

        // Determine how many elements will be in the new vector
        size_t new_size = std::min(m_Size, new_capacity);

        if (new_size < m_Size) {
            // If the vector is shrinking, destroy the elements that aren't getting copied
            for (size_t i = new_size; i < m_Size; i++) {
                m_Data[i].~T();
            }
        }

        // Allocate a new buffer
        T* new_buffer = (T*)(m_Allocator->Allocate(sizeof(T) * new_capacity, alignof(T)));

        // Move old buffer into new buffer
        for (size_t i = 0; i < new_size; i++) {
            new (&new_buffer[i]) T(std::move(m_Data[i]));
        }

        // Update capacity variable
        m_Capacity = new_capacity;

        // Update size of vector
        m_Size = new_size;

        // Swap buffer pointers

        if (m_Data != nullptr) {
            m_Allocator->Free(m_Data);
        }

        m_Data = new_buffer;
    }

    template <class T>
    inline size_t Vector<T>::Size() const
    {
        return m_Size;
    }

    template <class T>
    inline size_t Vector<T>::Capacity() const
    {
        return m_Capacity;
    }

    template <class T>
    inline void Vector<T>::Clear()
    {
        for (auto& element : *this) {
            element.~T();
        }

        m_Size = 0;
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

} // namespace sj
