// Screwjank Headers
#include <ScrewjankEngine/containers/List.hpp>

namespace sj
{
    ///////////////////////////////////////////////////////////////////////
    /// List Iterator List
    ///////////////////////////////////////////////////////////////////////
    template <class T, bool tIsBidirectional>
    template <bool tIsConst>
    inline List<T, tIsBidirectional>::IteratorBase<tIsConst>::IteratorBase(
        node_ptr node) : m_CurrNode(node)
    {
    }

    template <class T, bool tIsBidirectional>
    template <bool tIsConst>
    inline List<T, tIsBidirectional>::IteratorBase<tIsConst>::IteratorBase(
        const_node_ptr node) requires tIsConst : m_CurrNode(node)
    {
    }

    template <class T, bool tIsBidirectional>
    template <bool tIsConst>
    inline List<T, tIsBidirectional>::IteratorBase<tIsConst>::IteratorBase(nullptr_t null)
        : m_CurrNode(nullptr)
    {
    }

    template <class T, bool tIsBidirectional>
    template <bool tIsConst>
    inline List<T, tIsBidirectional>::IteratorBase<tIsConst>::IteratorBase(
        const IteratorBase<false>& other)
        : m_CurrNode(other.m_CurrNode)
    {
    }

    template <class T, bool tIsBidirectional>
    template <bool tIsConst>
    List<T, tIsBidirectional>::IteratorBase<tIsConst>::data_ref
    List<T, tIsBidirectional>::IteratorBase<tIsConst>::operator*()
    {
        return m_CurrNode->Data;
    }

    template <class T, bool tIsBidirectional>
    template <bool tIsConst>
    List<T, tIsBidirectional>::IteratorBase<tIsConst>::data_ptr
    List<T, tIsBidirectional>::IteratorBase<tIsConst>::operator->()
    {
        return &(m_CurrNode->Data);
    }

    template <class T, bool tIsBidirectional>
    template <bool tIsConst>
    inline List<T, tIsBidirectional>::IteratorBase<tIsConst>&
    List<T, tIsBidirectional>::IteratorBase<tIsConst>::operator++()
    {
        m_CurrNode = m_CurrNode->GetNext();
        return *this;
    }

    template <class T, bool tIsBidirectional>
    template <bool tIsConst>
    inline List<T, tIsBidirectional>::IteratorBase<tIsConst>
    List<T, tIsBidirectional>::IteratorBase<tIsConst>::operator++(int)
    {
        IteratorBase<tIsConst> tmp(*this);
        this->operator++();
        return tmp;
    }

    template <class T, bool tIsBidirectional>
    template <bool tIsConst>
    inline List<T, tIsBidirectional>::IteratorBase<tIsConst>&
    List<T, tIsBidirectional>::IteratorBase<tIsConst>::operator--() requires is_dl_list_node<node_type>
    {
        m_CurrNode = m_CurrNode->GetPrev();
        return *this;
    }

    template <class T, bool tIsBidirectional>
    template <bool tIsConst>
    inline List<T, tIsBidirectional>::IteratorBase<tIsConst>
    List<T, tIsBidirectional>::IteratorBase<tIsConst>::operator--(int) requires is_dl_list_node<node_type>
    {
        IteratorBase<tIsConst> tmp(*this);
        this->operator--();
        return tmp;
    }

    template <class T, bool tIsBidirectional>
    template <bool tIsConst>
    inline bool
    List<T, tIsBidirectional>::IteratorBase<tIsConst>::operator==(const IteratorBase& other) const
    {
        return m_CurrNode == other.m_CurrNode;
    }

    template <class T, bool tIsBidirectional>
    template <bool tIsConst>
    inline bool
    List<T, tIsBidirectional>::IteratorBase<tIsConst>::operator!=(const IteratorBase& other) const
    {
        return !(*this == other);
    }

    template <class T, bool tIsBidirectional>
    template <bool tIsConst>
    inline List<T, tIsBidirectional>::node_ptr
    List<T, tIsBidirectional>::IteratorBase<tIsConst>::GetNode()
    {
        return m_CurrNode;
    }

