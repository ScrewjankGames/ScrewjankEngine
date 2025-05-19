#pragma once

// STD Headers
#include <type_traits>
#include <utility>
#include <cmath>

// Library Headers

// Screwjank Headers
#include <ScrewjankShared/utils/Assert.hpp>

import sj.engine.system.memory;
import sj.shared.containers;

namespace sj {

    constexpr float kDefaultMaxLoadFactor = 0.9f;
    constexpr size_t CalculateOptimalSize(size_t requestedSize,
                                          float loadFactor = kDefaultMaxLoadFactor)
    {
        const float size = requestedSize / loadFactor;
        const size_t intSize = static_cast<size_t>(size);

        return size == (float)intSize ? intSize : intSize + 1;
    }

    template<class T>
    struct Element
    {
        using offset_t = int8_t;
        
        /** Offset that indicates an element is empty */
        static constexpr offset_t kEmptyOffset = -1;

        /**
         * Constructor
         */
        Element();

        /**
         * Constructor
         * @param offset The offset of the element from it's desired index
         * @param value Object to copy in element memory
         */
        Element(offset_t offset) noexcept;

        /**
         * Constructor
         * @param offset The offset of the element from it's desired index
         * @param value Object to copy in element memory
         */
        Element(offset_t offset, T& value) noexcept;

        /**
         * Constructor
         * @param offset The offset of the element from it's desired index
         * @param value Object to move in element memory
         */
        Element(offset_t offset, T&& value) noexcept;

        /**
         * Copy Constructor
         */
        Element(const Element& other);

        /**
         * Move Constructor
         */
        Element(Element&& other) noexcept;

        /**
         * Destructor
         */

        ~Element() requires std::is_trivially_destructible_v<T> = default;
        ~Element()
            requires (!std::is_trivially_destructible_v<T>)
        {
            if(!IsEmpty())
            {
                Value.~T();
            }
        }


        /**
         * Copy assignment operator
         */
        Element& operator=(const Element& other);

        /**
         * Move assignment operator
         */
        Element& operator=(Element&& other);

        /**
         * @return true if Element has never contained a value, else false
         */
        bool IsUninitialized() const;

        /**
         * @return True if the element does not currently contain a record
         */
        bool IsEmpty() const;


        /**
         * @return True if the element currently contains a record
         */
        bool HasValue() const;

        /** Element's distance from it's desired hash slot */
        offset_t Offset = kEmptyOffset;

        /** Nameless union to prevent premature initialization of value by Vector */
        union
        {
            T Value;
        };
    };

    /**
     * Iterator class to walk over all elements of UnorderedSet
     * @note SetIterators do not allow non-const access.
     */
    template <class Set_t>
    class SetIterator_t
    {
      public:
        using iterator_category = std::forward_iterator_tag;

        using value_type = const Set_t::value_type;
        using element_type = const Set_t::element_type;
        using pointer = typename Set_t::const_pointer;
        using const_pointer = typename Set_t::const_pointer;
        using element_pointer = element_type*;
        using reference = typename Set_t::const_reference;
        using const_reference = typename Set_t::const_reference;
        using difference_type = typename Set_t::difference_type;

      public:
        /**
         * Constructor
         */
        SetIterator_t(const Set_t* set, element_pointer element);

        /**
         * Dereference operator overload
         */
        [[nodiscard]] const_reference operator*() const;

        /**
         * Arrow operator overload
         */
        [[nodiscard]] const_pointer operator->() const;

        /**
         * Equality comparison operator
         */
        bool operator==(const SetIterator_t& other) const;

        /**
         * Inequality comparison operator
         */
        bool operator!=(const SetIterator_t& other) const;

        /**
         * Pre-increment operator overload
         */
        SetIterator_t& operator++();

        /**
         * Post-increment operator overload
         */
        SetIterator_t operator++(int);

      private:
        /** The element currently pointer at by this iterator */
        element_pointer m_currElement;
        
        const Set_t* m_set;
    };

    /**
     * Open-addressed robinhood hashed set
     */
    template <class T, class IMPL, class Hasher = std::hash<T>>
    class unordered_set_base
    {
      public:
        // Type aliases
        using key_type = T;
        using value_type = T;
        using size_type = size_t;
        using difference_type = std::ptrdiff_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;

        // Iterator definitions
        using const_iterator = SetIterator_t<const unordered_set_base<T, IMPL, Hasher>>;
        using iterator = const_iterator;
        using element_type = Element<T>;

        using offset_t = element_type::offset_t;

      public:

        /**
         * Default Constructor
         */
        unordered_set_base() = default;

        /**
         * Copy Constructor
         */
        unordered_set_base(const unordered_set_base& other) = default;

        /**
         * Move Constructor
         */
        unordered_set_base(unordered_set_base&& other) = default;

        /**
         * Copy Assignment operator
         */
        unordered_set_base<T, IMPL, Hasher>& operator=(const unordered_set_base& other) = default;

        /**
         * Move Assignment operator
         */
        unordered_set_base <T, IMPL, Hasher>& operator=(unordered_set_base&& other) = default;

        /**
         * Assignment from initializer list
         */
        unordered_set_base<T, IMPL, Hasher>& operator=(std::initializer_list<T> list);

