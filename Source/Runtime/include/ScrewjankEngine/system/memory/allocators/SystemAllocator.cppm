module;

#include <ScrewjankShared/utils/Assert.hpp>

#include <cstdlib>
#include <cstdint>
#include <cstddef>

export module sj.engine.system.memory.allocators:SystemAllocator;
import :Allocator;

export namespace sj {

    class SystemAllocator final : public Allocator
    {
    public:
        /**
         * Constructor
         */
        SystemAllocator() = default;

        /**
         * Destructor
         */
        virtual ~SystemAllocator() = default;

        void Init(size_t bytes = 0, void* start = 0)
        {
            SJ_ASSERT(start == nullptr, "Cannot init system allocator");
        }

        /**
         * Allocates size bites from the heap
         * @param size The number of bytes to allocate
         * @param alignment The alignment requirement for this allocation
         */
        [[nodiscard]] 
        virtual void* allocate(const size_t size, const size_t alignment = alignof(std::max_align_t)) 
        {
            return std::malloc(size);
        }

        /**
         * Marks memory as free
         * @param memory Pointer to the memory to free
         */
        virtual void deallocate(void* memory)
        {
            std::free(memory);
        }

        /**
         * Allocate enough aligned memory for the provided type
         * @tparam T the type to allocate memory for
         */
        template <class T>
        [[nodiscard]] void* AllocateType() 
        {
            return allocate(sizeof(T), alignof(T));
        }

        virtual uintptr_t Begin() const
        {
            return 0;
        }

        virtual uintptr_t End() const 
        {
            return 0;
        }
    };
} // namespace sj