    ///////////////////////////////////////////////////////////////////////
    /// Managed List
    ///////////////////////////////////////////////////////////////////////
    template <class T, bool tIsBidirectional>
    inline List<T, tIsBidirectional>::List() : List(MemorySystem::GetCurrentHeapZone())
    {
    }

    template <class T, bool tIsBidirectional>
    inline List<T, tIsBidirectional>::List(std::initializer_list<T> list)
        : List(MemorySystem::GetCurrentHeapZone(), list)
    {
    }

    template <class T, bool tIsBidirectional>
    inline List<T, tIsBidirectional>::List(HeapZone* heap_zone)
        : m_HeapZone(heap_zone)
    {
    }

    template <class T, bool tIsBidirectional>
    inline List<T, tIsBidirectional>::List(HeapZone* heap_zone, std::initializer_list<T> list)
        : List<T, tIsBidirectional>(heap_zone)
    {
        for(auto it = list.end() - 1; it >= list.begin(); it--)
        {
            PushFront(*it);
        }
    }

    template <class T, bool tIsBidirectional>
    inline List<T, tIsBidirectional>::List(const List<T, tIsBidirectional>& other)
    {
        m_HeapZone = other.m_HeapZone;
        HeapZoneScope hzScope(m_HeapZone);

        if(other.IsEmpty())
        {
            return;
        }

        ConstIterator it = other.begin();
        PushFront(*it);
        it++;

        node_ptr currNode = &m_List.Front();

        for(; it != other.end(); it++)
        {
            m_List.InsertAfter(currNode, m_HeapZone->New<node_type>(*it));
            currNode = currNode->GetNext();
        }
    }

    template <class T, bool tIsBidirectional>
    inline List<T, tIsBidirectional>::~List()
    {
        Clear();
    }

    template <class T, bool tIsBidirectional>
    inline List<T, tIsBidirectional>&
    List<T, tIsBidirectional>::operator=(const List<T, tIsBidirectional>& other)
    {
        Clear();

        if(!other.IsEmpty())
        {
            m_HeapZone = other.m_HeapZone;

            ConstIterator otherIt = other.begin();
            PushFront(*otherIt);
            otherIt++;

            for(; otherIt != other.end(); otherIt++)
            {
                auto currIt = BackIter();
                InsertAfter(currIt, *otherIt);
            }
        }

        return *this;
    }

    template <class T, bool tIsBidirectional>
    inline List<T, tIsBidirectional>&
    List<T, tIsBidirectional>::operator=(std::initializer_list<T> list)
    {
        Clear();

        for (auto it = list.end() - 1; it >= list.begin(); it--)
        {
            PushFront(*it);
        }

        return *this;
    }

    template <class T, bool tIsBidirectional>
    inline void List<T, tIsBidirectional>::PushFront(const T& value)
    {
        // Allocate node and copy data
        node_ptr newNode = m_HeapZone->New<node_type>(value);

        // Push to list impl
        m_List.PushFront(newNode);
    }

    template <class T, bool tIsBidirectional>
    inline void List<T, tIsBidirectional>::PushBack(const T& value)
    {
        node_ptr newNode = m_HeapZone->New<node_type>(value);

        m_List.PushBack(newNode);
    }

    template <class T, bool tIsBidirectional>
    inline void List<T, tIsBidirectional>::PopFront()
    {
        SJ_ASSERT(!IsEmpty(), "Cannot pop from empty list.");

        node_type& tmp = m_List.Front();
        m_List.PopFront();
        m_HeapZone->Free(&tmp);
    }

    template <class T, bool tIsBidirectional>
    template <class... Args>
    inline void List<T, tIsBidirectional>::EmplaceFront(Args&&... args)
    {
        node_ptr nodePtr = static_cast<node_ptr>(m_HeapZone->AllocateType<node_type>());
        new (nodePtr) node_type (T(std::forward<Args>(args)...));
        m_List.PushFront(nodePtr);
    }

