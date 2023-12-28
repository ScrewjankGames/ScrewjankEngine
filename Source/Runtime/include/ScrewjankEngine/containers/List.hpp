#pragma once
// STD Headers
#include <concepts>
#include <initializer_list>
#include <utility>

// Screwjank Headers
#include <ScrewjankEngine/core/Assert.hpp>
#include <ScrewjankEngine/containers/UnmanagedList.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>

namespace sj 
{
    template <class T, bool tIsBidirectional = false>
    class List
    {
    public:
        template <class T>
        struct FwdListNodeWrapper
        {
            T Data;
            FwdListNodeWrapper* Next = nullptr;

            FwdListNodeWrapper() = default;

            FwdListNodeWrapper(const T& data, FwdListNodeWrapper* next = nullptr) : Data(data), Next(next)
            {
            }

            FwdListNodeWrapper(T&& data, FwdListNodeWrapper* next = nullptr)
                : Data(std::move(data)), Next(next)
            {
            }

            FwdListNodeWrapper* GetNext()
            {
                return Next;
            }

            const FwdListNodeWrapper* GetNext() const
            {
                return Next;
            }

            void SetNext(FwdListNodeWrapper* next)
            {
                Next = next;
            }
        };

        template <class T>
        struct BiDiListNodeWrapper
        {
            T Data;
            BiDiListNodeWrapper* Next = nullptr;
            BiDiListNodeWrapper* Prev = nullptr;

            BiDiListNodeWrapper() = default;

            BiDiListNodeWrapper(const T& data,
                                BiDiListNodeWrapper* next = nullptr,
                                BiDiListNodeWrapper* prev = nullptr)
                : Data(data), Next(next)
            {
            }

            BiDiListNodeWrapper(T&& data,
                                BiDiListNodeWrapper* next = nullptr,
                                BiDiListNodeWrapper* prev = nullptr)
                : Data(std::move(data)), Next(next)
            {
            }

            BiDiListNodeWrapper* GetNext()
            {
                return Next;
            }

            const BiDiListNodeWrapper* GetNext() const
            {
                return Next;
            }

            void SetNext(BiDiListNodeWrapper* next)
            {
                Next = next;
            }

            BiDiListNodeWrapper* GetPrev()
            {
                return Prev;
            }
            BiDiListNodeWrapper* SetPrev(BiDiListNodeWrapper* prev)
            {
                Prev = prev;
            }
        };

        using node_type = std::conditional<tIsBidirectional, BiDiListNodeWrapper<T>, FwdListNodeWrapper<T>>::type;      
        using node_ptr = std::conditional<tIsBidirectional, BiDiListNodeWrapper<T>*, FwdListNodeWrapper<T>*>::type;
        using const_node_ptr = std::conditional<tIsBidirectional, const BiDiListNodeWrapper<T>*, const FwdListNodeWrapper<T>*>::type;

        template <bool tIsConst>
        class IteratorBase
        {
        public:
            template <bool tConstness>
            friend class IteratorBase;

            using iter_node_ptr = std::conditional<tIsConst, const_node_ptr, node_ptr>::type;
            using data_ptr = std::conditional<tIsConst, const T*, T*>::type;
            using data_ref = std::conditional<tIsConst, const T&, T&>::type;

            IteratorBase(node_ptr node);

            IteratorBase(const_node_ptr node) requires tIsConst;

            IteratorBase(nullptr_t null);

            /** Allows iterators of either type to be constructed from non-const iterator */
            IteratorBase(const IteratorBase<false>& other);

            data_ref operator*();
            data_ptr operator->();

            node_ptr GetNode();

            /** Pre-increment operator overload */
            IteratorBase& operator++();

            /** Post-increment operator overload */
            IteratorBase operator++(int);

            /** Pre-Decrement operator overload */
            IteratorBase& operator--() requires is_dl_list_node<node_type>;

            /** Post-Decrement operator overload */
            IteratorBase operator--(int) requires is_dl_list_node<node_type>;

            bool operator==(const IteratorBase& other) const;
            bool operator!=(const IteratorBase& other) const;

        private: 
            iter_node_ptr m_CurrNode;
        };

        // iterator type definitions
        using Iterator = IteratorBase<false>;
        using ConstIterator = IteratorBase<true>;

        /**
         * Implicit HeapZone Constructors
         * Uses MemorySystem::CurrentHeapZone to allocate memory
         */
        List();
        List(std::initializer_list<T> list);

        /**
         * Constructor
         */
        List(HeapZoneBase* heap_zone);

        /**
         * Initializer List Construction
         */
        List(HeapZoneBase* heap_zone, std::initializer_list<T> list);

        /**
         * Copy constructor 
         */
        List(const List<T, tIsBidirectional>& other);

        /**
         * Destructor
         */
        ~List();

        /**
         * Copy assignment operator 
         */
        List<T, tIsBidirectional>& operator=(const List<T, tIsBidirectional>& list);

        /**
         * List assignment operator
         */
        List<T, tIsBidirectional>& operator=(std::initializer_list<T> list);

        /*
         * Place a new element at the head of the list
         */
        void PushFront(const T& value);

        /**
         * Place a new element at the tail of the list 
         */
        void PushBack(const T& value);

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
        Iterator InsertAfter(Iterator pos, const T& value);

        /**
         * Constructs and inserts a value into the linked list after the given position
         */
        template <class... Args>
        Iterator EmplaceAfter(Iterator pos, Args&&... args);

        /**
         * Erases the element after pos
         * @return Iterator to the element following the erased one, or end() if no such element
         * exists.
         */
        Iterator EraseAfter(Iterator pos);

        /**
         * Clears all elements out of the array, destroys all list nodes
         */
        void Clear();

        /**
         * Non-const access the front of the linked-list
         * @return A reference to the first data value in the list
         */
        T& Front();

        /**
         * Read-only access the front of the linked-list
         * @return A reference to the first data value in the list
         */
        const T& Front() const;

        /**
         * Access to last element in list 
         */
        T& Back();
        const T& Back() const;

        /**
         * Access iterator of last element in the list 
         */
        Iterator BackIter();
        ConstIterator BackIter() const;

        /**
         * @return Size() == 0? 
         */
        bool IsEmpty() const;

        /**
         * @return Number of elements in container 
         */
        size_t Size() const;

      private:
        unmanaged_list<node_type> m_List;

        /** Allocator used to service allocations for container structures */
        HeapZoneBase* m_HeapZone;

      public:
        /** Begin function to allow range-based loop iterator */
        Iterator begin();

        /** Begin function to allow range-based loop iterator */
        ConstIterator begin() const;

        /** End function to allow range-based loop iteration */
        Iterator end();

        /** End function to allow range-based loop iteration */
        ConstIterator end() const;
    };

} // namespace sj

// Include inlines
#include <ScrewjankEngine/containers/List.inl>
