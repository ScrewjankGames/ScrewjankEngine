module;

// STD Headers
#include <concepts>

// Screwjank Headers
#include <ScrewjankStd/Assert.hpp>

export module sj.std.containers.unmanaged_list;

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
        struct end_of_list_sentinel
        {
        };

        template <bool tIsConst>
        class iterator_base
        {
        public:
            using difference_type = std::ptrdiff_t;
            using element_type = std::conditional_t<tIsConst, const NodeType, NodeType>;
            using pointer = element_type*;
            using reference = element_type&;

            template <bool tAnyConst>
            friend class iterator_base;

            iterator_base(end_of_list_sentinel _) : m_CurrNode(nullptr)
            {
            }

            iterator_base(NodeType* node) : m_CurrNode(node)
            {
            }

            /** Allow const iterators to be constructed from non-const ones.*/
            iterator_base(const iterator_base<false>& other) : iterator_base(other.m_CurrNode)
            {
            }

            auto operator*(this auto&& self) -> auto&& // reference or const_reference
            {
                return *(std::forward<decltype(self)>(self).m_CurrNode);
            }

            auto operator->(this auto&& self) -> auto&& // pointer or const_pointer
            {
                return std::forward<decltype(self)>(self).m_CurrNode;
            }

            /** Equality comparison */
            bool operator==(const iterator_base& other) const
            {
                return m_CurrNode == other.m_CurrNode;
            }

            /** Pre-increment operator overload */
            iterator_base& operator++()
            {
                m_CurrNode = m_CurrNode->get_next();
                return *this;
            }

            /** Post-increment operator overload */
            iterator_base operator++(int)
            {
                iterator_base<tIsConst> tmp(*this);
                this->operator++();
                return tmp;
            }

            /** Pre-Decrement operator overload */
            iterator_base& operator--()
                requires is_dl_list_node<NodeType>
            {
                m_CurrNode = m_CurrNode->get_prev();
                return *this;
            }

            /** Post-Decrement operator overload */
            iterator_base operator--(int)
                requires is_dl_list_node<NodeType>
            {
                iterator_base<tIsConst> tmp(*this);
                this->operator--();
                return tmp;
            }

            // Check whether the associated iterator is done.
            friend bool operator==(iterator_base<tIsConst> iter, end_of_list_sentinel _)
            {
                return iter.m_CurrNode == nullptr;
            }

        private:
            pointer m_CurrNode;
        };

        using iterator = iterator_base<false>;
        using const_iterator = iterator_base<true>;
        static_assert(std::input_or_output_iterator<iterator>);
        static_assert(std::input_or_output_iterator<const_iterator>);

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
            : m_Head(other.m_Head), m_Tail(other.m_Tail)
        {
            other.m_Head = nullptr;
            other.m_Tail = nullptr;
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
            if(empty())
            {
                m_Head = node;
                m_Tail = node;
            }
            else
            {
                node->set_next(m_Head);
                if constexpr(is_dl_list_node<NodeType>)
                {
                    node->set_prev(nullptr);
                    m_Head->set_prev(node);
                }

                m_Head = node;
            }
        }

        void push_back(NodeType* node)
        {
            if(empty())
            {
                m_Head = node;
                m_Tail = node;
            }
            else
            {
                m_Tail->set_next(node);
                node->set_next(nullptr);

                if constexpr(is_dl_list_node<NodeType>)
                {
                    node->set_prev(m_Tail);
                }

                m_Tail = node;
            }
        }

        iterator insert(iterator pos, NodeType* value)
        {
            insert_before(pos, value);
            return iterator(value);
        }

        void insert_before(iterator old_node, NodeType* new_node)
            requires(is_dl_list_node<NodeType>)
        {
            if(old_node == end())
            {
                push_back(new_node);
                return;
            }
            else if(old_node == begin())
            {
                push_front(new_node);
                return;
            }
            else
            {
                NodeType* prev = old_node->get_prev();

                prev->set_next(new_node);
                new_node->set_prev(prev);

                new_node->set_next(&(*old_node));
                old_node->set_prev(new_node);
            }
        }

        void insert_after(iterator old_node, NodeType* new_node)
        {
            SJ_ASSERT(old_node != end(), "Cannot insert after end of list");

            if(old_node == iterator(&back()))
            {
                push_back(new_node);
            }
            else
            {
                NodeType* old_next = old_node->get_next();

                old_node->set_next(new_node);
                new_node->set_next(old_next);

                if constexpr(is_dl_list_node<NodeType>)
                {
                    new_node->set_prev(old_node);

                    if(old_next)
                        old_next->set_prev(new_node);
                }
            }
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
        }

        void pop_front()
        {
            SJ_ASSERT(!empty(), "Cannot pop from empty list");
            m_Head = m_Head->get_next();
            
            if constexpr (is_dl_list_node<NodeType>)
            {
                if(m_Head != nullptr)
                    m_Head->set_prev(nullptr);
            }

            if(m_Head == nullptr)
                m_Tail = nullptr;
        }

        void pop_back()
            requires(is_dl_list_node<NodeType>)
        {
            SJ_ASSERT(m_Head != nullptr, "Cannot pop from empty list.");
            m_Tail = m_Tail->get_prev();
            if(m_Tail == nullptr)
                m_Head = nullptr;
            else
                m_Tail->set_next(nullptr);
        }

        iterator erase(const_iterator pos)
            requires(is_dl_list_node<NodeType>)
        {
            SJ_ASSERT(!empty(), "Cannot erase from empty list");
            SJ_ASSERT(pos != end(), "Cannot erase past end of list");

            if(pos == begin())
            {
                pop_front();
                return begin();
            }
            else if(pos == const_iterator(&back()))
            {
                pop_back();
                return iterator(end());
            }
            else
            {
                NodeType* prev = pos->get_prev();
                NodeType* next = pos->get_next();

                // Set Prev Next to Next
                if(prev)
                    prev->set_next(next);

                // Set Next Prev to Prev
                if(next)
                    next->set_prev(prev);

                return iterator(next);
            }
        }

        [[nodiscard]] bool empty() const
        {
            return begin() == end();
        }

        iterator begin()
        {
            return iterator(m_Head);
        }

        const_iterator begin() const
        {
            return iterator(m_Head);
        }

        auto end(this auto&& _) -> end_of_list_sentinel
        {
            return {};
        }

    private:
        NodeType* m_Head = nullptr;
        NodeType* m_Tail = nullptr;
    };

} // namespace sj