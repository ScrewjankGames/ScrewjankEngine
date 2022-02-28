#pragma once

// STD Headers
#include <concepts>

// Screwjank Headers
#include <ScrewjankEngine/core/Assert.hpp>

namespace sj
{
    /** Linked list node */
    template <class T>
    concept is_fwd_list_node = requires(T obj)
    {
        // type must have member fn GetNext that returns pointer to T
        { obj.GetNext() } -> std::convertible_to<T*>; 

        // type must have member fn SetNext that accepts pointer to T
        {obj.SetNext(std::declval<T*>())}; 
    };

    /** Doubly linked list node concept */
    template <class T>
    concept is_dl_list_node = is_fwd_list_node<T> && requires(T obj)
    {
        // type must have member fn GetPrev that returns pointer to T
        { std::is_same<decltype(obj.GetPrev()), T*>::value == true }; 

        // type must have member fn SetPrev that accepts pointer to T 
        { obj.SetPrev(std::declval<T*>()) }; 
    };

    template<class T>
    concept is_list_node = is_fwd_list_node<T> || is_dl_list_node<T>;

    /**
     * Linked list type that assumes no ownership over the nodes that pass through it.
     * It can accept singly or doubly linked list node types. Some functions are only
     * available for doubly linked list nodes
     */
    template <class NodeType> requires(is_list_node<NodeType>)
    class UnmanagedList
    {
    public:
        template<bool tIsConst>
        class IteratorBase
        {
        public:
            using node_type = std::conditional<tIsConst, const NodeType, NodeType>::value;
            using node_ptr = node_type*;
            using node_ref = node_type&;

            IteratorBase(NodeType* node);
            /** Allow const iterators to be constructed from non-const ones.*/
            IteratorBase(const IteratorBase<false>& other);

            node_ref operator*();
            node_ptr operator->();

            /** Pre-increment operator overload */
            IteratorBase& operator++();

            /** Post-increment operator overload */
            IteratorBase operator++(int);

            /** Pre-Decrement operator overload */
            IteratorBase& operator--() requires is_dl_list_node<NodeType>;

            /** Post-Decrement operator overload */
            IteratorBase operator--(int) requires is_dl_list_node<NodeType>;

        private:
            node_ptr m_CurrNode;
        };

        using Iterator = IteratorBase<false>;
        using ConstIterator = IteratorBase<true>;

        UnmanagedList() = default;
        ~UnmanagedList() = default;

        /**
         * Can't perform deep copies and shallow copies probably won't work too good
         */
        UnmanagedList(const UnmanagedList<NodeType>& other) = delete;

        NodeType& Front();
        const NodeType& Front() const;

        NodeType& Back();
        const NodeType& Back() const;

        void PushFront(NodeType* node);
        void InsertAfter(NodeType* oldNode, NodeType* newNode);
        void EraseAfter(NodeType* node);
        void PopFront();

        void PushBack(NodeType* node);
        void InsertBefore(NodeType* oldNode,
                          NodeType* newNode) requires(is_dl_list_node<NodeType>);

        void PopBack() requires(is_dl_list_node<NodeType>);

        size_t Size() const;
        bool IsEmpty() const;

        Iterator Begin();
        Iterator End();

        Iterator begin() { return Begin(); }
        Iterator end() { return End(); }

    private:
        NodeType* m_Head = nullptr;
        NodeType* m_Tail = nullptr;
        size_t m_Size = 0;
    };

} // namespace sj

#include <ScrewjankEngine/containers/UnmanagedList.inl>