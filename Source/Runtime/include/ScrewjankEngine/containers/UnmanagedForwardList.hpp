#pragma once

// STD Headers
#include <concepts>

namespace sj
{
    // Bare minimum functionality needed to work with an unmanaged forward list
    template <class T>
    concept is_fwd_list_node = requires(T obj)
    {
        {std::is_same<decltype(obj.GetNext()), T*>::value ==
         true}; // type has member fn GetNext that returns pointer to T
        {obj.SetNext(&obj)}; // type has member fn SetNext that accepts pointer to T
    };

    /**
     * Forward list type that assumes no ownership over the nodes that pass through it
     * Only here to cut down on boilerplate for things I want lists of but are allocated
     * in ways unfriendly to a normal Forward List
     */
    template <is_fwd_list_node NodeType>
    class UnmanagedForwardList
    {
    public:
        UnmanagedForwardList() = default;
        ~UnmanagedForwardList() = default;

        NodeType* Front();
        const NodeType* Front() const;

        void PushFront(NodeType* node);
        void InsertAfter(NodeType* oldNode, NodeType* newNode);

        void PopFront();

    private:
        NodeType* m_Head = nullptr;
    };
}

#include <ScrewjankEngine/containers/UnmanagedForwardList.inl>