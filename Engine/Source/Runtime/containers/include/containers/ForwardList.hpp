#pragma once
// STD Headers
#include <initializer_list>

// Library Headers

// Screwjank Headers
#include "core/Assert.hpp"
#include "core/MemorySystem.hpp"

namespace Screwjank {

    template <class ForwardList_t>
    class ForwardListConstIterator
    {
        // Iteratory concept
        using iterator_category = std::forward_iterator_tag;

        // Node type definitions
        using node_type = typename const ForwardList_t::node_type;
        using node_pointer = typename const ForwardList_t::node_pointer;
        using node_reference = const node_type&;

        // Value type definitions
        using value_type = typename const ForwardList_t::value_type;
        using reference = typename const value_type&;
        using pointer = const value_type*;

      public:
        ForwardListConstIterator(node_pointer node) : m_Node(node)
        {
        }

        /** Dereference operator overload */
        [[nodiscard]] reference operator*() const
        {
            return m_Node->Data;
        }

        /** Arrow operator overload */
        [[nodiscard]] pointer operator->() const
        {
            return &m_Node->Data;
        }

        /** Equality comparison operator */
        bool operator==(const ForwardListConstIterator& other) const
        {
            return m_Node == other.m_Node;
        }

        /** Inequality comparison operator */
        bool operator!=(const ForwardListConstIterator& other) const
        {
            return !(*this == other);
        }

        /** Pre-increment operator overload */
        ForwardListConstIterator& operator++()
        {
            m_Node = m_Node->Next;
            return *this;
        }

        /** Post-increment operator overload */
        ForwardListConstIterator operator++(int)
        {
            ForwardListIterator tmp(*this);
            this->operator++();
            return tmp;
        }

        node_pointer m_Node;
    };

    template <class ForwardList_t>
    class ForwardListIterator
    {
        // Iteratory concept
        using iterator_category = std::forward_iterator_tag;

        // Node type definitions
        using node_type = typename ForwardList_t::node_type;
        using node_pointer = typename ForwardList_t::node_pointer;
        using node_reference = node_type&;

        // Value type definitions
        using value_type = typename ForwardList_t::value_type;
        using reference = typename value_type&;
        using pointer = value_type*;

      public:
        ForwardListIterator(node_pointer node) : m_Node(node)
        {
        }

        /** Dereference operator overload */
        [[nodiscard]] reference operator*() const
        {
            return m_Node->Data;
        }

        /** Arrow operator overload */
        [[nodiscard]] pointer operator->() const
        {
            return &m_Node->Data;
        }

        /** Equality comparison operator */
        bool operator==(const ForwardListIterator& other) const
        {
            return m_Node == other.m_Node;
        }

        /** Inequality comparison operator */
        bool operator!=(const ForwardListIterator& other) const
        {
            return !(*this == other);
        }

        /** Pre-increment operator overload */
        ForwardListIterator& operator++()
        {
            m_Node = m_Node->Next;
            return *this;
        }

        /** Post-increment operator overload */
        ForwardListIterator operator++(int)
        {
            ForwardListIterator tmp(*this);
            this->operator++();
            return tmp;
        }

        node_pointer m_Node;
    };

    template <class T>
    struct ForwardListNode
    {
        /** Constructor */
        ForwardListNode(const T& data, ForwardListNode* next = nullptr) : Data(data), Next(next)
        {
        }

        bool operator==(const ForwardListNode& other) const
        {
            return Data == other.Data && Next = other.Next;
        }

        T Data;
        ForwardListNode* Next;
    };

    template <class T>
    class ForwardList
    {
      public:
        // Node type definitions
        using node_type = ForwardListNode<T>;
        using node_pointer = node_type*;
        using node_reference = node_type&;
        using const_node_pointer = const node_type*;
        using const_node_reference = const node_type&;

        // Value type definitions
        using value_type = T;
        using reference = value_type&;
        using const_reference = const reference;
        using pointer = value_type*;
        using difference_type = ptrdiff_t;
        using size_type = size_t;

        // iterator type definitions
        friend class ForwardListIterator<T>;
        using iterator = typename ForwardListIterator<ForwardList<T>>;
        using const_iterator = typename ForwardListConstIterator<ForwardList<T>>;

        /** Constructor */
        ForwardList(Allocator* allocator = MemorySystem::GetDefaultAllocator());

        /** Destructor */
        ~ForwardList();

        /** Place a new element at the head of the list */
        void PushFront(const T& value);

        /**
         * Removes element at the front of the list
         * @note Element's destructor is not called if this is a list with a pointer data type
         */
        void PopFront();

        /**
         * Constructs and inserts element an the front of the list
         * @param args...  Arguments to be forwarded to value_types's constructor
         */
        template <class... Args>
        void EmplaceFront(Args&&... args);

        /**
         * Inserts a value into the linked list after the given position (pos->Next = value)
         * @return Iterator to the newly inserted value
         */
        iterator InsertAfter(const_iterator pos, const T& value);