        /**
         * Equality comparison operator.
         * @note: First sizes are compared. If equal, every element in the first set is searched for
         * in the other
         */
        bool operator==(const unordered_set_base<T, IMPL, Hasher>& other) const;

        /**
         * Looks up supplied key in set
         * @return iterator to element if found, else end()
         */
        [[nodiscard]] const_iterator find(const T& key) const;

        /**
         * Find element given it's hash
         */
        [[nodiscard]] const_iterator find_from_hash(size_t hash, const T& key) const;

        /**
         * @return True if key is in set, else false
         */
        bool contains(const T& key) const;

        /**
         * Insert an element into the set
         * @return An iterator to the element inserted
         */
        std::pair<iterator, bool> insert(const T& key);

        /**
         * Range-based insert  
         */
        template <class InputIterator>
        void insert(InputIterator first, InputIterator last);

        /**
         * Range-based emplace 
         */
        template <class InputIterator>
        void emplace(InputIterator first, InputIterator last);

        /**
         * Emplace an element into the set
         * @return An iterator to the element inserted
         */
        template <class... Args>
        std::pair<iterator, bool> emplace(Args&&... args);

        /**
         * Erase an element from the set
         */
        bool erase(const T& key);

        /**
         * Erase all elements in the set
         */
        void clear();

        /**
         * Rehash set to contain num_buckets buckets
         */
        void rehash(size_t capacity);

        /**
         * Returns the number of elements stored in the set
         */
        size_t count() const;

        /**
         * Returns the number of elements this Set can hold
         */
        size_t capacity() const;

        /**
         * Ranged-based for loop compatabile iterator to begining of set
         */
        const_iterator begin() const;

        /**
         * Ranged-based for loop compatabile iterator to end of set
         */
        const_iterator end() const;

      protected:

        /** Global maximum for how far an element can be from it's desired position */
        static constexpr offset_t kProbeLimit = std::numeric_limits<offset_t>::max();

      private:
        /**
         * Insert the value with the current offset at rick_index, and re-insert the rich element
         * @param poor_record The element that will be replacing the element at rich_index
         * @param rich_index The index of the set that will be stolen from and re-inserted
         */
        void RobinHoodInsert(element_type&& poor_record, size_t rich_index);

        void Backshift(size_type erasedIndex);

        auto& GetElements()
        {
            return static_cast<IMPL*>(this)->m_Elements;
        }

        const auto& GetElements() const
        {
            return static_cast<const IMPL*>(this)->m_Elements;
        }

        auto& GetCount()
        {
            return static_cast<IMPL*>(this)->m_Count;
        }

        const auto& GetCount() const
        {
            return static_cast<const IMPL*>(this)->m_Count;
        }

        /** Functor used to hash keys */
        Hasher m_HashFunctor = {};

        /** The maximum load factor of the set */
        static constexpr float s_MaxLoadFactor = kDefaultMaxLoadFactor;

    };

    template <class T,
              size_t tRequestedSize,
              size_t tRealSize = CalculateOptimalSize(tRequestedSize)>
    class static_unordered_set
        : public unordered_set_base<T, static_unordered_set<T, tRequestedSize, tRealSize>>
    {
        using Base = unordered_set_base<T, static_unordered_set<T, tRequestedSize, tRealSize>>;
        friend Base;

    public:
        static_unordered_set() = default;

        template <class InputIterator>
        static_unordered_set(InputIterator first, InputIterator last);
        
        using Base::operator=;
        
    private:
        friend Base;
        static constexpr bool kIsGrowable = false;

        /** Number of elements in the set */
        size_t m_Count = 0;
        std::array<Element<T>, tRealSize> m_Elements;
    };

    template<class T>
    class dynamic_unordered_set : public unordered_set_base<T, dynamic_unordered_set<T>>
    {
        using Base = unordered_set_base<T, dynamic_unordered_set<T>>;
        friend Base;
    public:
        /**
         * Implicit MemSpace Constructors
         * Uses MemorySystem::CurrentMemSpace to allocate memory
         */
        dynamic_unordered_set();

        dynamic_unordered_set(const dynamic_unordered_set<T>& other);
        dynamic_unordered_set(dynamic_unordered_set<T>&& other);

        dynamic_unordered_set(std::initializer_list<T> list);

        template <class InputIterator>
        dynamic_unordered_set(InputIterator first, InputIterator last);
     
        /**
         * Default constructor
         */
        dynamic_unordered_set(sj::memory_resource* mem_resource);

        /**
         * List Initialization Constructor
         */
        dynamic_unordered_set(sj::memory_resource* mem_resource, std::initializer_list<T> list);

        /**
         * Range Constructor
         */
        template <class InputIterator>
        dynamic_unordered_set(sj::memory_resource* mem_resource, InputIterator first, InputIterator last);

        using Base::operator=;

    private:
        friend Base;
        static constexpr bool kIsGrowable = true;
        
        /** Number of elements in the set */
        size_t m_Count = 0;

        dynamic_array<Element<T>, size_t> m_Elements;
    };

} // namespace sj

// Include inlines
#include <ScrewjankEngine/containers/UnorderedSet.inl>
