#include <ScrewjankEngine/containers/UnmanagedList.hpp>

namespace sj
{
    template <class NodeType> requires(is_list_node<NodeType>) 
    template <bool tIsConst>
    inline UnmanagedList<NodeType>::IteratorBase<tIsConst>::IteratorBase(NodeType* node)
        : m_CurrNode(node)
    {
        
    }

    template <class NodeType> requires(is_list_node<NodeType>) 
    template <bool tIsConst>
    inline UnmanagedList<NodeType>::IteratorBase<tIsConst>::IteratorBase(
        const IteratorBase<false>& other)
        : IteratorBase(reinterpret_cast<IteratorBase<true>&>(other))
    {

    }

    template <class NodeType>
    requires(is_list_node<NodeType>) 
    template <bool tIsConst>
    inline UnmanagedList<NodeType>::IteratorBase<tIsConst>::node_ref 
        UnmanagedList<NodeType>::IteratorBase<tIsConst>::operator*()
    {
        return *m_CurrNode;
    }

    template <class NodeType> requires(is_list_node<NodeType>) 
    template <bool tIsConst>
    inline UnmanagedList<NodeType>::IteratorBase<tIsConst>::node_ptr
        UnmanagedList<NodeType>::IteratorBase<tIsConst>::operator->()
    {
        return m_CurrNode;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    template <bool tIsConst>
    inline bool UnmanagedList<NodeType>::IteratorBase<tIsConst>::operator==(const IteratorBase& other) const
    {
        return m_CurrNode == other.m_CurrNode;
    }

    template <class NodeType>
    requires(is_list_node<NodeType>)
    template <bool tIsConst>
    inline bool UnmanagedList<NodeType>::IteratorBase<tIsConst>::operator!=(const IteratorBase& other) const
    {
        return !(m_CurrNode == other.m_CurrNode);
    }

    template <class NodeType> requires(is_list_node<NodeType>) 
    template <bool tIsConst>
    inline UnmanagedList<NodeType>::IteratorBase<tIsConst>& 
        UnmanagedList<NodeType>::IteratorBase<tIsConst>::operator++()
    {
        m_CurrNode = m_CurrNode->GetNext();
        return *this;
    }

    template <class NodeType> requires(is_list_node<NodeType>) 
    template <bool tIsConst>
    inline UnmanagedList<NodeType>::IteratorBase<tIsConst> UnmanagedList<NodeType>::IteratorBase<tIsConst>::operator++(int)
    {
        IteratorBase<tIsConst> tmp(*this);
        this->operator++();
        return tmp;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    template <bool tIsConst>
    inline UnmanagedList<NodeType>::IteratorBase<tIsConst>& UnmanagedList<NodeType>::IteratorBase<tIsConst>::operator--() 
        requires is_dl_list_node<NodeType>
    {
        m_CurrNode = m_CurrNode->GetPrev();
        return *this;
    }

    template <class NodeType> requires(is_list_node<NodeType>) 
    template <bool tIsConst>
    inline UnmanagedList<NodeType>::IteratorBase<tIsConst> UnmanagedList<NodeType>::IteratorBase<tIsConst>::operator--(int)
        requires is_dl_list_node<NodeType>
    {
        IteratorBase<tIsConst> tmp(*this);
        this->operator--();
        return tmp;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    inline UnmanagedList<NodeType>::UnmanagedList(UnmanagedList<NodeType>&& other)
    {
        m_Head = other.m_Head;
        other.m_Head = nullptr;

        m_Tail = other.m_Tail;
        other.m_Tail = nullptr;

        m_Size = other.m_Size;
        other.m_Size = 0;
    }

    template <class NodeType>
        requires(is_list_node<NodeType>)
    inline NodeType& UnmanagedList<NodeType>::Front()
    {
        SJ_ASSERT(m_Head != nullptr, "Cannot peek empty list");
        return *m_Head;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    inline const NodeType& UnmanagedList<NodeType>::Front() const
    {
        SJ_ASSERT(m_Head != nullptr, "Cannot peek empty list");
        return *m_Head;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    inline NodeType& UnmanagedList<NodeType>::Back()
    {
        SJ_ASSERT(m_Head != nullptr, "Cannot peek empty list");
        return *m_Tail;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    inline const NodeType& UnmanagedList<NodeType>::Back() const
    {
        SJ_ASSERT(m_Head != nullptr, "Cannot peek empty list");
        return *m_Tail;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    inline void sj::UnmanagedList<NodeType>::PushFront(NodeType* node)
    {
        if(m_Head == nullptr)
        {
            // On first insert, also set tail
            m_Tail = node;
        }

        node->SetNext(m_Head);
        if constexpr(is_dl_list_node<NodeType>)
        {
            node->SetPrev(nullptr);
        }

        m_Head = node;
        m_Size++;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    inline void UnmanagedList<NodeType>::InsertAfter(NodeType* oldNode, NodeType* newNode)
    {
        NodeType* oldNext = oldNode->GetNext();

        oldNode->SetNext(newNode);
        newNode->SetNext(oldNext);

        if constexpr(is_dl_list_node<NodeType>)
        {
            newNode->SetPrev(oldNode);

            if(oldNext)
            {
                oldNext->SetPrev(newNode);
            }
        }

        if(oldNode == m_Tail)
        {
            m_Tail = newNode;
        }

        m_Size++;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    inline void UnmanagedList<NodeType>::EraseAfter(NodeType* node)
    {
        NodeType* next = node->GetNext();

        if(next)
        {
            NodeType* nextNext = next->GetNext();
            node->SetNext(nextNext);

            if constexpr(is_dl_list_node<NodeType>)
            {
                if(nextNext)
                {
                    nextNext->SetPrev(node);
                }
            }
        }

        m_Size--;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    inline void UnmanagedList<NodeType>::PopFront()
    {
        SJ_ASSERT(m_Head != nullptr, "Cannot pop from empty list.");
        m_Head = m_Head->GetNext();

        if(m_Head == nullptr)
        {
            m_Tail = nullptr;
        }

        m_Size--;
    }

    template <class NodeType> requires(is_list_node<NodeType>) 
    inline void UnmanagedList<NodeType>::PushBack(NodeType* node)
    {
        if(m_Tail == nullptr)
        {
            PushFront(node);
        }
        else
        {
            m_Tail->SetNext(node);
        
            if constexpr(is_dl_list_node<NodeType>)
            {
                node->SetPrev(m_Tail);
            }

            node->SetNext(nullptr);

            m_Tail = node;
        }

        m_Size++;
    }

    template <class NodeType> requires(is_list_node<NodeType>) 
    inline void UnmanagedList<NodeType>::InsertBefore(NodeType* oldNode, NodeType* newNode) 
        requires(is_dl_list_node<NodeType>)
    {
        SJ_ASSERT(oldNode != nullptr, "Cannot insert before null node");
        NodeType* prev = oldNode->GetPrev();

        prev->SetNext(newNode);
        newNode->SetPrev(prev);

        newNode->SetNext(oldNode);
        oldNode->SetPrev(newNode);

        m_Size++;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    inline void UnmanagedList<NodeType>::PopBack() requires(is_dl_list_node<NodeType>)
    {
        SJ_ASSERT(m_Head != nullptr, "Cannot pop from empty list.");

        if(m_Tail == m_Head)
        {
            PopFront();
        }
        else
        {
            m_Tail = m_Tail->GetPrev();
        }
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    inline void UnmanagedList<NodeType>::Erase(NodeType* node) requires(is_dl_list_node<NodeType>)
    {
        NodeType* prev = node->GetPrev(); 
        NodeType* next = node->GetNext();
        
        // Set Prev Next to Next
        if (prev)
        {
            prev->SetNext(next);
        }

        // Set Next Prev to Prev
        if(next)
        {
            next->SetPrev(prev);
        }

        m_Size--;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    inline size_t UnmanagedList<NodeType>::Size() const
    {
        return m_Size;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    inline bool UnmanagedList<NodeType>::IsEmpty() const
    {
        return Size() == 0;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    inline typename UnmanagedList<NodeType>::Iterator UnmanagedList<NodeType>::Begin()
    {
        return Iterator(m_Head);
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    inline typename UnmanagedList<NodeType>::Iterator UnmanagedList<NodeType>::End()
    {
        return Iterator(nullptr);
    }

}