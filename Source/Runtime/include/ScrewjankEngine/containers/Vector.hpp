#pragma once

// STD Headers
#include <compare>
#include <utility>

// Library Headers

// Scewjank Headers
#include <ScrewjankEngine/containers/Array.hpp>
#include <ScrewjankShared/utils/Assert.hpp>
#include <ScrewjankEngine/utils/Log.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>

namespace sj {

    template <class T>
    class dynamic_vector
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
        using iterator = T*;
        using const_iterator = const T*;

        /**
         * Implicit MemSpace Constructors
         * Uses MemorySystem::CurrentMemSpace to allocate memory
         */
        dynamic_vector();
        dynamic_vector(size_t count);
        dynamic_vector(size_t count, const T& value);
        dynamic_vector(std::initializer_list<T> list);
        dynamic_vector(T* buffer, size_t count);

        /**
         * Default Constructor
         * @param count The number of elements to default construct in this vector
         */
        explicit dynamic_vector(IMemSpace* mem_space);

        /**
         * Value Initialization Constructor
         * @param count The number of elements to default construct in this vector
         */
        dynamic_vector(IMemSpace* mem_space, size_t count);

        /**
         * Value Initialization Constructor
         * @param count The number of elements to construct in this vector
         * @param value The value to use when initializing constructed elements
         */
        dynamic_vector(IMemSpace* mem_space, size_t count, const T& value);

        /**
         * List initialization Constructor
         */
        dynamic_vector(IMemSpace* mem_space, std::initializer_list<T> list);

        /**
         * Copy Constructor
         */
        dynamic_vector(const dynamic_vector<T>& other);

        /**
         * Move Constructor
         */
        dynamic_vector(dynamic_vector<T>&& other) noexcept;

        /**
         * Destructor
         */
        ~dynamic_vector();

        /**
         * Copy assignment from other vector
         */
        dynamic_vector<T>& operator=(const dynamic_vector<T>& other);

        /**
         * Move assignment from other vector
         */
        dynamic_vector<T>& operator=(dynamic_vector<T>&& other) noexcept;

        /**
         * Assignment from initializer list
         */
        dynamic_vector<T>& operator=(std::initializer_list<T> list);

        /** Array Index Operator */
        T& operator[](const size_t index);

        /** Array Index Operator */
        const T& operator[](const size_t index) const;

        auto&& at(this auto&& self, size_t index);

        /**
         * @return The first element in the vector
         */
        auto&& front(this auto&& self);

        /**
         * @return The last element in the vector
         */
        auto&& back(this auto&& self);

        /**
         * Inserts a new element onto the end of the array
         * @note Iterators are invalidated if the array is resized
         */
        void push_back(const T& value);

        /**
         * Pushes another vector onto the back of this vector
         */
        void push_back(const dynamic_vector<T>& other);

        /**
         * Constructs an element into the array
         * @return A reference to the element inserted
         */
        template <class... Args>
        T& emplace_back(Args&&... args);

        /**
         * Removes and destructs element at the back of the array
         */
        void pop_back();

        /**
         * Removes and destructs element at the back of the array
         */
        iterator insert(const size_t index, const T& value);

        /**
         * Copies an entire other vector into this vector, starting at provided index
         */
        void insert(const size_t index, const dynamic_vector& other);

        /**
         * Removes and destructs element at the back of the array
         */
        template <class... Args>
        void emplace(const size_t index, Args&&... args);

        /**
         * Removes an element from the vector
         * @param pos An iterator to the elment to be erased
         * @return An iterator to the next element after pos
         */
        iterator erase(iterator pos);

        /**
         * @return The number of elements actively stored in the array
         */
        size_t size() const;

        /**
         * Grows vector to requrested new_capacity, copies current data over, and default
         * initializes empty indices
         * @param new_size The new size of the vector
         */
        void resize(size_t new_size);

        /**
         * Grows vector to requrested new_capacity, copies current data over, and default
         * initializes empty indices
         * @param new_size The new size of the vector
         * @param value The value to initialize newly added elements with
         */
        void resize(size_t new_size, const T& value);

        /**
         * Changes the capacity of the vector without affecting Vector.size()
         * @param new_capacity The new number of elements you would like the vector to have
         */
        void reserve(size_t new_capacity);

        /**
         * @return The number of elements the vector can currently contain
         */
        size_t capacity() const;

        /**
         * Clear the contents of this vector
         */
        void clear();

        /**
         * @return Whether or not the vector currently contains elements  
         */
        bool empty();

        /**
         * Allows access to the raw C-Style array
         */
        auto data(this auto&& self); // -> (const?) T*

        /**
         * Function to allow use in ranged based for loops
         */
        auto begin(this auto&& self); // -> (const?) T*
        auto end(this auto&& self); // -> (const?) T*

      private:
        /** Current size of the dynamic array */
        size_t m_Size;

        /** Current capacity of the dynamic array, not including the sentinel element */
        size_t m_Capacity;

        /** Allocator used to service this vector */
        IMemSpace* m_BackingZone;

        /** Pointer to the data buffer */
        T* m_Data;

    };

} // namespace sj

// Include inlines
#include <ScrewjankEngine/containers/Vector.inl>