        /**
         * Inserts a value into the linked list after the given position (pos->Next = value)
         * @return Iterator to the newly inserted value
         */
        iterator InsertAfter(iterator pos, const T& value);

        /**
         * Constructs and inserts a value into the linked list after the given position
         */
        template <class... Args>
        iterator EmplaceAfter(const_iterator pos, Args&&... args);

        /**
         * Constructs and inserts a value into the linked list after the given position
         */
        template <class... Args>
        iterator EmplaceAfter(iterator pos, Args&&... args);

        /**
         * Erases the element after pos
         * @return Iterator to the element following the erased one, or end() if no such element
         * exists.
         */
        iterator EraseAfter(const_iterator pos);

        /**
         * Erases the element after pos
         * @return Iterator to the element following the erased one, or end() if no such element
         * exists.
         */
        iterator EraseAfter(iterator pos);

        /**
         * Clears all elements out of the array, destroys all list nodes
         */
        void Clear();

        /**
         * Access the front of the linked-list
         * @return A reference to the first data value in the list
         */
        T& Front();

      private:
        /** Pointer to the head of the list */
        node_pointer m_Head;

        /** Allocator used to service allocations for container structures */
        Allocator* m_Allocator;

      public:
        /** Begin function to allow range-based loop iterator */
        iterator begin();

        /** End function to allow range-based loop iteration */
        iterator end();
    };

    template <class T>
    inline ForwardList<T>::ForwardList(Allocator* allocator)
        : m_Allocator(allocator), m_Head(nullptr)
    {
    }

    template <class T>
    inline ForwardList<T>::~ForwardList()
    {
        Clear();
    }

    template <class T>
    inline void ForwardList<T>::PushFront(const T& value)
    {
        m_Head = m_Allocator->New<ForwardList<T>::node_type>(value, m_Head);
    }

    template <class T>
    inline void ForwardList<T>::PopFront()
    {
        SJ_ASSERT(m_Head != nullptr, "Cannot pop from empty list.");

        auto tmp = m_Head;
        m_Head = m_Head->Next;

        m_Allocator->Free(tmp);
    }

    template <class T>
    template <class... Args>
    inline void ForwardList<T>::EmplaceFront(Args&&... args)
    {
        m_Head =
            m_Allocator->New<ForwardList<T>::node_type>(T(std::forward<Args>(args)...), m_Head);
    }

    template <class T>
    inline typename ForwardList<T>::iterator ForwardList<T>::InsertAfter(const_iterator pos,
                                                                         const T& value)
    {
        auto new_node = m_Allocator->New<ForwardList<T>::node_type>(value, pos.m_Node->Next);

        pos.m_Node->Next = new_node;

        return ForwardList<T>::iterator(new_node);
    }

    template <class T>
    inline typename ForwardList<T>::iterator ForwardList<T>::InsertAfter(iterator pos,
                                                                         const T& value)
    {
        return InsertAfter(const_iterator(pos.m_Node), value);
    }

    template <class T>
    template <class... Args>
    inline typename ForwardList<T>::iterator ForwardList<T>::EmplaceAfter(const_iterator pos,
                                                                          Args&&... args)
    {
        auto new_node = m_Allocator->New<ForwardList<T>::node_type>(T(std::forward<Args>(args)...),
                                                                    pos.m_Node->Next);
        pos.m_Node->Next = new_node;
        return ForwardList<T>::iterator(new_node);
    }

    template <class T>
    template <class... Args>
    inline typename ForwardList<T>::iterator ForwardList<T>::EmplaceAfter(iterator pos,
                                                                          Args&&... args)
    {
        return EmplaceAfter(const_iterator(pos.m_Node), std::forward<Args>(args)...);
    }

    template <class T>
    inline typename ForwardList<T>::iterator ForwardList<T>::EraseAfter(const_iterator pos)
    {
        SJ_ASSERT(pos.m_Node->Next != nullptr, "Cannot erase after last element of list");

        auto dead_node = pos.m_Node->Next;
        pos.m_Node->Next = dead_node->Next;

        m_Allocator->Delete(dead_node);
        return iterator(pos.m_Node->Next);
    }

    template <class T>
    inline typename ForwardList<T>::iterator ForwardList<T>::EraseAfter(iterator pos)
    {
        return EraseAfter(const_iterator(pos.m_Node));
    }

    template <class T>
    inline void ForwardList<T>::Clear()
    {
        if (m_Head == nullptr) {
            return;
        }

        // Free all the nodes allocated for the list
        auto curr = m_Head;
        while (curr->Next != nullptr) {
            auto tmp = curr->Next;
            m_Allocator->Free(curr);
            curr = tmp;
        }

        m_Head = nullptr;
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
    inline typename ForwardList<T>::iterator ForwardList<T>::end()
    {
        return iterator(nullptr);
    }

} // namespace Screwjank
