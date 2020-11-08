#pragma once
// STD Headers
#include <initializer_list>

// Library Headers

// Screwjank Headers
#include "core/Assert.hpp"
#include "system/Memory.hpp"
#include "system/Memory.hpp"

namespace sj {

    template <class ForwardList_t>
    class ForwardListConstIterator
    {
        // Iteratory concept
        using iterator_category = std::forward_iterator_tag;

        // Node type definitions
        using node_type = typename const ForwardList_t::node_type;
        using node_pointer = typename const ForwardList_t::node_type*;
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
            ForwardListConstIterator tmp(*this);
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

        /**
         * Constructor
         */
        ForwardList(Allocator* allocator = MemorySystem::GetDefaultAllocator());

        /**
         * Initializer List Construction
         */
        ForwardList(Allocator* allocator, std::initializer_list<T> list);

        /**
         * Copy Constructor
         */
        ForwardList(const ForwardList<T>& other);

        /**
         * Move Constructor
         */
        ForwardList(ForwardList<T>&& other) noexcept;

        /**
         * Destructor
         */
        ~ForwardList();

        /**
         * List assignment operator
         */
        ForwardList<T>& operator=(std::initializer_list<T> list);

        /**
         * Copy assignment operator
         */
        ForwardList<T>& operator=(const ForwardList<T>& other);

        /**
         * Move assignment operator
         */
        ForwardList<T>& operator=(ForwardList<T>&& other);

        /*
         * Place a new element at the head of the list
         */
        void PushFront(const T& value);

        /**
         * Removes element at the front of the list
         * @note Element's real destructor is not called if this is a list with a pointer data type
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
        iterator InsertAfter(iterator pos, const T& value);

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
        iterator EraseAfter(iterator pos);

        /**
         * Clears all elements out of the array, destroys all list nodes
         */
        void Clear();

        /**
         * Read-only access the front of the linked-list
         * @return A reference to the first data value in the list
         */
        const T& Front() const;

        /**
         * Non-const access the front of the linked-list
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

        /** Begin function to allow range-based loop iterator */
        const_iterator begin() const;

        /** Begin function to allow range-based loop iterator */
        const_iterator cbegin() const;

        /** End function to allow range-based loop iteration */
        iterator end();

        /** End function to allow range-based loop iteration */
        const_iterator end() const;

        /** Begin function to allow range-based loop iterator */
        const_iterator cend() const;
    };

    template <class T>
    inline ForwardList<T>::ForwardList(Allocator* allocator)
        : m_Allocator(allocator), m_Head(nullptr)
    {
    }

    template <class T>
    inline ForwardList<T>::ForwardList(Allocator* allocator, std::initializer_list<T> list)
        : ForwardList<T>(allocator)
    {
        for (auto it = list.end() - 1; it >= list.begin(); it--) {
            PushFront(*it);
        }
    }

    template <class T>
    inline ForwardList<T>::ForwardList(const ForwardList<T>& other)
    {
        m_Allocator = other.m_Allocator;
        m_Head = nullptr;

        PushFront(other.Front());

        auto it = begin();
        auto other_it = other.cbegin();
        other_it++;

        for (; other_it != other.cend(); other_it++, it++) {
            InsertAfter(it, *other_it);
        }
    }

    template <class T>
    inline ForwardList<T>::ForwardList(ForwardList<T>&& other) noexcept
    {
        m_Allocator = other.m_Allocator;

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

        for (auto it = list.end() - 1; it >= list.begin(); it--) {
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

        for (; other_it != other.cend(); other_it++, it++) {
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

        for (; other_it != other.cend(); other_it++, it++) {
            InsertAfter(it, std::move(*other_it));
        }

        return *this;
    }

    template <class T>
    inline void ForwardList<T>::PushFront(const T& value)
    {
        m_Head = New<ForwardList<T>::node_type>(m_Allocator, value, m_Head);
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
            New<ForwardList<T>::node_type>(m_Allocator, T(std::forward<Args>(args)...), m_Head);
    }

    template <class T>
    inline typename ForwardList<T>::iterator ForwardList<T>::InsertAfter(iterator pos,
                                                                         const T& value)
    {
        auto new_node = New<ForwardList<T>::node_type>(m_Allocator, value, pos.m_Node->Next);

        pos.m_Node->Next = new_node;

        return ForwardList<T>::iterator(new_node);
    }

    template <class T>
    template <class... Args>
    inline typename ForwardList<T>::iterator ForwardList<T>::EmplaceAfter(iterator pos,
                                                                          Args&&... args)
    {
        auto new_node = New<ForwardList<T>::node_type>(m_Allocator,
                                                       T(std::forward<Args>(args)...),
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

        Delete(m_Allocator, dead_node);
        return iterator(pos.m_Node->Next);
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

} // namespace sj
