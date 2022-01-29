#include <ScrewjankEngine/containers/UnmanagedList.hpp>

namespace sj
{
    template <class NodeType> requires(is_list_node<NodeType>) 
    template <bool IsConst>
    inline UnmanagedList<NodeType>::IteratorBase<IsConst>::IteratorBase(NodeType* node)
        : m_CurrNode(node)
    {
        
    }

    template <class NodeType> requires(is_list_node<NodeType>) 
    template <bool IsConst>
    inline UnmanagedList<NodeType>::IteratorBase<IsConst>& 
        UnmanagedList<NodeType>::IteratorBase<IsConst>::operator++()
    {
        m_CurrNode = m_CurrNode->GetNext();
        return *this;
    }

    template <class NodeType> requires(is_list_node<NodeType>) 
    template <bool IsConst>
    inline UnmanagedList<NodeType>::IteratorBase<IsConst> UnmanagedList<NodeType>::IteratorBase<IsConst>::operator++(int)
    {
        IteratorBase<IsConst> tmp(*this);
        this->operator++();
        return tmp;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
    template <bool IsConst>
    inline UnmanagedList<NodeType>::IteratorBase<IsConst>& UnmanagedList<NodeType>::IteratorBase<IsConst>::operator--() 
        requires is_dl_list_node<NodeType>
    {
        m_CurrNode = m_CurrNode->GetPrev();
        return *this;
    }

    template <class NodeType> requires(is_list_node<NodeType>) 
    template <bool IsConst>
    inline UnmanagedList<NodeType>::IteratorBase<IsConst>& UnmanagedList<NodeType>::IteratorBase<IsConst>::operator--(int)
        requires is_dl_list_node<NodeType>
    {
        IteratorBase<IsConst> tmp(*this);
        this->operator--();
        return tmp;
    }

    template <class NodeType> requires(is_list_node<NodeType>)
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

        m_Size--;
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
    inline size_t UnmanagedList<NodeType>::Size() const
    {
        return m_Size;
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