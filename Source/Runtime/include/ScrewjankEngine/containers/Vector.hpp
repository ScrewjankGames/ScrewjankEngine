#pragma once

// STD Headers
#include <compare>
#include <utility>

// Library Headers

// Scewjank Headers
#include <ScrewjankEngine/containers/Array.hpp>
#include <ScrewjankEngine/core/Assert.hpp>
#include <ScrewjankEngine/core/Log.hpp>
#include <ScrewjankEngine/system/Memory.hpp>

namespace sj {

    /**
     * Class to define both const and non-const iterators over vectors
     *  @tparam Vector_t The cv-qualified type of vector being iterated over
     */
    template <class Vector_t>
    class VectorIteratorBase
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
        VectorIteratorBase(pointer element) noexcept : m_CurrElement(element)
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
        bool operator==(const VectorIteratorBase& other) const
        {
            return m_CurrElement == other.m_CurrElement;
        }

        /** Inequality comparison operator */
        bool operator!=(const VectorIteratorBase& other) const
        {
            return !(*this == other);
        }

        auto operator<=>(const VectorIteratorBase& other) const = default;

        /** Addition operator overload */
        VectorIteratorBase operator+(size_t num) const
        {
            return VectorIteratorBase(m_CurrElement + num);
        }

        /** Addition operator overload */
        VectorIteratorBase operator-(size_t num) const
        {
            return VectorIteratorBase(m_CurrElement - num);
        }

        /** Pre-increment operator overload */
        VectorIteratorBase& operator++()
        {
            ++m_CurrElement;
            return *this;
        }

        /** Post-increment operator overload */
        VectorIteratorBase operator++(int)
        {
            VectorIteratorBase tmp(*this);
            this->operator++();
            return tmp;
        }

        /** Pre-decrement operator overload */
        VectorIteratorBase& operator--()
        {
            --m_CurrElement;
            return *this;
        }

        /** Post-decrement operator overload */
        VectorIteratorBase& operator--(int)
        {
            VectorIteratorBase tmp(*this);
            --*this;
            return tmp;
        }

        /** Compound assignment operator overload */
        VectorIteratorBase& operator+=(size_t val)
        {
            m_CurrElement += val;
            return *this;
        }

        /** Compound assignment operator overload */
        VectorIteratorBase& operator-=(size_t val)
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
        using iterator = typename VectorIteratorBase<Vector<T>>;
        using const_iterator = typename VectorIteratorBase<const Vector<T>>;

        
        /**
         * Implicit HeapZone Constructors
         * Uses MemorySystem::CurrentHeapZone to allocate memory
         */
        Vector();
        Vector(size_t count);
        Vector(size_t count, const T& value);
        Vector(std::initializer_list<T> list);

        /**
         * Default Constructor
         * @param count The number of elements to default construct in this vector
         */
        explicit Vector(HeapZone* heap_zone);

        /**
         * Value Initialization Constructor
         * @param count The number of elements to default construct in this vector
         */
        Vector(HeapZone* heap_zone, size_t count);

        /**
         * Value Initialization Constructor
         * @param count The number of elements to construct in this vector
         * @param value The value to use when initializing constructed elements
         */
        Vector(HeapZone* heap_zone, size_t count, const T& value);

        /**
         * List initialization Constructor
         */
        Vector(HeapZone* heap_zone, std::initializer_list<T> list);

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
         * Copy assignment from other vector
         */
        Vector<T>& operator=(const Vector<T>& other);

        /**
         * Move assignment from other vector
         */
        Vector<T>& operator=(Vector<T>&& other) noexcept;

        /**
         * Assignment from initializer list
         */
        Vector<T>& operator=(std::initializer_list<T> list);

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
        iterator Insert(const size_t index, const T& value);

        /**
         * Copies an entire other vector into this vector, starting at provided index
         */
        void Insert(const size_t index, const Vector& other);

        /**
         * Removes and destructs element at the back of the array
         */
        template <class... Args>
        void Emplace(const size_t index, Args&&... args);

        /**
         * Removes an element from the vector
         * @param pos An iterator to the elment to be erased
         * @return An iterator to the next element after pos
         */
        iterator Erase(iterator pos);

        /**
         * @return The number of elements actively stored in the array
         */
        size_t Size() const;

        /**
         * Grows vector to requrested new_capacity, copies current data over, and default
         * initializes empty indices
         * @param new_size The new size of the vector
         */
        void Resize(size_t new_size);

        /**
         * Grows vector to requrested new_capacity, copies current data over, and default
         * initializes empty indices
         * @param new_size The new size of the vector
         * @param value The value to initialize newly added elements with
         */
        void Resize(size_t new_size, const T& value);

        /**
         * Changes the capacity of the vector without affecting Vector.Size()
         * @param new_capacity The new number of elements you would like the vector to have
         */
        void Reserve(size_t new_capacity);

        /**
         * @return The number of elements the vector can currently contain
         */
        size_t Capacity() const;

        /**
         * Clear the contents of this vector
         */
        void Clear();

        /**
         * @return Whether or not the vector currently contains elements  
         */
        bool Empty();

        /**
         * Allows access to the raw C-Style array
         */
        T* Data();

      private:
        /** Current size of the dynamic array */
        size_t m_Size;

        /** Current capacity of the dynamic array, not including the sentinel element */
        size_t m_Capacity;

        /** Allocator used to service this vector */
        HeapZone* m_BackingZone;

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

} // namespace sj

// Include inlines
#include <ScrewjankEngine/containers/Vector.inl>