#pragma once
// STD Headers
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

        using bucket_iterator = typename Set_t::const_bucket_iterator;
        using const_bucket_iterator = typename Set_t::const_bucket_iterator;
        using local_iterator = typename Set_t::const_local_iterator;
        using const_local_iterator = typename Set_t::const_local_iterator;

      public:
        /**
         * Constructor
         * @param bucket The bucket the iterator points to
         */
        SetIterator_t(bucket_iterator bucket)
        {
            m_BucketIter = bucket;
            m_ElementIter = bucket->begin();
        }

        /**
         * Constructor
         * @param bucket The bucket the iterator points to
         */
        SetIterator_t(bucket_iterator bucket, local_iterator element)
        {
            m_BucketIter = bucket;
            m_ElementIter = bucket->begin();
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
        bool operator==(const SetIterator_t& other) const
        {
            // Elements are unique among buckets, so the element iterator determines equality
            return m_ElementIter == other.m_ElementIter;
        }

        /** Inequality comparison operator */
        bool operator!=(const SetIterator_t& other) const
        {
            return !(*this == other);
        }

        /** Pre-increment operator overload */
        SetIterator_t& operator++()
        {
            if (m_ElementIter == m_BucketIter->end() || m_ElementIter + 1 == m_BucketIter->end()) {
                m_BucketIter++;
                m_ElementIter = m_BucketIter->start();
            } else {
                m_ElementIter++;
            }

            return *this;
        }

        /** Post-increment operator overload */
        SetIterator_t& operator++(int)
        {
            SetIterator_t tmp(*this);
            this->operator++();
            return tmp;
        }

      private:
        bucket_iterator m_BucketIter;
        local_iterator m_ElementIter;
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
        using iterator = typename SetIterator_t<const UnorderedSet<T, Hasher>>;
        using const_iterator = typename SetIterator_t<const UnorderedSet<T, Hasher>>;
        using local_iterator = typename const Bucket::iterator;
        using const_local_iterator = typename Bucket::const_iterator;

      public:
        /**
         * Default constructor
         */
        UnorderedSet(Allocator* allocator = MemorySystem::GetDefaultAllocator());

        /**
         * Copy assignment from other vector
         */
        UnorderedSet<T>& operator=(const UnorderedSet<T>& other);

        /**
         * Move assignment from other vector
         */
        UnorderedSet<T>& operator=(UnorderedSet<T>&& other);

        /**
         * Insert a value into the set
         */
        std::pair<iterator, bool> Insert(const T& value);

        void Erase(const T& value);

        /**
         * Sets the number of buckets in the container to n or more.
         * @note if n <= to the current bucket count, no action is taken
         * @note Invalidates ALL iterators
         */
        void Rehash(size_t n);

        /**
         * Removes all elements in the Set
         */
        void Clear();

      private:
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

      public:
        using bucket_iterator = typename decltype(m_Buckets)::const_iterator;
        using const_bucket_iterator = typename decltype(m_Buckets)::const_iterator;

        iterator begin();
        iterator end();
    };

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::UnorderedSet(Allocator* allocator)
        : m_Allocator(allocator), m_MaxLoadFactor(1.0f), m_Size(0), m_BucketMask(0),
          m_Buckets(m_Allocator), m_HashFunctor()
    {
    }

    template <class T, class Hasher>
    inline UnorderedSet<T>& UnorderedSet<T, Hasher>::operator=(const UnorderedSet<T>& other)
    {
        // TODO: insert return statement here
    }

    template <class T, class Hasher>
    inline UnorderedSet<T>& UnorderedSet<T, Hasher>::operator=(UnorderedSet<T>&& other)
    {
        Clear();

        m_Allocator = other.m_Allocator;
        m_BucketMask = other.m_Allocator;
        m_Buckets = std::move(other.m_Buckets);
        m_HashFunctor = other.m_HashFunctor;
        m_MaxLoadFactor = other.m_MaxLoadFactor;
        m_Size = other.m_Size;
    }

    template <class T, class Hasher>
    inline std::pair<typename UnorderedSet<T, Hasher>::iterator, bool>
    UnorderedSet<T, Hasher>::Insert(const T& value)
    {
        // Adding this element will push us over our load factor. Rehash
        if (m_Buckets.Size() == 0) {
            Rehash(1);
        } else if ((m_Size + 1.0f) / m_Buckets.Size() > m_MaxLoadFactor) {
            Rehash(m_Buckets.Size() * 2);
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
    inline void UnorderedSet<T, Hasher>::Erase(const T& value)
    {
        size_t bucket = m_HashFunctor(value) & m_BucketMask;

        // m_Buckets[bucket].Erase();
    }

    template <class T, class Hasher>
    inline void UnorderedSet<T, Hasher>::Rehash(size_t n)
    {
        if (n <= m_Buckets.Size()) {
            return;
        }

        SJ_ASSERT((n & (n - 1)) == 0, "Bucket count must be a perfect power of two!");

        /**
         * Allocate a new bucket list
         */
        Vector<Bucket> new_buckets(m_Allocator, n);
        m_BucketMask = n - 1;

        // Don't need to check for duplicate keys
        for (auto& entry : *this) {
            size_t bucket = m_HashFunctor(entry) & m_BucketMask;
            new_buckets[bucket].EmplaceBack(std::move(entry));
        }

        *this = std::move(new_buckets);
    }

    template <class T, class Hasher>
    inline void UnorderedSet<T, Hasher>::Clear()
    {
        for (auto& bucket : m_Buckets) {
            bucket.Clear();
        }
    }

    template <class T, class Hasher>
    inline typename UnorderedSet<T, Hasher>::iterator UnorderedSet<T, Hasher>::begin()
    {
        if (m_Buckets.Size() == 0) {
            return end();
        }

        // return iterator(m_Buckets.start(), m_Buckets.At(0).start());
    }

    template <class T, class Hasher>
    inline typename UnorderedSet<T, Hasher>::iterator UnorderedSet<T, Hasher>::end()
    {
        auto last_bucket = m_Buckets.At(m_Buckets.Size() - 1);
        // return iterator(local_iterator(last_bucket), ;
    }

} // namespace sj
