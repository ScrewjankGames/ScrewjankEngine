module;

// STD Headers
#include <concepts>

// Screwjank Headers
#include <ScrewjankStd/Assert.hpp>

export module sj.std.containers:unmanaged_list;

export namespace sj
{
    /** Linked list node */
    template <class T>
    concept is_fwd_list_node = requires(T obj) {
        // type must have member fn get_next that returns pointer to T
        { obj.get_next() } -> std::convertible_to<T*>;

        // type must have member fn set_next that accepts pointer to T
        { obj.set_next(std::declval<T*>()) };
    };

    /** Doubly linked list node concept */
    template <class T>
    concept is_dl_list_node = is_fwd_list_node<T> && requires(T obj) {
        // type must have member fn get_prev that returns pointer to T
        { std::is_same_v<decltype(obj.get_prev()), T*> == true };

        // type must have member fn set_prev that accepts pointer to T
        { obj.set_prev(std::declval<T*>()) };
    };

    template <class T>
    concept is_list_node = is_fwd_list_node<T> || is_dl_list_node<T>;

    /**
     * Linked list type that assumes no ownership over the nodes that pass through it.
     * It can accept singly or doubly linked list node types. Some functions are only
     * available for doubly linked list nodes
     */
    template <class NodeType>
        requires(is_list_node<NodeType>)
    class unmanaged_list
    {
    public:
        template <bool tIsConst>
        class IteratorBase
        {
        public:
            using node_type = std::conditional_t<tIsConst, const NodeType, NodeType>;
            using node_ptr = node_type*;
            using node_ref = node_type&;

            IteratorBase(NodeType* node) : m_CurrNode(node)
            {
            }
            /** Allow const iterators to be constructed from non-const ones.*/
            IteratorBase(const IteratorBase<false>& other)
                : IteratorBase(reinterpret_cast<IteratorBase<true>&>(other))
            {
            }

            node_ref operator*()
            {
                return *m_CurrNode;
            }
            node_ptr operator->()
            {
                return m_CurrNode;
            }

            /** Equality comparison */
            bool operator==(const IteratorBase& other) const
            {
                return m_CurrNode == other.m_CurrNode;
            }
            bool operator!=(const IteratorBase& other) const
            {
                return !(m_CurrNode == other.m_CurrNode);
            }

            /** Pre-increment operator overload */
            IteratorBase& operator++()
            {
                m_CurrNode = m_CurrNode->get_next();
                return *this;
            }

            /** Post-increment operator overload */
            IteratorBase operator++(int)
            {
                IteratorBase<tIsConst> tmp(*this);
                this->operator++();
                return tmp;
            }

            /** Pre-Decrement operator overload */
            IteratorBase& operator--()
                requires is_dl_list_node<NodeType>
            {
                m_CurrNode = m_CurrNode->get_prev();
                return *this;
            }

            /** Post-Decrement operator overload */
            IteratorBase operator--(int)
                requires is_dl_list_node<NodeType>
            {
                IteratorBase<tIsConst> tmp(*this);
                this->operator--();
                return tmp;
            }

        private:
            node_ptr m_CurrNode;
        };

        using iterator = IteratorBase<false>;
        using const_iterator = IteratorBase<true>;

        unmanaged_list() = default;
        ~unmanaged_list() = default;

        /**
         * No copy constructor
         * Can't perform deep copies and shallow copies probably won't work too good
         */
        unmanaged_list(const unmanaged_list<NodeType>& other) = delete;

        /**
         * Move Constructor
         */
        unmanaged_list(unmanaged_list<NodeType>&& other) noexcept
            : m_Head(other.m_Head), m_Tail(other.m_Tail), m_Size(other.m_Size)
        {
            other.m_Head = nullptr;
            other.m_Tail = nullptr;
            other.m_Size = 0;
        }

        auto front(this auto&& self) -> auto&& // const? NodeType&
        {
            SJ_ASSERT(self.m_Head != nullptr, "Cannot peek empty list");
            return *self.m_Head;
        }

        auto back() -> auto&& // const? NodeType&
        {
            SJ_ASSERT(m_Head != nullptr, "Cannot peek empty list");
            return *m_Tail;
        }

        void push_front(NodeType* node)
        {
            if(m_Head == nullptr)
            {
                // On first insert, also set tail
                m_Tail = node;
            }

            node->set_next(m_Head);
            if constexpr(is_dl_list_node<NodeType>)
            {
                node->set_prev(nullptr);
            }

            m_Head = node;
            m_Size++;
        }

        void insert_after(NodeType* oldNode, NodeType* newNode)
        {
            NodeType* oldNext = oldNode->get_next();

            oldNode->set_next(newNode);
            newNode->set_next(oldNext);

            if constexpr(is_dl_list_node<NodeType>)
            {
                newNode->set_prev(oldNode);

                if(oldNext)
                {
                    oldNext->set_prev(newNode);
                }
            }

            if(oldNode == m_Tail)
            {
                m_Tail = newNode;
            }

            m_Size++;
        }

        void erase_after(NodeType* node)
        {
            NodeType* next = node->get_next();

            if(next)
            {
                NodeType* nextNext = next->get_next();
                node->set_next(nextNext);

                if constexpr(is_dl_list_node<NodeType>)
                {
                    if(nextNext)
                    {
                        nextNext->set_prev(node);
                    }
                }
            }

            m_Size--;
        }

        void pop_front()
        {
            SJ_ASSERT(m_Head != nullptr, "Cannot pop from empty list.");
            m_Head = m_Head->get_next();

            if(m_Head == nullptr)
            {
                m_Tail = nullptr;
            }

            m_Size--;
        }

        void push_back(NodeType* node)
        {
            if(m_Tail == nullptr)
            {
                push_front(node);
            }
            else
            {
                m_Tail->set_next(node);

                if constexpr(is_dl_list_node<NodeType>)
                {
                    node->set_prev(m_Tail);
                }

                node->set_next(nullptr);

                m_Tail = node;
            }

            m_Size++;
        }

        void insert_before(NodeType* oldNode, NodeType* newNode)
            requires(is_dl_list_node<NodeType>)
        {
            SJ_ASSERT(oldNode != nullptr, "Cannot insert before null node");
            NodeType* prev = oldNode->get_prev();

            prev->set_next(newNode);
            newNode->set_prev(prev);

            newNode->set_next(oldNode);
            oldNode->set_prev(newNode);

            m_Size++;
        }

        void pop_back()
            requires(is_dl_list_node<NodeType>)
        {
            SJ_ASSERT(m_Head != nullptr, "Cannot pop from empty list.");

            if(m_Tail == m_Head)
            {
                pop_front();
            }
            else
            {
                m_Tail = m_Tail->get_prev();
            }
        }

        void erase(NodeType* node)
            requires(is_dl_list_node<NodeType>)
        {
            NodeType* prev = node->get_prev();
            NodeType* next = node->get_next();

            // Set Prev Next to Next
            if(prev)
            {
                prev->set_next(next);
            }

            // Set Next Prev to Prev
            if(next)
            {
                next->set_prev(prev);
            }

            m_Size--;
        }

        [[nodiscard]] size_t size() const
        {
            return m_Size;
        }

        [[nodiscard]] bool empty() const
        {
            return size() == 0;
        }

        iterator begin()
        {
            return iterator(m_Head);
        }

        iterator end()
        {
            return iterator(nullptr);
        }

    private:
        NodeType* m_Head = nullptr;
        NodeType* m_Tail = nullptr;
        size_t m_Size = 0;
    };

} // namespace sj