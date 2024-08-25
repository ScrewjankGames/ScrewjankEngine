#include <ScrewjankEngine/containers/UnorderedSet.hpp>

namespace sj
{
    template <class T, size_t tRequestedSize, size_t tRealSize>
    template <class InputIterator>
    inline static_unordered_set<T, tRequestedSize, tRealSize>::static_unordered_set(InputIterator first, InputIterator last)
    {
        this->emplace(first, last);
    }


    template <class T>
    inline dynamic_unordered_set<T>::dynamic_unordered_set()
        : dynamic_unordered_set<T>(MemorySystem::GetCurrentMemSpace())
    {
    }

    template <class T>
    inline dynamic_unordered_set<T>::dynamic_unordered_set(const dynamic_unordered_set<T>& other)
        : Base(other),
          m_Elements(other.m_Elements), 
          m_Count(other.m_Count)
    {

    }

    template <class T>
    inline dynamic_unordered_set<T>::dynamic_unordered_set(dynamic_unordered_set<T>&& other)
        : Base(other),
          m_Elements(std::move(other.m_Elements)), 
          m_Count(other.m_Count)
    {
        other.m_Count = 0;
    }

    template <class T>
    inline dynamic_unordered_set<T>::dynamic_unordered_set(std::initializer_list<T> list)
        : dynamic_unordered_set<T>(MemorySystem::GetCurrentMemSpace(), list)
    {
    }

    template <class T>
    template <class InputIterator>
    inline dynamic_unordered_set<T>::dynamic_unordered_set(InputIterator first, InputIterator last)
        : dynamic_unordered_set(MemorySystem::GetCurrentMemSpace(), first, last)
    {
    }

    template <class T>
    inline dynamic_unordered_set<T>::dynamic_unordered_set(IMemSpace* mem_space)
        : m_Elements(0, mem_space)    
    {
    }

    template <class T>
    inline dynamic_unordered_set<T>::dynamic_unordered_set( IMemSpace* mem_space, std::initializer_list<T> list)
        : dynamic_unordered_set<T>(mem_space)
    {
        for (auto key : list)
        {
            this->insert(key);
        }
    }

    template <class T>
    template <class InputIterator>
    inline dynamic_unordered_set<T>::dynamic_unordered_set(IMemSpace* mem_space,
                                                 InputIterator first,
                                                 InputIterator last)
        : dynamic_unordered_set<T>(mem_space)
    {
        this->emplace(first, last);
    }

    template <class T, class IMPL, class Hasher>
    inline unordered_set_base<T, IMPL, Hasher>&
    unordered_set_base<T, IMPL, Hasher>::operator=(std::initializer_list<T> list)
    {
        clear();
        for (auto& key : list)
        {
            insert(key);
        }

        return *this;
    }

    template <class T, class IMPL, class Hasher>
    inline bool unordered_set_base<T, IMPL, Hasher>::operator==(
        const unordered_set_base<T, IMPL, Hasher>& other) const
    {
        if(GetCount() != other.count())
        {
            return false;
        }

        // Look up each element in this set in the other set
        for (auto& element : *this)
        {
            if (!other.contains(element))
            {
                return false;
            }
        }

        return true;
    }

    template <class T, class IMPL, class Hasher>
    inline typename unordered_set_base<T, IMPL, Hasher>::const_iterator
    unordered_set_base<T, IMPL, Hasher>::find(const T& key) const
    {
        auto hash = m_HashFunctor(key);
        return find_from_hash(hash, key);
    }

    template <class T, class IMPL, class Hasher>
    inline typename unordered_set_base<T, IMPL, Hasher>::const_iterator
    unordered_set_base<T, IMPL, Hasher>::find_from_hash(size_t hash, const T& key) const
    {
        if(capacity() == 0)
        {
            return end();
        }

        auto desired_index = hash % capacity();

        for(auto [index, curr_offset] = std::tuple {desired_index, offset_t(0)};
            GetElements()[index].Offset >= curr_offset;
            index = ((index + 1) % capacity()), curr_offset++)
        {

            if(key == GetElements()[index].Value)
            {
                return const_iterator(this, &GetElements()[index]);
            }
        }

        return end();
    }

    template <class T, class IMPL, class Hasher>
    inline bool unordered_set_base<T, IMPL, Hasher>::contains(const T& key) const
    {
        return find(key) != end();
    }

    template <class T, class IMPL, class Hasher>
    inline std::pair<typename unordered_set_base<T, IMPL, Hasher>::iterator, bool>
    unordered_set_base<T, IMPL, Hasher>::insert(const T& value)
    {
        return emplace(value);
    }

