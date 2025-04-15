module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <type_traits>
export module sj.shared.containers:any;

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
        kDestroy
    };

    template<class T>
    void ManagerFn(Op op, void* self, void* arg )
    {
        switch (op) 
        {
            default:
        }
    }

    using ManagerFnPtr = void(*)(Op op, void* self, void* arg);
}

export namespace sj
{
    // std::any but with a fixed size buffer/alignment
    template<size_t tSize, size_t tAlign>
    class static_any
    {
        static_any() = default;

        template<class T> 
        static_any(T&& val) requires priv::holdable<T, tSize, tAlign>
        {
            new (m_buffer) T(std::forward<T>(val));
            m_managerFn = priv::ManagerFn<std::decay<T>>;
        }

        template<class T>
        T& get()
        {
            static_assert(priv::ManagerFn<T> == m_managerFn );
            return *reinterpret_cast<T*>(m_buffer);
        }


    private:
        alignas(tAlign) std::byte m_buffer[tSize];
        priv::ManagerFnPtr m_managerFn = nullptr;
    };
}