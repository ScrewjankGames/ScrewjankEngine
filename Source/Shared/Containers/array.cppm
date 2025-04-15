module;
// Shared Includes
#include <ScrewjankShared/utils/Assert.hpp>

// STD Includes
#include <cstddef>
#include <iterator>
#include <memory>

export module sj.shared.containers:array;

export namespace sj
{
    template <class T, class size_type=uint32_t, class Allocator = std::pmr::polymorphic_allocator<T>>
    class dynamic_array
    {
    public:
        // STL type aliases
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator_concept = std::contiguous_iterator_tag;

        dynamic_array() = default;

        dynamic_array(const Allocator& alloc)
            : m_allocator(alloc)
        {

        }

        dynamic_array(size_type capacity, const Allocator& alloc = Allocator())
            : m_allocator(alloc)
        {
            resize(capacity);
        }

        dynamic_array(size_type capacity, const value_type& defaultValue, const Allocator& alloc = Allocator())
            : m_allocator(alloc)
        {
            resize(capacity);
            for(value_type& value : *this)
            {
                value = defaultValue;
            }
        }

        dynamic_array(std::initializer_list<T> elems) 
            : m_allocator(Allocator()),
              m_capacity(elems.size())
        {
            m_array = m_allocator.allocate(m_capacity);

            for(size_type i = 0; const T& elem : elems)
            {
                new(std::addressof(m_array[i])) T(elem);
                i++;
            }
        }

        /**
         * Copy Constructor 
         */
        dynamic_array(const dynamic_array& other)
            : dynamic_array(
                other.m_capacity, 
                std::allocator_traits<Allocator>::select_on_container_copy_construction(other.m_allocator)
            )
        {
            if constexpr (std::is_trivially_copy_constructible_v<T>)
            {
                std::memcpy(m_array, other.m_array, sizeof(T) * m_capacity);
            }
            else
            {
                int i = 0;
                for(const T& elem : other)
                {
                    m_array[i] = elem;
                    i++;
                }
            }
        }

        /**
         * Move Constructor 
         */
        dynamic_array(dynamic_array&& other)
            : m_allocator(std::move(other.m_allocator)), m_array(other.m_array), m_capacity(other.m_capacity)
        {
            other.m_array = nullptr;
            other.m_capacity = 0;
        }

        /**
         * Destructor 
         */
        ~dynamic_array()
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                for(auto& it : *this)
                {
                    it.~T();
                }
            }

            if(m_array != nullptr)
                m_allocator.deallocate(m_array, m_capacity);
        }

        /**
         * Array index operator
         * @note Complexity is O(1)
         * @return reference to element at spot index in array
         */
        auto&& operator[](this auto&& self, size_type index)
        {
            return self.m_array[index];
        }

        /**
         * Inequality comparison operator
         */
        friend bool operator==(const dynamic_array& lhs,
                               const dynamic_array& rhs)
        {
            if(lhs.m_capacity != rhs.m_capacity)
            {
                return false;
            }

            for(size_type i = 0; i < lhs.m_capacity; i++)
            {
                if(lhs[i] != rhs[i])
                {
                    return false;
                }
            }

            return true;
        }

        friend bool operator!=(const dynamic_array<T, size_type>& lhs,
                                      const dynamic_array<T, size_type>& rhs)
        {
            return !(lhs == rhs);
        }

        /**
         * Bounds-checked element index operator
         */
        auto&& at(this auto&& self, size_type index) // -> (const?) T&
        {
            SJ_ASSERT(index >= 0 && index < self.m_capacity, "dynamic_array out of bounds!");
            return self.m_array[index];
        }

        /**
         * Returns reference to element at index 0
         */
        auto&& front(this auto&& self) // -> (const?) T&
        {
            return self.m_array[0];
        }

        /**
         * Returns reference to last index of array
         */
        auto&& back(this auto&& self) // -> (const?) T&
        {
            return self.m_array[self.m_capacity - 1];
        }

        /**
         * @return Size of the array
         */
        size_type size() const
        {
            return m_capacity;
        }

        /**
         * Returns pointer to backing raw C array, allows modification of elements
         */
        auto&& data(this auto&& self) // -> (const?) T*
        {
            return self.m_array;
        }

        /**
         * Function to allow use in ranged based for loops
         */
        auto begin(this auto&& self) // -> (const?) T*
        {
            return self.m_array;
        }

        auto end(this auto&& self) // -> (const?) T*
        {
            return self.m_array + self.m_capacity;
        }

        void resize(size_type newCapacity)
        {
            T* newArray = nullptr;
            if(newCapacity > 0)
            {
                newArray = m_allocator.allocate(newCapacity);
                
                // Move over everything that will fit
                const size_type numRetainedElements = std::min(m_capacity, newCapacity);
                for(int i = 0; i < numRetainedElements; i++)
                {
                    new (std::addressof(newArray[i])) T(std::move(m_array[i]));
                }

                if(newCapacity > m_capacity)
                {
                    for(int i = m_capacity; i < newCapacity; i++)
                        new (std::addressof(newArray[i])) T();
                }
            }

            if(m_array)
            {
                if constexpr(!std::is_trivially_destructible_v<T>)
                {
                    // Destroy anything that doesn't fit ( when newCapacity < m_capacity)
                    for(size_type i = newCapacity; i < m_capacity; i++)
                    {
                        m_array[i].~T();
                    }
                }

                m_allocator.deallocate(m_array, m_capacity);
            }

            m_array = newArray;
            m_capacity = newCapacity;
        }

    private:
        Allocator m_allocator = Allocator();

        T* m_array = nullptr;
        size_type m_capacity = 0;
    };
}