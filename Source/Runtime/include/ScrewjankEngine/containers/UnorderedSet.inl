#include <ScrewjankEngine/containers/UnorderedSet.hpp>

namespace sj
{
    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::UnorderedSet()
        : UnorderedSet(MemorySystem::GetCurrentHeapZone())
    {
    }

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::UnorderedSet(std::initializer_list<T> list)
        : UnorderedSet(MemorySystem::GetCurrentHeapZone(), list)
    {
    }

    template <class T, class Hasher>
    template <class InputIterator>
    inline UnorderedSet<T, Hasher>::UnorderedSet(InputIterator first, InputIterator last)
        : UnorderedSet(MemorySystem::GetCurrentHeapZone(), first, last)
    {
    }

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::UnorderedSet(HeapZoneBase* heap_zone)
        : m_HashFunctor(), m_Elements(heap_zone, 2, Element {s_SentinelElement}), m_IndexMask(0),
          m_Count(0), m_MaxLoadFactor(0.9f)
    {
    }

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::UnorderedSet(HeapZoneBase* heap_zone, std::initializer_list<T> list)
        : UnorderedSet(heap_zone)
    {
        for (auto key : list)
        {
            Insert(key);
        }
    }

    template <class T, class Hasher>
    template <class InputIterator>
    inline UnorderedSet<T, Hasher>::UnorderedSet(HeapZoneBase* heap_zone,
                                                 InputIterator first,
                                                 InputIterator last)
        : UnorderedSet(heap_zone)
    {
        Emplace(first, last);
    }

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::UnorderedSet(UnorderedSet<T, Hasher>&& other) noexcept
        : m_Elements(std::move(other.m_Elements))
    {
        other.m_Elements.resize(2, Element {s_SentinelElement});
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
        for (auto& key : list)
        {
            Insert(key);
        }

        return *this;
    }

