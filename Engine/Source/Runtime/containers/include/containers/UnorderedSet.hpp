#pragma once
// STD Headers
#include <type_traits>
#include <utility>

// Library Headers

// Screwjank Headers
#include "core/MemorySystem.hpp"
#include "containers/Vector.hpp"

namespace sj {
    template <class Set_t>
    class SetIterator_t
    {
      public:
        using iterator_category = std::forward_iterator_tag;

        using value_type = std::conditional_t<std::is_const<Set_t>::value,
                                              typename const Set_t::value_type,
                                              typename Set_t::value_type>;
    };

    /**
     * Closed addressing set using dynamic arrays for buckets
     */
    template <class T, class Hasher = std::hash<T>>
    class UnorderedSet
    {
      private:
        struct Bucket
        {
            Vector<T> Entries;

            // Iterators
            using iterator = typename decltype(Entries)::iterator;
            using const_iterator = typename decltype(Entries)::const_iterator;

            Bucket() : Entries(m_Allocator)
            {
            }

            typename Vector<T>::iterator begin()
            {
                return Entries.begin();
            }

            typename Vector<T>::iterator end()
            {
                return Entries.end();
            }
        };

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
        using iterator = typename SetIterator_t<UnorderedSet<T, Hasher>>;
        using const_iterator = typename SetIterator_t<const UnorderedSet<T, Hasher>>;
        using local_iterator = typename Bucket::iterator;
        using const_local_iterator = typename Bucket::const_iterator;

      public:
        /**
         * Default constructor
         */
        UnorderedSet(Allocator* allocator = MemorySystem::GetDefaultAllocator());

        /**
         * Insert a value into the set
         */
        std::pair<iterator, bool> Insert(const T& value);

        /**
         * Sets the number of buckets in the container to n or more.
         * @note if n <= to the current bucket count, no action is taken
         * @note Invalidates ALL iterators
         */
        void Rehash(size_t n);

        /** Allocator to be used for the set's operations */
        Allocator* m_Allocator;

        /** The upper bound for the set's load factor during operation */
        float m_MaxLoadFactor;

        /** The number of elements contained in the set */
        size_t m_Size;

        /**
         * Always equal to m_Buckets.Size() - 1, allows a fast approximation of the mod operator
         * when bucket size is a perfect power of two
         */
        size_t m_BucketMask;

        /** Vector of buckets to represent the hashed set */
        Vector<Bucket> m_Buckets;

        /** Functor used to hash elements of the list */
        Hasher m_HashFunctor;
    };

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::UnorderedSet(Allocator* allocator)
        : m_Allocator(allocator), m_MaxLoadFactor(1.0f), m_Size(0), m_BucketMask(0),
          m_Buckets(m_Allocator), m_HashFunctor()
    {
    }

    template <class T, class Hasher>
    inline std::pair<typename UnorderedSet<T, Hasher>::iterator, bool>
    UnorderedSet<T, Hasher>::Insert(const T& value)
    {
        // Adding this element will push us over our load factor. Rehash
        if (m_Buckets.Size() == 0 || (m_Size + 1.0f) / m_Buckets.Size() > m_MaxLoadFactor) {
            Rehash(m_Buckets.Size() + 1);
        }

        // Buckets.Size() is always a power of two
        // m_BucketMask is always Buckets.Size() - 1;
        // This allows us to use the & operator instead of %
        auto index = m_HashFunctor(value) & m_BucketMask;

        auto bucket = m_Buckets[index];

        // Ensure the key is unique in the bucket
        auto it = bucket.begin();
        for (; it != (bucket.end() - 1); it++) {
            if (value == *it) {
                // Return the element with the matching key, and indicate that the new value was not
                // inserted
                return {it, false};
            }
        }

        return std::pair<iterator, bool>();
    }

    template <class T, class Hasher>
    inline void UnorderedSet<T, Hasher>::Rehash(size_t n)
    {
        if (n <= m_Buckets.Size()) {
            return;
        }

        /**
         * Allocate a new bucket list
         */
        Vector<Bucket> new_buckets(m_Allocator, n);

        // for (auto entry : *this) {
        //}
    }

} // namespace sj
