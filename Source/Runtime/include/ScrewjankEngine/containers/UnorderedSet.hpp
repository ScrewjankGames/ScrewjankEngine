#pragma once

// STD Headers
#include <type_traits>
#include <utility>

// Library Headers

// Screwjank Headers
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/containers/Vector.hpp>

namespace sj {
    /**
     * Iterator class to walk over all elements of UnorderedSet
     * @note SetIterators do not allow non-const access.
     */
    template <class Set_t>
    class SetIterator_t
    {
      public:
        using iterator_category = std::forward_iterator_tag;

        using value_type = typename const Set_t::value_type;
        using element_type = typename const Set_t::element_type;
        using pointer = typename Set_t::const_pointer;
        using const_pointer = typename Set_t::const_pointer;
        using element_pointer = typename element_type*;
        using reference = typename Set_t::const_reference;
        using const_reference = typename Set_t::const_reference;
        using difference_type = typename Set_t::difference_type;

      public:
        /**
         * Constructor
         */
        SetIterator_t(element_pointer element);

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
        element_pointer m_CurrElement;
    };

    /**
     * Open-addressed robinhood hashed set
     */
    template <class T, class Hasher = std::hash<T>>
    class UnorderedSet
    {
        struct Element;

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
        using offset_t = int8_t;

        // Iterator definitions
        using const_iterator = typename SetIterator_t<const UnorderedSet<T, Hasher>>;
        using iterator = const_iterator;
        using element_type = Element;

      public:
        /**
         * Implicit HeapZone Constructors
         * Uses MemorySystem::CurrentHeapZone to allocate memory
         */
        UnorderedSet();
        UnorderedSet(std::initializer_list<T> list);
        template <class InputIterator>
        UnorderedSet(InputIterator first, InputIterator last);

        /**
         * Default constructor
         */
        UnorderedSet(HeapZone* heap_zone);

        /**
         * List Initialization Constructor
         */
        UnorderedSet(HeapZone* heap_zone, std::initializer_list<T> list);

        /**
         * Range Constructor
         */
        template <class InputIterator>
        UnorderedSet(HeapZone* heap_zone, InputIterator first, InputIterator last);

        /**
         * Copy Constructor
         */
        UnorderedSet(UnorderedSet& other) = default;

        /**
         * Move Constructor
         */
        UnorderedSet(UnorderedSet&& other) noexcept;

        /**
         * Move Assignment operator
         */
        UnorderedSet<T>& operator=(UnorderedSet&& other);

        /**
         * Assignment from initializer list
         */
        UnorderedSet<T>& operator=(std::initializer_list<T> list);

        /**
         * Equality comparison operator.
         * @note: First sizes are compared. If equal, every element in the first set is searched for
         * in the other
         */
        bool operator==(const UnorderedSet<T>& other) const;

        /**
         * Looks up supplied key in set
         * @return iterator to element if found, else end()
         */
        [[nodiscard]] const_iterator Find(const T& key) const;

        /**
         * @return True if key is in set, else false
         */
        bool Contains(const T& key) const;

        /**
         * Insert an element into the set
         * @return An iterator to the element inserted
         */
        std::pair<iterator, bool> Insert(const T& key);

        /**
         * Range-based insert  
         */
        template <class InputIterator>
        void Insert(InputIterator first, InputIterator last);

        /**
         * Range-based emplace 
         */
        template <class InputIterator>
        void Emplace(InputIterator first, InputIterator last);

        /**
         * Emplace an element into the set
         * @return An iterator to the element inserted
         */
        template <class... Args>
        std::pair<iterator, bool> Emplace(Args&&... args);

        /**
         * Erase an element from the set
         */
        bool Erase(const T& key);

        /**
         * Erase all elements in the set
         */
        void Clear();

        /**
         * Rehash set to contain num_buckets buckets
         */
        void Rehash(size_t capacity);

        /**
         * Returns the number of elements stored in the set
         */
        size_t Count() const;

        /**
         * Returns the number of elements this Set can hold
         * @note This figure is one less than the capacity of the underlying vector due to the
         * sentinel value
         */
        size_t Capacity() const;

      private:
        /** Global maximum for how far an element can be from it's desired position */
        static constexpr offset_t kProbeLimit = std::numeric_limits<offset_t>::max();

        /** Offset that indicates an element is empty */
        static constexpr offset_t kEmptyOffset = -1;

        /** Flag to indicate a cell was erased so later lookups can probe correctly */
        static constexpr offset_t kTombstoneOffset = -2;

        /** Flag to indicate a cell is reserved and marks the end of the set for iterators */
        static constexpr offset_t kEndOfSetSentinel = -3;

        struct Element
        {
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
            ~Element();

            /**
             * Copy assignment operator
             */
            Element& operator=(Element& other);

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
             * @return True if the element represents the end of the set
             */
            bool IsSentinel() const;

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

        /** The Sentinel Element is used to detect the end of the list */
        static inline Element s_SentinelElement = Element(kEndOfSetSentinel);

        /** Functor used to hash keys */
        Hasher m_HashFunctor;

        /** Entries of the hash set */
        dynamic_vector<Element> m_Elements;

        /** Mask used to assign hashed keys to buckets */
        size_t m_IndexMask;

        /** Number of elements in the set */
        size_t m_Count;

        /** The maximum load factor of the set */
        float m_MaxLoadFactor;

        /**
         * Insert the value with the current offset at rick_index, and re-insert the rich element
         * @param poor_record The element that will be replacing the element at rich_index
         * @param rich_index The index of the set that will be stolen from and re-inserted
         */
        void RobinHoodInsert(Element&& poor_record, size_t rich_index);

      public:
        /**
         * Ranged-based for loop compatabile iterator to begining of set
         */
        const_iterator begin() const;

        /**
         * Ranged-based for loop compatabile iterator to end of set
         */
        const_iterator end() const;
    };
} // namespace sj

// Include inlines
#include <ScrewjankEngine/containers/UnorderedSet.inl>