    template <class T, class Hasher>
    inline bool UnorderedSet<T, Hasher>::operator==(const UnorderedSet<T>& other) const
    {
        if (m_Count != other.Count())
        {
            return false;
        }

        // Look up each element in this set in the other set
        for (auto& element : *this)
        {
            if (!other.Contains(element))
            {
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
             index = (index + 1) & m_IndexMask, curr_offset++)
        {

            if (key == m_Elements[index].Value)
            {
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
    template <class InputIterator>
    inline void UnorderedSet<T, Hasher>::Insert(InputIterator first, InputIterator last)
    {
        for (auto it = first; it != last; it++)
        {
            Insert(*it);
        }
    }

    template <class T, class Hasher>
    template <class InputIterator>
    inline void UnorderedSet<T, Hasher>::Emplace(InputIterator first, InputIterator last)
    {
        for (auto it = first; it != last; it++)
        {
            Emplace(*it);
        }
    }

    template <class T, class Hasher>
    template <class... Args>
    inline std::pair<typename UnorderedSet<T, Hasher>::iterator, bool>
    UnorderedSet<T, Hasher>::Emplace(Args&&... args)
    {
        // construct a new record on the stack
        Element new_record(0, std::move(T(std::forward<Args>(args)...)));
        auto hash = m_HashFunctor(new_record.Value);
        auto desired_index = hash & m_IndexMask;

        // Ensure supplied key is not in the Set (same as the Find() operation)
        size_t search_start_index = desired_index;

        for (auto [index, curr_offset] = std::tuple {desired_index, offset_t(0)};
             m_Elements[index].Offset >= curr_offset;
             index = (index + 1) & m_IndexMask, curr_offset++)
        {

            if (new_record.Value == m_Elements[index].Value)
            {
                return {const_iterator(&m_Elements[index]), false};
            }
        }

        // The new key is unique, try to insert it
        auto new_count = m_Count + 1;

        // If there's not enough space, rehash
        if ((new_count / Capacity()) > m_MaxLoadFactor || new_count > Capacity())
        {
            Rehash(Capacity() * 2);

            // Recompute desired index, rehashing changes index mask
            desired_index = hash & m_IndexMask;
        }

        // Perform the robin hood insertion, with a maximum probe count of kProbeLimit
        for (size_t index = desired_index; new_record.Offset < kProbeLimit;
             index = (index + 1) & m_IndexMask, new_record.Offset++)
        {

            // If we've found an open entry, take it
            if (m_Elements[index].IsEmpty())
            {
                m_Elements[index] = std::move(new_record);
                m_Count++;
                return {const_iterator(&m_Elements[index]), true};
            }
            else if (m_Elements[index].Offset < new_record.Offset)
            {
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
             index = (index + 1) & m_IndexMask, offset++)
        {
            if (m_Elements[index].Value == key)
            {
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
        m_Elements.clear();
        m_Count = 0;
    }

    template <class T, class Hasher>
    inline void UnorderedSet<T, Hasher>::Rehash(size_t new_capacity)
    {
        SJ_ASSERT((new_capacity & (new_capacity - 1)) == 0,
                  "UnorderedSet capacity must be a power of two!");

        if (new_capacity <= Capacity())
        {
            return;
        }

        // Move the old element set into a temporary
        dynamic_vector<Element> old_set(std::move(m_Elements));

        // Reserve enough space for the new capacity, plus one slot for the sentinel element
        m_Elements.resize(new_capacity + 1);
        m_Elements[new_capacity] = s_SentinelElement;

        // Reset tracking data
        m_IndexMask = new_capacity - 1;

        // Reset count and re-insert all the old data back into the set
        m_Count = 0;
        for (auto& element : old_set)
        {
            if (element.HasValue())
            {
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
        return m_Elements.capacity() - 1;
    }

    template <class T, class Hasher>
    inline void UnorderedSet<T, Hasher>::RobinHoodInsert(Element&& poor_record, size_t rich_index)
    {
        // Swap the rich and poor entries
        Element rich_record(std::move(m_Elements[rich_index]));

        m_Elements[rich_index] = std::move(poor_record);

        for (size_t index = rich_index; rich_record.Offset < kProbeLimit;
             index = (index + 1) & m_IndexMask, rich_record.Offset++)
        {
            // If we've found an open entry, take it
            if (m_Elements[index].IsEmpty())
            {
                m_Count++;
                m_Elements[index] = std::move(rich_record);
                return;
            }
            else if (m_Elements[index].Offset < rich_record.Offset)
            {
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
        for (auto elementIt = m_Elements.begin(); elementIt != m_Elements.end(); elementIt++)
        {
            if (!elementIt->IsEmpty())
            {
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
    inline UnorderedSet<T, Hasher>::Element::Element(offset_t offset) noexcept
    {
        Offset = offset;
    }

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::Element::Element(offset_t offset, T& value) noexcept
    {
        Offset = offset;
        new (std::addressof(Value)) T(value);
    }

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::Element::Element(offset_t offset, T&& value) noexcept
    {
        Offset = offset;
        new (std::addressof(Value)) T(value);
    }

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::Element::Element(const Element& other)
    {
        Offset = other.Offset;
        if (!IsEmpty())
        {
            new (std::addressof(Value)) T(other.Value);
        }
    }

    template <class T, class Hasher>
    inline UnorderedSet<T, Hasher>::Element::Element(Element&& other) noexcept
    {
        Offset = other.Offset;
        if (!IsEmpty())
        {
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
        if (!IsEmpty())
        {
            Value.~T();
        }

        Offset = other.Offset;
        if (!other.IsEmpty())
        {
            new (std::addressof(Value)) T(other.Value);
        }

        return *this;
    }

    template <class T, class Hasher>
    inline typename UnorderedSet<T, Hasher>::Element&
    UnorderedSet<T, Hasher>::Element::operator=(Element&& other)
    {
        if (!IsEmpty())
        {
            Value.~T();
        }

        Offset = other.Offset;
        if (!other.IsEmpty())
        {
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
        do
        {
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
