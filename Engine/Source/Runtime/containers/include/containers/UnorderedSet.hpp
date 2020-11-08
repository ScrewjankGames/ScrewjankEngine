#pragma once

// STD Headers
#include <optional>
#include <type_traits>
#include <utility>

// Library Headers

// Screwjank Headers
#include "system/Memory.hpp"
#include "containers/Vector.hpp"

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
         * Default constructor
         */
        UnorderedSet(Allocator* allocator = MemorySystem::GetDefaultAllocator());

        /**
         * List Initialization Constructor
         */
        UnorderedSet(Allocator* allocator, std::initializer_list<T> list);

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
        const_iterator Find(const T& key) const;

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
             * @param args... Arguments to be forwarded to value_type's constructor
             */
            template <class... Args>
            Element(offset_t offset, Args&&... args) noexcept;

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
        static inline Element s_SentinelElement = {kEndOfSetSentinel};

        /** Functor used to hash keys */
        Hasher m_HashFunctor;

        /** Entries of the hash set */
        Vector<Element> m_Elements;

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

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// UnorderedSet Implementation
    ///////////////////////////////////////////////////////////////////////////////////////////////

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::UnorderedSet(Allocator* allocator)
        : m_HashFunctor(), m_Elements(allocator, 2, Element {s_SentinelElement}), m_IndexMask(0),
          m_Count(0), m_MaxLoadFactor(0.9f)
    {
    }

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::UnorderedSet(Allocator* allocator,
                                                 std::initializer_list<T> list)
        : UnorderedSet(allocator)
    {
        for (auto key : list) {
            Insert(key);
        }
    }

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::UnorderedSet(UnorderedSet<T, Hasher>&& other) noexcept
        : m_Elements(std::move(other.m_Elements))
    {
        other.m_Elements.Resize(2, Element {s_SentinelElement});
        m_HashFunctor = other.m_HashFunctor;

        m_IndexMask = other.m_IndexMask;
        other.m_IndexMask = 0;

        m_Count = other.Count();
        other.m_Count = 0;

        m_MaxLoadFactor = other.m_MaxLoadFactor;
    }

    template <class T, class Hasher>
    inline UnorderedSet<T>& UnorderedSet<T, Hasher>::operator=(UnorderedSet&& other)
    {
        m_Elements = std::move(other.m_Elements);
        other.m_Elements.Resize(2, Element {s_SentinelElement});

        m_HashFunctor = other.m_HashFunctor;

        m_IndexMask = other.m_IndexMask;
        other.m_IndexMask = 0;

        m_Count = other.Count();
        other.m_Count = 0;

        m_MaxLoadFactor = other.m_MaxLoadFactor;
    }

    template <class T, class Hasher>
    inline UnorderedSet<T>& UnorderedSet<T, Hasher>::operator=(std::initializer_list<T> list)
    {
        Clear();
        for (auto& key : list) {
            Insert(key);
        }

        return *this;
    }

    template <class T, class Hasher>
    inline bool UnorderedSet<T, Hasher>::operator==(const UnorderedSet<T>& other) const
    {
        if (m_Count != other.Count()) {
            return false;
        }

        // Look up each element in this set in the other set
        for (auto& element : *this) {
            if (!other.Contains(element)) {
                return false;
            }
        }

        return true;
    }

    template <class T, class Hasher>
    inline typename UnorderedSet<T, Hasher>::const_iterator
    UnorderedSet<T, Hasher>::Find(const T& key) const
    {
        auto hash = m_HashFunctor(key);
        auto desired_index = hash & m_IndexMask;

        for (auto [index, curr_offset] = std::tuple {desired_index, offset_t(0)};
             m_Elements[index].Offset >= curr_offset;
             index = (index + 1) & m_IndexMask, curr_offset++) {

            if (key == m_Elements[index].Value) {
                return const_iterator(&m_Elements[index]);
            }
        }

        return end();
    }

    template <class T, class Hasher>
    inline bool UnorderedSet<T, Hasher>::Contains(const T& key) const
    {
        return Find(key) != end();
    }

    template <class T, class Hasher>
    inline std::pair<typename UnorderedSet<T, Hasher>::iterator, bool>
    UnorderedSet<T, Hasher>::Insert(const T& value)
    {
        return Emplace(value);
    }

    template <class T, class Hasher>
    template <class... Args>
    inline std::pair<typename UnorderedSet<T, Hasher>::iterator, bool>
    UnorderedSet<T, Hasher>::Emplace(Args&&... args)
    {
        // construct a new record on the stack
        Element new_record(0, std::forward<Args>(args)...);
        auto hash = m_HashFunctor(new_record.Value);
        auto desired_index = hash & m_IndexMask;

        // Ensure supplied key is not in the Set (same as the Find() operation)
        size_t search_start_index = desired_index;

        for (auto [index, curr_offset] = std::tuple {desired_index, offset_t(0)};
             m_Elements[index].Offset >= curr_offset;
             index = (index + 1) & m_IndexMask, curr_offset++) {

            if (new_record.Value == m_Elements[index].Value) {
                return {const_iterator(&m_Elements[index]), false};
            }
        }

        // The new key is unique, try to insert it
        auto new_count = m_Count + 1;

        // If there's not enough space, rehash
        if ((new_count / Capacity()) > m_MaxLoadFactor || new_count > Capacity()) {
            Rehash(Capacity() * 2);

            // Recompute desired index, rehashing changes index mask
            desired_index = hash & m_IndexMask;
        }

        // Perform the robin hood insertion, with a maximum probe count of kProbeLimit
        for (size_t index = desired_index; new_record.Offset < kProbeLimit;
             index = (index + 1) & m_IndexMask, new_record.Offset++) {

            // If we've found an open entry, take it
            if (m_Elements[index].IsEmpty()) {
                m_Elements[index] = std::move(new_record);
                m_Count++;
                return {const_iterator(&m_Elements[index]), true};
            } else if (m_Elements[index].Offset < new_record.Offset) {
                RobinHoodInsert(std::move(new_record), index);
                return {const_iterator(&m_Elements[index]), true};
            }
        }

        SJ_ASSERT(false,
                  "UnorderedSet insertion failed due to excessive collision count (>= {}!). "
                  "Check your "
                  "hash function.",
                  kProbeLimit);

        return {nullptr, false};
    }

    template <class T, class Hasher>
    inline bool UnorderedSet<T, Hasher>::Erase(const T& key)
    {
        auto hash = m_HashFunctor(key);
        auto start_index = hash & m_IndexMask;

        for (auto [index, offset] = std::tuple {start_index, offset_t(0)};
             m_Elements[index].Offset >= offset;
             index = (index + 1) & m_IndexMask, offset++) {
            if (m_Elements[index].Value == key) {
                m_Count--;
                m_Elements[index].Offset = kTombstoneOffset;
                return true;
            }
        }

        return false;
    }

    template <class T, class Hasher>
    inline void UnorderedSet<T, Hasher>::Clear()
    {
        m_Elements.Clear();
        m_Count = 0;
    }

    template <class T, class Hasher>
    inline void UnorderedSet<T, Hasher>::Rehash(size_t new_capacity)
    {
        SJ_ASSERT((new_capacity & (new_capacity - 1)) == 0,
                  "UnorderedSet capacity must be a power of two!");

        if (new_capacity <= Capacity()) {
            return;
        }

        // Move the old element set into a temporary
        Vector<Element> old_set(std::move(m_Elements));

        // Reserve enough space for the new capacity, plus one slot for the sentinel element
        m_Elements.Resize(new_capacity + 1);
        m_Elements[new_capacity] = s_SentinelElement;

        // Reset tracking data
        m_IndexMask = new_capacity - 1;

        // Reset count and re-insert all the old data back into the set
        m_Count = 0;
        for (auto& element : old_set) {
            if (element.HasValue()) {
                Emplace(std::move(element.Value));
            }
        }
    }

    template <class T, class Hasher>
    inline size_t UnorderedSet<T, Hasher>::Count() const
    {
        return m_Count;
    }

    template <class T, class Hasher>
    inline size_t UnorderedSet<T, Hasher>::Capacity() const
    {
        return m_Elements.Capacity() - 1;
    }

    template <class T, class Hasher>
    inline void UnorderedSet<T, Hasher>::RobinHoodInsert(Element&& poor_record, size_t rich_index)
    {
        // Swap the rich and poor entries
        Element rich_record(std::move(m_Elements[rich_index]));

        m_Elements[rich_index] = std::move(poor_record);

        for (size_t index = rich_index; rich_record.Offset < kProbeLimit;
             index = (index + 1) & m_IndexMask, rich_record.Offset++) {
            // If we've found an open entry, take it
            if (m_Elements[index].IsEmpty()) {
                m_Count++;
                m_Elements[index] = std::move(rich_record);
                return;
            } else if (m_Elements[index].Offset < rich_record.Offset) {
                // We found a richer record. Recursively call this robin hood function to again
                // steal from the rich
                m_Count++;
                RobinHoodInsert(std::move(rich_record), index);
                return;
            }
        }
    }

    template <class T, class Hasher>
    inline typename UnorderedSet<T, Hasher>::iterator UnorderedSet<T, Hasher>::begin() const
    {
        // Return the first non-empty element
        for (auto elementIt = m_Elements.begin(); elementIt != m_Elements.end(); elementIt++) {
            if (!elementIt->IsEmpty()) {
                return iterator(&(*elementIt));
            }
        }

        return iterator(&(*m_Elements.begin()));
    }

    template <class T, class Hasher>
    inline typename UnorderedSet<T, Hasher>::iterator UnorderedSet<T, Hasher>::end() const
    {
        return iterator(&(*(m_Elements.end() - 1)));
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// UnorderedSet::Element Implementation
    ///////////////////////////////////////////////////////////////////////////////////////////////

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::Element::Element()
    {
        Offset = kEmptyOffset;
    }

    template <class T, class Hasher>
    template <class... Args>
    inline UnorderedSet<T, Hasher>::Element::Element(offset_t offset, Args&&... args) noexcept
    {
        Offset = offset;
        new (std::addressof(Value)) T(std::forward<Args>(args)...);
    }

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::Element::Element(const Element& other)
    {
        Offset = other.Offset;
        if (!IsEmpty()) {
            new (std::addressof(Value)) T(other.Value);
        }
    }

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::Element::Element(Element&& other) noexcept
    {
        Offset = other.Offset;
        if (!IsEmpty()) {
            new (std::addressof(Value)) T(std::move(other.Value));
        }
    }

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::Element::~Element()
    {
    }

    template <class T, class Hasher>
    inline typename UnorderedSet<T, Hasher>::Element&
    UnorderedSet<T, Hasher>::Element::operator=(Element& other)
    {
        if (!IsEmpty()) {
            Value.~T();
        }

        Offset = other.Offset;
        if (!other.IsEmpty()) {
            new (std::addressof(Value)) T(other.Value);
        }

        return *this;
    }

    template <class T, class Hasher>
    inline typename UnorderedSet<T, Hasher>::Element&
    UnorderedSet<T, Hasher>::Element::operator=(Element&& other)
    {
        if (!IsEmpty()) {
            Value.~T();
        }

        Offset = other.Offset;
        if (!other.IsEmpty()) {
            new (std::addressof(Value)) T(std::move(other.Value));
        }

        return *this;
    }

    template <class T, class Hasher>
    inline bool UnorderedSet<T, Hasher>::Element::IsUninitialized() const
    {
        return Offset == kEmptyOffset;
    }

    template <class T, class Hasher>
    inline bool UnorderedSet<T, Hasher>::Element::IsEmpty() const
    {
        return Offset == kEmptyOffset || Offset == kTombstoneOffset;
    }

    template <class T, class Hasher>
    inline bool UnorderedSet<T, Hasher>::Element::IsSentinel() const
    {
        return Offset == kEndOfSetSentinel;
    }

    template <class T, class Hasher>
    inline bool UnorderedSet<T, Hasher>::Element::HasValue() const
    {
        return Offset >= 0;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// SetIterator Implementation
    ///////////////////////////////////////////////////////////////////////////////////////////////

    template <class Set_t>
    inline SetIterator_t<Set_t>::SetIterator_t(element_pointer element) : m_CurrElement(element)
    {
    }

    template <class Set_t>
    inline typename SetIterator_t<Set_t>::const_reference SetIterator_t<Set_t>::operator*() const
    {
        return m_CurrElement->Value;
    }

    template <class Set_t>
    inline typename SetIterator_t<Set_t>::const_pointer SetIterator_t<Set_t>::operator->() const
    {
        return &m_CurrElement->Value;
    }

    template <class Set_t>
    inline bool SetIterator_t<Set_t>::operator==(const SetIterator_t& other) const
    {
        return m_CurrElement == other.m_CurrElement;
    }

    template <class Set_t>
    inline bool SetIterator_t<Set_t>::operator!=(const SetIterator_t& other) const
    {
        return !(*this == other);
    }

    template <class Set_t>
    inline SetIterator_t<Set_t>& SetIterator_t<Set_t>::operator++()
    {
        do {
            m_CurrElement++;
        } while (m_CurrElement->IsEmpty() && !m_CurrElement->IsSentinel());

        return *this;
    }

    template <class Set_t>
    inline SetIterator_t<Set_t> SetIterator_t<Set_t>::operator++(int)
    {
        SetIterator_t<Set_t> tmp(*this);
        this->operator++();
        return tmp;
    }
} // namespace sj
