#pragma once

// STD Headers
#include <concepts>

// Library Headers

// Screwjank Headers
#include <core/Assert.hpp>

namespace sj {

    struct Nullopt_t
    {
        struct _NulloptTag
        {
        };

        constexpr explicit Nullopt_t(_NulloptTag)
        {
        }
    };

    inline constexpr Nullopt_t NullOpt {Nullopt_t::_NulloptTag {}};

    template<class T>
	class Optional
    {
      public:
        /**
         * Default Constructor
         */
        constexpr Optional() noexcept;

        /**
         * Constructor
         * @param nullopt The type tag used to denote the container should hold no value
         */
        constexpr Optional(Nullopt_t nullopt) noexcept;

        /**
         * Constructor
         * @param value The value to initialize the optional with
         */
        template<class U = T>
        requires std::convertible_to<U, T>
        constexpr Optional(U&& value);

        /**
         * Copy Constructor
         */
        constexpr Optional(const Optional& other);

        /**
         * Copy Constructor 
         */
        template <class U = T>
        requires std::convertible_to<U, T>
        Optional(const Optional<U>& other) noexcept;

        /**
         * Move Constructor  
         */
        template <class U = T>
        requires std::convertible_to<U, T>
        Optional(Optional<U>&& other) noexcept;

        /**
         * Destructor 
         */
        ~Optional();

        /**
         * Copy assignment operator  
         */
        constexpr Optional& operator=(const Optional& other) noexcept;

        /**
         * Move assignment operator
         */
        constexpr Optional& operator=(Optional&& other) noexcept;

        /**
         * Copy assignment operator  
         */
        template<class U>
        requires std::convertible_to<U, T>
        Optional& operator=(const Optional<U>& other) noexcept;

        /**
         * Move assignment operator
         */
        template<class U>
        requires std::convertible_to<U, T>
        Optional& operator=(Optional<U>&& other) noexcept;

        /**
         * Resets the Optional to contain no value
         * @param nullopt The NullOpt object
         */
        Optional& operator=(Nullopt_t nullopt) noexcept; 

        /**
         * Value assignment operator 
         */
        template <class U = T>
        requires std::convertible_to<U, T>
        Optional& operator=(U&& value) noexcept;

        /**
         * Allows optional to be implicitly converted to a boolean (like a pointer)
         * @return value of m_HasValue
         */
        constexpr explicit operator bool() const noexcept;

        /**
         * Arrow operator overload
         */
        constexpr const T* operator->() const;
        
        /**
         * Arrow operator overload
         */
        constexpr T* operator->();


        /**
         *  Dereference from const l-value operator overload
         */
        constexpr const T& operator*() const&;

        /**
         *  Dereference from l-value operator overload
         */
        constexpr T& operator*() &;

        /**
         *  Dereference from const r-value operator overload
         */
        constexpr const T&& operator*() const&&;

        /**
         *  Dereference from l-value operator overload
         */
        constexpr T&& operator*() &&;

        /**
         * @return Whether or not the Optional has a value 
         */
        constexpr bool HasValue() const noexcept;

        /**
         * Used for extracting Value from a non-const lvalue Optional
         * @note Asserts when HasValue() is false
         * @return m_Value
         */
        constexpr T& Value() &;

        /**
         * Used for extracting Value from a const lvalue Optional
         * @note Asserts when HasValue() is false
         * @return m_Value
         */
        constexpr const T& Value() const &;

        /**
         * Used for extracting Value as rvalue reference a non-const rvalue Optional
         * @note Asserts when HasValue() is false
         * @return m_Value
         */
        constexpr T&& Value() &&;

        /**
         * Used for extracting Value as rvalue reference a const rvalue Optional
         * @note Asserts when HasValue() is false
         * @return m_Value
         */
        constexpr const T&& Value() const&&;

        /**
         * Returns the value of the optional, or the specified alternative value 
         * @param default_value The default value to be returned if the Optional is empty
         */
        constexpr T ValueOr(T&& default_value) const;

        /**
         * Swaps the contents of this optional with another
         * @param other The other optional to swap with
         */
        void Swap(Optional& other) noexcept;

      private:
        union
        {
            T m_Value;
        };

        bool m_HasValue;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// Non member functions and operations
    ///////////////////////////////////////////////////////////////////////////////////////////////


    /**
     * Swaps the contents of two optionals
     */
    template<class T>
    constexpr inline void Swap(Optional<T>& left, Optional<T>& right)
    {
        left.Swap(right);
    }

