module;

// STD Headers
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <format>
#include <memory_resource>

export module sj.std.memory.allocators:Allocator;

export namespace sj
{
    /**
     * Concept for template deduction required when supplying allocators to containers
     */
    template <class T>
    concept allocator_concept = requires(T obj) {
        // These statements must compile for something to be considered an allocator
        { obj.allocate(size_t()) };
        { obj.deallocate(nullptr) };
    };

    /**
     * Concept for template deduction required when supplying allocators to containers
     */
    template <class T>
    concept is_allocator_ptr = requires(T obj) {
        // These statements must compile for something to be considered an allocator pointer
        { obj->allocate(size_t()) };
        { obj->deallocate(nullptr) };
    };

    class memory_resource : public std::pmr::memory_resource
    {
    public:
        memory_resource() = default;

        // Derived resources may provide constructors similar to these, but all resources should be
        // initializable post-construction
        virtual void init(size_t numBytes, void* buffer) = 0;

        void init(size_t numBytes, std::pmr::memory_resource& hostResource)
        {
            void* memory = hostResource.allocate(numBytes);
            init(numBytes, memory);
        }
        
        [[nodiscard]] virtual bool contains_ptr(void* ptr) const = 0;

#ifndef SJ_GOLD
        void set_debug_name(const char* name)
        {
            std::format_to_n(m_DebugName.data(), 256, "{}", name);
        }
    protected:
        [[nodiscard]] bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
        {
            return uintptr_t(this) == uintptr_t(&other);
        }

    private:
        std::array<char, 256> m_DebugName = {};
#endif
    };
} // namespace sj