    template <class T, bool tIsBidirectional>
    inline typename List<T, tIsBidirectional>::Iterator 
    List<T, tIsBidirectional>::InsertAfter(Iterator pos, const T& value)
    {
        node_ptr nodePtr = m_HeapZone->New<node_type>(value);

        m_List.InsertAfter(pos.GetNode(), nodePtr);

        return List<T, tIsBidirectional>::Iterator(nodePtr);
    }

    template <class T, bool tIsBidirectional>
    template <class... Args>
    inline typename List<T, tIsBidirectional>::Iterator List<T, tIsBidirectional>::EmplaceAfter(Iterator pos,
                                                                          Args&&... args)
    {
        node_ptr nodePtr = static_cast<node_ptr>(m_HeapZone->AllocateType<node_type>());
        new(nodePtr) node_type {T(std::forward<Args>(args)...)};

        m_List.InsertAfter(pos.GetNode(), nodePtr);

        return List<T, tIsBidirectional>::Iterator(nodePtr);
    }

    template <class T, bool tIsBidirectional>
    inline typename List<T, tIsBidirectional>::Iterator List<T, tIsBidirectional>::EraseAfter(Iterator pos)
    {
        SJ_ASSERT(pos.GetNode()->Next != nullptr, "Cannot erase after last element of list");

        node_ptr dead_node = pos.GetNode()->Next;
        m_List.EraseAfter(pos.GetNode());
        m_HeapZone->Delete(dead_node);

        return ++pos;
    }

    template <class T, bool tIsBidirectional>
    inline void List<T, tIsBidirectional>::Clear()
    {
        // Free all the nodes allocated for the list
        while(!m_List.IsEmpty())
        {
            node_ptr curr = &m_List.Front();
            m_List.PopFront();
            m_HeapZone->Delete(curr);
        }
    }

    template <class T, bool tIsBidirectional>
    inline const T& List<T, tIsBidirectional>::Front() const
    {
        SJ_ASSERT(!IsEmpty(), "Cannot call Front() on empty list");

        return m_List.Front().Data;
    }

    template <class T, bool tIsBidirectional>
    inline T& List<T, tIsBidirectional>::Back()
    {
        return const_cast<T&>(const_cast<const List*>(this)->Back());
    }

    template <class T, bool tIsBidirectional>
    inline const T& List<T, tIsBidirectional>::Back() const
    {
        return m_List.Back();
    }

    template <class T, bool tIsBidirectional>
    inline typename List<T, tIsBidirectional>::Iterator List<T, tIsBidirectional>::BackIter()
    {
        return Iterator(&m_List.Back());
    }

    template <class T, bool tIsBidirectional>
    inline T& List<T, tIsBidirectional>::Front()
    {
        SJ_ASSERT(!IsEmpty(), "Cannot call Front() on empty list");

        return m_List.Front().Data;
    }

    template <class T, bool tIsBidirectional>
    inline bool List<T, tIsBidirectional>::IsEmpty() const
    {
        return m_List.IsEmpty();
    }

    template <class T, bool tIsBidirectional>
    inline size_t List<T, tIsBidirectional>::Size() const
    {
        return m_List.Size();
    }

    template <class T, bool tIsBidirectional>
    inline typename List<T, tIsBidirectional>::Iterator List<T, tIsBidirectional>::begin()
    {
        return Iterator(&m_List.Front());
    }

    template <class T, bool tIsBidirectional>
    inline typename List<T, tIsBidirectional>::ConstIterator List<T, tIsBidirectional>::begin() const
    {
        return ConstIterator(&(m_List.Front()));
    }

    template <class T, bool tIsBidirectional>
    inline typename List<T, tIsBidirectional>::Iterator List<T, tIsBidirectional>::end()
    {
        return Iterator(nullptr);
    }

    template <class T, bool tIsBidirectional>
    inline typename List<T, tIsBidirectional>::ConstIterator List<T, tIsBidirectional>::end() const
    {
        return ConstIterator(nullptr);
    }
}