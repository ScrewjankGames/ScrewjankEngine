#pragma once

// STD Headers
#include <optional>
#include <type_traits>
#include <utility>

// Library Headers

// Screwjank Headers
#include "core/MemorySystem.hpp"
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
        using pointer = typename Set_t::const_pointer;
        using const_pointer = typename Set_t::const_pointer;
        using reference = typename Set_t::const_reference;
        using const_reference = typename Set_t::const_reference;

        using difference_type = typename Set_t::difference_type;

      public:
        /**
         * Constructor
         */
        SetIterator_t(pointer element) : m_CurrElement(element)
        {
        }

      private:
        /** The element currently pointer at by this iterator */
        pointer m_CurrElement;
    };

    /**
     * Open-addressed robinhood hashed set
     */
    template <class T, class Hasher = std::hash<T>>
    class UnorderedSet
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
        using iterator = typename SetIterator_t<const UnorderedSet<T, Hasher>>;
        using const_iterator = typename SetIterator_t<const UnorderedSet<T, Hasher>>;

      public:
        /**
         * Default constructor
         */
        UnorderedSet(Allocator* allocator = MemorySystem::GetDefaultAllocator());

        /**
         * Insert an element into the set
         * @return An iterator to the element inserted
         */
        iterator Insert(const T& key);

        /**
         * Emplace an element into the set
         * @return An iterator to the element inserted
         */
        template <class... Args>
        iterator Emplace(Args&&... args);

        /**
         * Erase an element from the set
         */
        bool Erase(const T& key);

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
         */
        size_t Capacity() const;

      private:
        /** Offset that indicates an element is empty */
        static constexpr uint8_t PROBE_LIMIT = std::numeric_limits<uint8_t>::max() - 1;

        /** Flag to indicate a cell was erased so later lookups can probe correctly */
        static constexpr uint8_t TOMBSTONE_VALUE = std::numeric_limits<uint8_t>::max();

        struct Element
        {
            /**
             * Constructor
             */
            Element() {};

            /**
             * Copy Constructor
             */
            Element(const Element& other)
            {
                Offset = other.Offset;
                if (!IsEmpty()) {
                    new (std::addressof(Value)) T(other.Value);
                }
            }

            /**
             * Move Constructor
             */
            Element(Element&& other)
            {
                Offset = other.Offset;
                if (!IsEmpty()) {
                    new (std::addressof(Value)) T(std::move(other.Value));
                }
            }

            /**
             * Destructor
             */
            ~Element() {};

            bool IsEmpty() const
            {
                // Both PROBE_LIMIT and TOMBSTONE_VALUE denote empty cells.
                return Offset >= PROBE_LIMIT;
            }

            bool HasValue() const
            {
                // Both PROBE_LIMIT and TOMBSTONE_VALUE denote empty cells.
                return Offset < PROBE_LIMIT;
            }

            template <class... Args>
            void EmplaceValue(Args&&... args)
            {
                new (std::addressof(Value)) T(std::forward<Args>(args)...);
            }

            /** Element's distance from it's desired hash slot, where -1 is uninitialized
               element */
            uint8_t Offset = PROBE_LIMIT;

            /** Nameless union to prevent premature initialization of value by Vector */
            union
            {
                T Value;
            };
        };

        /** Backing allocator for the datastructure */
        Allocator* m_Allocator;

        /** Functor used to hash keys */
        Hasher m_HashFunctor;

        /** Entries of the hash set */
        Vector<Element> m_Elements;

        /** Mask used to assign hashed keys to buckets */
        size_t m_IndexMask;

        /** Number of elements in the set */
        size_t m_Count;

        /** Used to terminate lookups early */
        uint8_t m_MaxProbeDistance;

        /** The maximum load factor of the set */
        float m_MaxLoadFactor;
    };

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::UnorderedSet(Allocator* allocator)
        : m_Allocator(allocator), m_HashFunctor(), m_Elements(allocator, 1), m_IndexMask(0),
          m_Count(0), m_MaxProbeDistance(0), m_MaxLoadFactor(0.9f)
    {
    }

    template <class T, class Hasher>
    inline typename UnorderedSet<T, Hasher>::iterator
    UnorderedSet<T, Hasher>::Insert(const T& value)
    {
        return Emplace(value);
    }

    template <class T, class Hasher>
    template <class... Args>
    inline typename UnorderedSet<T, Hasher>::iterator
    UnorderedSet<T, Hasher>::Emplace(Args&&... args)
    {
        auto new_count = m_Count + 1;
        // If there's not enough space, rehash
        if ((new_count / Capacity()) > m_MaxLoadFactor || new_count > m_Elements.Capacity()) {
            Rehash(m_Elements.Capacity() * 2);
        }

        // Insert the element
        T value(std::forward<Args>(args)...);
        auto hash = m_HashFunctor(value);
        auto start_index = hash & m_IndexMask;

        for (int8_t i = 0; i < PROBE_LIMIT; i++) {
            // Allow probing to wrap around the set
            auto index = (start_index + i) & m_IndexMask;

            if (m_Elements[index].IsEmpty()) {
                m_Elements[index].Offset = i;
                m_Elements[index].EmplaceValue(std::move(value));
                if (i > m_MaxProbeDistance) {
                    m_MaxProbeDistance = i;
                }

                return iterator(nullptr);
            }
        }

        SJ_ASSERT(
            false,
            "UnorderedSet insertion failed due to excessive collision count (>= {}!). Check your "
            "hash function.",
            PROBE_LIMIT);

        return iterator(nullptr);
    }

    template <class T, class Hasher>
    inline bool UnorderedSet<T, Hasher>::Erase(const T& key)
    {
    }

    template <class T, class Hasher>
    inline void UnorderedSet<T, Hasher>::Rehash(size_t new_capacity)
    {
        SJ_ASSERT((new_capacity & (new_capacity - 1)) == 0,
                  "UnorderedSet capacity must be a power of two!");

        if (new_capacity <= m_Elements.Capacity()) {
            return;
        }

        // Move the old element set into a temporary
        Vector<Element> old_set(std::move(m_Elements));
        m_Elements = Vector<Element>(m_Allocator, new_capacity);

        // Reset tracking data
        m_Count = 0;
        m_IndexMask = new_capacity - 1;
        m_MaxProbeDistance = 0;

        // Re-insert all the old data into the new UnorderedSet
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
        return m_Elements.Capacity();
    }

} // namespace sj
