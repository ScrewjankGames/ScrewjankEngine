// Screwjank Headers
#include <ScrewjankEngine/containers/ForwardList.hpp>

namespace sj
{
    template <class T>
    inline ForwardList<T>::ForwardList() : ForwardList(MemorySystem::GetCurrentHeapZone())
    {
    }

    template <class T>
    inline ForwardList<T>::ForwardList(std::initializer_list<T> list)
        : ForwardList(MemorySystem::GetCurrentHeapZone(), list)
    {
    }

    template <class T>
    inline ForwardList<T>::ForwardList(HeapZone* heap_zone)
        : m_HeapZone(heap_zone), m_Head(nullptr)
    {
    }

    template <class T>
    inline ForwardList<T>::ForwardList(HeapZone* heap_zone, std::initializer_list<T> list)
        : ForwardList<T>(heap_zone)
    {
        for (auto it = list.end() - 1; it >= list.begin(); it--)
        {
            PushFront(*it);
        }
    }

    template <class T>
    inline ForwardList<T>::ForwardList(const ForwardList<T>& other)
    {
        m_HeapZone = other.m_HeapZone;
        m_Head = nullptr;

        PushFront(other.Front());

        auto it = begin();
        auto other_it = other.cbegin();
        other_it++;

        for (; other_it != other.cend(); other_it++, it++)
        {
            InsertAfter(it, *other_it);
        }
    }

    template <class T>
    inline ForwardList<T>::ForwardList(ForwardList<T>&& other) noexcept
    {
        m_HeapZone = other.m_HeapZone;

        // Take ownership of the other list's head element
        m_Head = other.m_Head;
        other.m_Head = nullptr;
    }

    template <class T>
    inline ForwardList<T>::~ForwardList()
    {
        Clear();
    }

    template <class T>
    inline ForwardList<T>& ForwardList<T>::operator=(std::initializer_list<T> list)
    {
        Clear();

        for (auto it = list.end() - 1; it >= list.begin(); it--)
        {
            PushFront(*it);
        }

        return *this;
    }

    template <class T>
    inline ForwardList<T>& ForwardList<T>::operator=(const ForwardList<T>& other)
    {
        Clear();

        PushFront(other.Front());

        auto it = begin();
        auto other_it = other.cbegin();
        other_it++;

        for (; other_it != other.cend(); other_it++, it++)
        {
            InsertAfter(it, *other_it);
        }

        return *this;
    }

    template <class T>
    inline ForwardList<T>& ForwardList<T>::operator=(ForwardList<T>&& other)
    {
        Clear();

        PushFront(other.Front());

        auto it = begin();
        auto other_it = other.begin();
        other_it++;

        for (; other_it != other.cend(); other_it++, it++)
        {
            InsertAfter(it, std::move(*other_it));
        }

        return *this;
    }

    template <class T>
    inline void ForwardList<T>::PushFront(const T& value)
    {
        m_Head = m_HeapZone->New<ForwardList<T>::node_type>(value, m_Head);
    }

    template <class T>
    inline void ForwardList<T>::PopFront()
    {
        SJ_ASSERT(m_Head != nullptr, "Cannot pop from empty list.");

        auto tmp = m_Head;
        m_Head = m_Head->Next;

        m_HeapZone->Free(tmp);
    }

    template <class T>
    template <class... Args>
    inline void ForwardList<T>::EmplaceFront(Args&&... args)
    {
        m_Head =
            m_HeapZone->New<ForwardList<T>::node_type>(T(std::forward<Args>(args)...), m_Head);
    }

    template <class T>
    inline typename ForwardList<T>::iterator ForwardList<T>::InsertAfter(iterator pos,
                                                                         const T& value)
    {
        auto new_node = m_HeapZone->New<ForwardList<T>::node_type>(value, pos.m_Node->Next);

        pos.m_Node->Next = new_node;

        return ForwardList<T>::iterator(new_node);
    }

    template <class T>
    template <class... Args>
    inline typename ForwardList<T>::iterator ForwardList<T>::EmplaceAfter(iterator pos,
                                                                          Args&&... args)
    {
        auto new_node = m_HeapZone->New<ForwardList<T>::node_type>(T(std::forward<Args>(args)...),
                                                                    pos.m_Node->Next);
        pos.m_Node->Next = new_node;
        return ForwardList<T>::iterator(new_node);
    }

    template <class T>
    inline typename ForwardList<T>::iterator ForwardList<T>::EraseAfter(iterator pos)
    {
        SJ_ASSERT(pos.m_Node->Next != nullptr, "Cannot erase after last element of list");

        auto dead_node = pos.m_Node->Next;
        pos.m_Node->Next = dead_node->Next;

        m_HeapZone->Delete(dead_node);
        return iterator(pos.m_Node->Next);
    }

    template <class T>
    inline void ForwardList<T>::Clear()
    {
        if (m_Head == nullptr)
        {
            return;
        }

        // Free all the nodes allocated for the list
        auto curr = m_Head;
        while (curr->Next != nullptr)
        {
            auto tmp = curr->Next;
            m_HeapZone->Free(curr);
            curr = tmp;
        }

        m_Head = nullptr;
    }

    template <class T>
    inline const T& ForwardList<T>::Front() const
    {
        SJ_ASSERT(m_Head != nullptr, "Cannot call Front() on empty list");

        return m_Head->Data;
    }

    template <class T>
    inline T& ForwardList<T>::Front()
    {
        SJ_ASSERT(m_Head != nullptr, "Cannot call Front() on empty list");

        return m_Head->Data;
    }

    template <class T>
    inline typename ForwardList<T>::iterator ForwardList<T>::begin()
    {
        return iterator(m_Head);
    }

    template <class T>
    inline typename ForwardList<T>::const_iterator ForwardList<T>::begin() const
    {
        return const_iterator(m_Head);
    }

    template <class T>
    inline typename ForwardList<T>::const_iterator ForwardList<T>::cbegin() const
    {
        return const_iterator(m_Head);
    }

    template <class T>
    inline typename ForwardList<T>::iterator ForwardList<T>::end()
    {
        return iterator(nullptr);
    }

    template <class T>
    inline typename ForwardList<T>::const_iterator ForwardList<T>::end() const
    {
        return const_iterator(nullptr);
    }

    template <class T>
    inline typename ForwardList<T>::const_iterator ForwardList<T>::cend() const
    {
        return const_iterator(nullptr);
    }
}