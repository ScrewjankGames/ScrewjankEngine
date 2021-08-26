#pragma once
// STD Headers
#include <initializer_list>
#include <utility>

// Screwjank Headers
#include <core/Assert.hpp>
#include <system/Memory.hpp>

namespace sj {

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

      private:
        template <bool tIsConst>
        class ForwardListIteratorBase
        {
            template <bool tOtherIsConst>
            friend class ForwardListIteratorBase;

            // Iteratory concept
            using iterator_category = std::forward_iterator_tag;

            // Node type definitions
            using node_type = std::conditional_t<tIsConst,
                                                 typename const ForwardList::node_type,
                                                 typename ForwardList::node_type>;

            using node_pointer = typename node_type*;
            using node_reference = const node_type&;

            // Value type definitions
            using value_type = std::conditional_t<tIsConst,
                                                  typename const ForwardList::value_type,
                                                  typename ForwardList::value_type>;

            using reference = value_type&;
            using pointer = value_type*;

          public:
            ForwardListIteratorBase(node_pointer node) : m_Node(node) {}

            template <bool tIsConst>
            ForwardListIteratorBase(const ForwardListIteratorBase<false>& other) { m_Node = other.m_Node; }

            template <bool tIsConst>
            ForwardListIteratorBase(ForwardListIteratorBase<false>&& other)
            {
                m_Node = other.m_Node;
                other.m_Node = nullptr;
            }
        
            /** Dereference operator overload */
            [[nodiscard]] reference operator*() const { return m_Node->Data; }

            /** Arrow operator overload */
            [[nodiscard]] pointer operator->() const { return &m_Node->Data; }

            /** Equality comparison operator */
            bool operator==(const ForwardListIteratorBase& other) const { return m_Node == other.m_Node; }

            /** Inequality comparison operator */
            bool operator!=(const ForwardListIteratorBase& other) const { return !(*this == other); }

            /** Pre-increment operator overload */
            ForwardListIteratorBase& operator++()
            {
                m_Node = m_Node->Next;
                return *this;
            }

            /** Post-increment operator overload */
            ForwardListIteratorBase operator++(int)
            {
                ForwardListIteratorBase tmp(*this);
                this->operator++();
                return tmp;
            }

            node_pointer m_Node;
        };

      public:
          
        // iterator type definitions
        using iterator = typename ForwardListIteratorBase<false>;
        using const_iterator = typename ForwardListIteratorBase<true>;

        /**
         * Implicit HeapZone Constructors
         * Uses MemorySystem::CurrentHeapZone to allocate memory
         */
        ForwardList();
        ForwardList(std::initializer_list<T> list);

        /**
         * Constructor
         */
        ForwardList(HeapZone* heap_zone);

        /**
         * Initializer List Construction
         */
        ForwardList(HeapZone* heap_zone, std::initializer_list<T> list);

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
        HeapZone* m_HeapZone;

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

} // namespace sj

// Include inlines
#include <containers/ForwardList.inl>