    template <class T, class IMPL, class Hasher>
    template <class InputIterator>
    inline void unordered_set_base<T, IMPL, Hasher>::insert(InputIterator first, InputIterator last)
    {
        for (auto it = first; it != last; it++)
        {
            insert(*it);
        }
    }

    template <class T, class IMPL, class Hasher>
    template <class InputIterator>
    inline void unordered_set_base<T, IMPL, Hasher>::emplace(InputIterator first, InputIterator last)
    {
        for (auto it = first; it != last; it++)
        {
            emplace(*it);
        }
    }

    template <class T, class IMPL, class Hasher>
    template <class... Args>
    inline std::pair<typename unordered_set_base<T, IMPL, Hasher>::iterator, bool>
    unordered_set_base<T, IMPL, Hasher>::emplace(Args&&... args)
    {
        // construct a new record on the stack
        Element<T> new_record(0);
        new(std::addressof(new_record.Value)) T(std::forward<Args>(args)...);

        size_t hash = m_HashFunctor(new_record.Value);

        if(const_iterator foundIt = find_from_hash(hash, new_record.Value); foundIt != end())
        {
            return {foundIt, false};
        }

        // The new key is unique, try to insert it
        size_type new_count = GetCount() + 1;
        
        // If there's not enough space, rehash
        if(new_count > capacity() || (new_count / capacity()) > s_MaxLoadFactor)
        {
            if constexpr(IMPL::kIsGrowable)
            {
                rehash(capacity() == 0 ? 2 : capacity() * 2);

            }
            else
            {
                SJ_ASSERT(false, "Unordered set out of space!");
            }
        }
        
        size_type desired_index = hash % capacity();

        // Perform the robin hood insertion, with a maximum probe count of kProbeLimit
        for (size_type index = desired_index; new_record.Offset < kProbeLimit;
             index = ((index + 1) % capacity()), new_record.Offset++)
        {

            // If we've found an open entry, take it
            if (GetElements()[index].IsEmpty())
            {
                GetElements()[index] = std::move(new_record);
                GetCount()++;
                return {const_iterator(this, &GetElements()[index]), true};
            }
            else if (GetElements()[index].Offset < new_record.Offset)
            {
                RobinHoodInsert(std::move(new_record), index);
                GetCount()++;
                return {const_iterator(this, &GetElements()[index]), true};
            }
        }

        SJ_ASSERT(false,
                  "UnorderedSet insertion failed due to excessive collision count (>= {}!). "
                  "Check your hash function.",
                  kProbeLimit);

        return { const_iterator (this, nullptr), false };
    }

    template <class T, class IMPL, class Hasher>
    inline bool unordered_set_base<T, IMPL, Hasher>::erase(const T& key)
    {
        auto hash = m_HashFunctor(key);
        size_type index = hash % capacity();
        offset_t offset = 0;

        for(; GetElements()[index].Offset >= offset; index = ((index + 1) % capacity()), offset++)
        {
            auto& element = GetElements()[index];
            if(element.Value == key)
            {
                GetCount()--;

                if constexpr(!std::is_trivially_destructible_v<T>)
                {
                    element.~element_type();
                }
                
                Backshift(index);

                return true;
            }
        }

        return false;
    }

    template <class T, class IMPL, class Hasher>
    inline void unordered_set_base<T, IMPL, Hasher>::clear()
    {
        for(element_type& element : GetElements())
        {
            if constexpr(!std::is_trivially_destructible_v<T>)
            {
                element.~element_type();
            }
           
            element.Offset = element_type::kEmptyOffset;
        }

        GetCount() = 0;
    }

    template <class T, class IMPL, class Hasher>
    inline void unordered_set_base<T, IMPL, Hasher>::rehash(size_t new_capacity)
    {
        if (new_capacity <= capacity())
        {
            return;
        }

        // Move the old element set into a temporary
        auto old_set(std::move(GetElements()));

        // Reserve enough space for the new capacity
        GetElements().resize(new_capacity);

        // Reset count and re-insert all the old data back into the set
        GetCount() = 0;
        for (auto& element : old_set)
        {
            if (element.HasValue())
            {
                emplace(std::move(element.Value));
            }
        }
    }

    template <class T, class IMPL, class Hasher>
    inline size_t unordered_set_base<T, IMPL, Hasher>::count() const
    {
        return GetCount();
    }

    template <class T, class IMPL, class Hasher>
    inline size_t unordered_set_base<T, IMPL, Hasher>::capacity() const
    {
        return GetElements().size();
    }

