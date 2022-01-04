#include <ScrewjankEngine/containers/UnmanagedForwardList.hpp>

namespace sj
{
    template <is_fwd_list_node NodeType>
    inline NodeType* UnmanagedForwardList<NodeType>::Front()
    {
        return m_Head;
    }

    template <is_fwd_list_node NodeType>
    inline const NodeType* UnmanagedForwardList<NodeType>::Front() const
    {
        return m_Head;
    }

    template <is_fwd_list_node NodeType>
    inline void sj::UnmanagedForwardList<NodeType>::PushFront(NodeType* node)
    {
        node->SetNext(m_Head);
        m_Head = node;
    }

    template <is_fwd_list_node NodeType>
    inline void UnmanagedForwardList<NodeType>::InsertAfter(NodeType* oldNode, NodeType* newNode)
    {
        NodeType* oldNext = oldNode->GetNext();

        oldNode->SetNext(newNode);
        newNode->SetNext(oldNext);
    }

    template <is_fwd_list_node NodeType>
    inline void UnmanagedForwardList<NodeType>::PopFront()
    {
        SJ_ASSERT(m_Head != nullptr, "Cannot pop from empty list.");
        m_Head = m_Head->GetNext();
    }
}