    template<class T>
    constexpr inline bool operator==(const Optional<T>& lhs, const Optional<T>& rhs)
    {
        return (lhs.HasValue() && rhs.HasValue()) ? *lhs == *rhs : false;
    }

    template <class T>
    constexpr inline bool operator==(const T& lhs, const Optional<T>& rhs)
    {
        return (rhs.HasValue()) ? lhs == *rhs : false;
    }

    template <class T>
    constexpr inline bool operator==(const Optional<T>& lhs, const T& rhs)
    {
        return rhs == lhs;
    }

    template <class T>
    constexpr inline bool operator==(const Optional<T>& lhs, Nullopt_t)
    {
        return lhs.HasValue() == false;
    }

    template <class T>
    constexpr inline bool operator==(Nullopt_t, const Optional<T>& rhs)
    {
        return rhs.HasValue() == false;
    }


    template <class T>
    constexpr inline bool operator!=(const Optional<T>& lhs, const Optional<T>& rhs)
    {
        return !(lhs == rhs);
    }

    template <class T>
    constexpr inline bool operator!=(T& lhs, const Optional<T>& rhs)
    {
        return (rhs.HasValue()) ? lhs != *rhs : true;
    }

    template <class T>
    constexpr inline bool operator!=(const Optional<T>& lhs, T& rhs)
    {
        return rhs != lhs;
    }

    template <class T>
    constexpr inline auto operator<=>(const Optional<T>& lhs, const Optional<T>& rhs)
        -> decltype(std::declval<T>() <=> std::declval<T>())
    {
        return (lhs.HasValue() && rhs.HasValue()) ? *lhs <=> *rhs : bool(lhs) <=> bool(rhs);
    }

    template <class T>
    constexpr inline auto operator<=>(const Optional<T>& lhs, T& rhs)
        -> decltype(std::declval<T>() <=> std::declval<T>())
    {
        return (lhs.HasValue()) ? *lhs <=> rhs : std::strong_ordering::less;
    }

    template<class T>
    inline constexpr Optional<T>::Optional() noexcept : m_HasValue(false)
    {

    }

    template <class T>
    inline constexpr sj::Optional<T>::Optional(Nullopt_t nullopt) noexcept : m_HasValue(false)
    {

    }

    template <class T>
    template <class U>
    requires std::convertible_to<U, T>
    inline constexpr Optional<T>::Optional(U&& value)
    {
        m_HasValue = true;

        new (std::addressof(m_Value)) T(std::move(value));
    }

    template <class T>
    inline constexpr Optional<T>::Optional(const Optional<T>& other)
    {
        m_HasValue = other.HasValue();

        if (m_HasValue)
        {
            new (std::addressof(m_Value)) T(other.Value());
        }
    }

    template <class T>
    template <class U>
    requires std::convertible_to<U, T>
    inline Optional<T>::Optional(const Optional<U>& other) noexcept
    {
        m_HasValue = other.HasValue();

        if (m_HasValue) 
        {
            new (std::addressof(m_Value)) T(other.Value());
        }
    }

    template <class T>
    template <class U>
    requires std::convertible_to<U, T>
    inline Optional<T>::Optional(Optional<U>&& other) noexcept
    {
        m_HasValue = other.HasValue();

        if (m_HasValue) 
        {
            new (std::addressof(m_Value)) T(std::move(other.Value()));
        }
    }

    template <class T>
    inline Optional<T>::~Optional()
    {
        if (m_HasValue) 
        {
            m_Value.~T();
        }
    }

    template <class T>
    inline constexpr Optional<T>& Optional<T>::operator=(const Optional& other) noexcept
    {
        if (other.HasValue())
        {
            if (m_HasValue)
            {
                m_Value = other.m_Value;
            }
            else
            {
                m_HasValue = true;
                new (std::addressof(m_Value)) T(other.m_Value);
            }
        }
        else
        {
            if (m_HasValue)
            {
                m_HasValue.~T();
            }

            m_HasValue = false;
        }

        return *this;
    }

    template <class T>
    inline constexpr Optional<T>& Optional<T>::operator=(Optional&& other) noexcept
    {
        if (other.HasValue())
        {
            if (m_HasValue)
            {
                m_Value = std::move(other.m_Value);
            }
            else
            {
                m_HasValue = true;
                new (std::addressof(m_Value)) T(std::move(other.m_Value));
            }
        }
        else
        {
            if (m_HasValue)
            {
                m_HasValue.~T();
            }

            m_HasValue = false;
        }
    }