    template <class T, class IMPL, class Hasher>
    inline void unordered_set_base<T, IMPL, Hasher>::RobinHoodInsert(unordered_set_base<T, IMPL, Hasher>::element_type&& poor_record,
                                                         size_t rich_index)
    {
        // Swap the rich and poor entries
        Element rich_record(std::move(GetElements()[rich_index]));

        GetElements()[rich_index] = std::move(poor_record);

        for (size_t index = rich_index; rich_record.Offset < kProbeLimit;
             index = ((index + 1) % capacity()), rich_record.Offset++)
        {
            // If we've found an open entry, take it
            if (GetElements()[index].IsEmpty())
            {
                GetElements()[index] = std::move(rich_record);
                return;
            }
            else if (GetElements()[index].Offset < rich_record.Offset)
            {
                // We found a richer record. Recursively call this robin hood function to again
                // steal from the rich
                RobinHoodInsert(std::move(rich_record), index);
                return;
            }
        }
    }

    template <class T, class IMPL, class Hasher>
    inline void unordered_set_base<T, IMPL, Hasher>::Backshift(size_type erasedIndex)
    {
        size_type currIndex = erasedIndex;
        GetElements()[currIndex].Offset = element_type::kEmptyOffset;

        do
        {
            size_t nextIndex = (currIndex + 1) % capacity();

            if(GetElements()[nextIndex].IsEmpty() || GetElements()[nextIndex].Offset == 0)
            {
                break;
            }

            std::swap(GetElements()[currIndex], GetElements()[nextIndex]);

            currIndex = nextIndex;
        } while(currIndex != erasedIndex);

    }

    template <class T, class IMPL, class Hasher>
    inline typename unordered_set_base<T, IMPL, Hasher>::iterator unordered_set_base<T, IMPL, Hasher>::begin() const
    {
        // Return the first non-empty element
        for (auto elementIt = GetElements().begin(); elementIt != GetElements().end(); elementIt++)
        {
            if (!elementIt->IsEmpty())
            {
                return iterator(this, &(*elementIt));
            }
        }

        return iterator(this, std::to_address(GetElements().begin()));
    }

    template <class T, class IMPL, class Hasher>
    inline typename unordered_set_base<T, IMPL, Hasher>::iterator unordered_set_base<T, IMPL, Hasher>::end() const
    {
        return iterator(this, std::to_address(GetElements().end()));
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// UnorderedSet::Element Implementation
    ///////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    inline Element<T>::Element()
    {
        Offset = kEmptyOffset;
    }

    template <class T>
    inline Element<T>::Element(offset_t offset) noexcept
    {
        Offset = offset;
    }

    template <class T>
    inline Element<T>::Element(offset_t offset, T& value) noexcept
    {
        Offset = offset;
        new (std::addressof(Value)) T(value);
    }

    template <class T>
    inline Element<T>::Element(offset_t offset, T&& value) noexcept
    {
        Offset = offset;
        new (std::addressof(Value)) T(std::move(value));
    }

    template <class T>
    inline Element<T>::Element(const Element& other)
    {
        Offset = other.Offset;
        if (!IsEmpty())
        {
            new (std::addressof(Value)) T(other.Value);
        }
    }

    template <class T>
    inline Element<T>::Element(Element&& other) noexcept
    {
        Offset = other.Offset;
        if (!IsEmpty())
        {
            new (std::addressof(Value)) T(std::move(other.Value));
        }

        other.Offset = kEmptyOffset;
    }

    template <class T>
    inline Element<T>&
    Element<T>::operator=(const Element& other)
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

    template <class T>
    inline Element<T>&
    Element<T>::operator=(Element&& other)
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

        other.Offset = kEmptyOffset;
        return *this;
    }

    template <class T>
    inline bool Element<T>::IsUninitialized() const
    {
        return Offset == kEmptyOffset;
    }

    template <class T>
    inline bool Element<T>::IsEmpty() const
    {
        return Offset == kEmptyOffset;
    }

    template <class T>
    inline bool Element<T>::HasValue() const
    {
        return Offset >= 0;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// SetIterator Implementation
    ///////////////////////////////////////////////////////////////////////////////////////////////

    template <class Set_t>
    inline SetIterator_t<Set_t>::SetIterator_t(const Set_t* set, element_pointer element)
        : m_set(set), m_currElement(element)
    {
    }

    template <class Set_t>
    inline typename SetIterator_t<Set_t>::const_reference SetIterator_t<Set_t>::operator*() const
    {
        return m_currElement->Value;
    }

    template <class Set_t>
    inline typename SetIterator_t<Set_t>::const_pointer SetIterator_t<Set_t>::operator->() const
    {
        return &m_currElement->Value;
    }

    template <class Set_t>
    inline bool SetIterator_t<Set_t>::operator==(const SetIterator_t& other) const
    {
        return m_currElement == other.m_currElement;
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
            m_currElement++;
        } while (m_currElement->IsEmpty() && (*this != m_set->end() ));

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
