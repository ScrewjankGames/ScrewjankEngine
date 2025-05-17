module;
#include <ScrewjankShared/utils/Assert.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <type_traits>
export module sj.shared.containers:any;
import sj.shared.core;

export namespace sj {
    template<size_t tSize, size_t tAlign>
    class static_any;
}

namespace priv
{
    template<class T, size_t tBufferSize, size_t tBufferAlign>
    concept holdable = requires
    {
        sizeof(T) <= tBufferSize;
        alignof(T) <= tBufferAlign;
    };

    enum class Op : uint8_t
    {
        kCopy,
        kMove,
        kDestroy
    };

    union ArgType
    {
        void* arg;
        const void* c_arg;
    };

    template<class T>
    void ManagerFn(Op op, void* any, ArgType arg)
    {
        T* self = reinterpret_cast<T*>(any);
        switch (op) 
        {
            case Op::kCopy:
            {
                new (self) T(*reinterpret_cast<const T*>(arg.c_arg));
                break;
            }
            case Op::kMove:
            {
                new (self) T(std::move(*reinterpret_cast<T*>(arg.arg)));
                break;
            }
            case Op::kDestroy:
            {
                self->~T();
                break;
            }
            default:
                SJ_ASSERT(false, "Unhandled sj::any operation!");
                break;
        }
    }

    using ManagerFnPtr = void(*)(Op op, void* self, ArgType arg);
}

export namespace sj
{
    // std::any but with a fixed size buffer/alignment
    template<size_t tSize, size_t tAlign=alignof(std::max_align_t)>
    class static_any
    {
    public:
        static_any() = default;

        static_any(const static_any& other)
            : m_managerFn(other.m_managerFn)
        {
            if(m_managerFn)
            {
                priv::ArgType arg;
                arg.c_arg = other.m_buffer;
                m_managerFn( priv::Op::kCopy, m_buffer, arg);
            }
        }

        static_any(static_any&& other)
        {
            m_managerFn = std::exchange(other.m_managerFn, nullptr);
            
            if(m_managerFn)
            {
                priv::ArgType arg;
                arg.arg = other.m_buffer;
                m_managerFn( priv::Op::kMove, m_buffer, arg);
            }
        }

        template<class T> 
        static_any(T&& val) 
            requires (
                priv::holdable<T, tSize, tAlign> 
                && !is_instantiation_of_v<typename std::decay<T>::type, static_any>
            )
            : m_managerFn( priv::ManagerFn<typename std::decay<T>::type> )
        {
            new (m_buffer) typename std::decay<T>::type(std::forward<T>(val));
        }

        ~static_any()
        {
            if(m_managerFn)
            {
                priv::ArgType arg;
                arg.arg = nullptr;
                m_managerFn(priv::Op::kDestroy, m_buffer, arg);
            }
        }

        static_any& operator=(const static_any& other)
        {
            *this = static_any(other);
            return *this;
        }

        static_any& operator=(static_any&& other)
        {
            SJ_ASSERT(this != &other, "Self assignment of static_any not allowed");
            if (!other.has_value())
                reset();
            else
            {
                reset();
                m_managerFn = std::exchange(other.m_managerFn, nullptr);
                priv::ArgType arg;
                arg.arg = other.m_buffer;
                m_managerFn(priv::Op::kMove, m_buffer, arg);
            }

            return *this;
        }

        template<typename T>
        static_any& operator=(T&& rhs)
            requires (
                priv::holdable<T, tSize, tAlign> 
                && !is_instantiation_of_v<typename std::decay<T>::type, static_any>
            )
        {
            *this = any(std::forward<T>(rhs));
            return *this;
        }


        bool has_value() const
        {
            return m_managerFn != nullptr;
        }

        template<class T>
        T& get()
        {
            SJ_ASSERT(priv::ManagerFn<T> == m_managerFn, "bad any cast" );
            return *reinterpret_cast<T*>(m_buffer);
        }

        void reset()
        {
            if(m_managerFn)
                m_managerFn(priv::Op::kDestroy, m_buffer, nullptr);
            
            m_managerFn = nullptr;    
        }

    private:
        alignas(tAlign) std::byte m_buffer[tSize];
        priv::ManagerFnPtr m_managerFn = nullptr;
    };
}