    template <class T>
    template <class U>
    requires std::convertible_to<U, T>
    inline Optional<T>& Optional<T>::operator=(const Optional<U>& other) noexcept
    {
        if (other.HasValue())
        {
            if (m_HasValue)
            {
                m_Value = other.m_Value;
            }
            else
            {
                m_HasValue = true;
                new (std::addressof(m_Value)) T(other.m_Value);
            }
        }
        else
        {
            if (m_HasValue)
            {
                m_HasValue.~T();
            }

            m_HasValue = false;
        }

        return *this;
    }

    template <class T>
    template <class U>
    requires std::convertible_to<U, T>
    inline Optional<T>& Optional<T>::operator=(Optional<U>&& other) noexcept
    {
        if (other.HasValue())
        {
            if (m_HasValue)
            {
                m_Value = std::move(other.m_Value);
            }
            else
            {
                m_HasValue = true;
                new (std::addressof(m_Value)) T(std::move(other.m_Value));
            }
        }
        else
        {
            if (m_HasValue)
            {
                m_HasValue.~T();
            }

            m_HasValue = false;
        }
    }
    
    template <class T>
    inline Optional<T>& Optional<T>::operator=(Nullopt_t) noexcept
    {
        m_HasValue = false;
        return *this;
    }

    template <class T>
    template <class U>
    requires std::convertible_to<U, T>
    inline Optional<T>& Optional<T>::operator=(U&& value) noexcept
    {
        if (HasValue())
        {
            m_Value = std::forward<T>((T)value);
        }
        else
        {
            m_HasValue = true;
            new (std::addressof(m_Value)) T(std::forward<T>((T)value));
        }

        return *this;
    }
    
    template <class T>
    inline constexpr Optional<T>::operator bool() const noexcept
    {
        return m_HasValue;
    }

    template <class T>
    inline constexpr const T* Optional<T>::operator->() const
    {
        SJ_ASSERT(HasValue(), "Optional value accessed with no value");
        return &m_Value;
    }

    template <class T>
    inline constexpr T* Optional<T>::operator->()
    {
        SJ_ASSERT(HasValue(), "Optional value accessed with no value");
        return &m_Value;
    }
    
    template <class T>
    inline constexpr const T& Optional<T>::operator*() const&
    {
        SJ_ASSERT(HasValue(), "Optional value accessed with no value");
        return m_Value;
    }

    template <class T>
    inline constexpr T& Optional<T>::operator*() &
    {
        SJ_ASSERT(HasValue(), "Optional value accessed with no value");
        return m_Value;
    }

    template <class T>
    inline constexpr const T&& Optional<T>::operator*() const&&
    {
        SJ_ASSERT(HasValue(), "Optional value accessed with no value");
        return std::move(m_Value);
    }

    template <class T>
    inline constexpr T&& Optional<T>::operator*() &&
    {
        SJ_ASSERT(HasValue(), "Optional value accessed with no value");
        return std::move(m_Value);
    }

    template <class T>
    inline constexpr bool Optional<T>::HasValue() const noexcept
    {
        return m_HasValue;
    }

    template <class T>
    inline constexpr T& Optional<T>::Value() &
    {
        SJ_ASSERT(HasValue(), "Optional value accessed with no value");
        return m_Value;
    }
    
    template <class T>
    inline constexpr const T& Optional<T>::Value() const&
    {
        SJ_ASSERT(HasValue(), "Optional value accessed with no value");

        return m_Value;
    }

    template <class T>
    inline constexpr T&& Optional<T>::Value() &&
    {
        SJ_ASSERT(HasValue(), "Optional value accessed with no value");

        return std::move(m_Value);
    }

    template <class T>
    inline constexpr const T&& Optional<T>::Value() const&&
    {
        SJ_ASSERT(HasValue(), "Optional value accessed with no value");

        return std::move(m_Value);
    }
    
    template <class T>
    inline constexpr T Optional<T>::ValueOr(T&& default_value) const
    {
        return (HasValue()) ? m_Value : std::forward<T>(default_value);
    }
    
    template <class T>
    inline void Optional<T>::Swap(Optional& other) noexcept
    {
        if (HasValue() && other.HasValue()) 
        {
            auto tmp_value = std::move(m_Value);
            m_Value = std::move(other.m_Value);
            other.m_Value = tmp_value;
        } 
        else if (HasValue() && !other.HasValue()) 
        {
            other.m_Value = std::move(m_Value);
            other.m_HasValue = true;

            m_HasValue = false;
        }
        else if (!HasValue() && other.HasValue())
        {
            new(std::addressof(m_Value)) T(std::move(other.m_Value));
            other.m_HasValue = false;

            m_HasValue = true;
        }
    }
}