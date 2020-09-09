#pragma once
// STD Headers

// Library Headers

// Screwjank Headers
#include "system/Allocator.hpp"

namespace Screwjank {
    /**
     * Class that manage's engines default memory allocator and provides access to system memory
     * info
     */
    class MemorySystem
    {
      public:
        /** Provides global access to the memory system */
        static MemorySystem* Get();

        /** Provides global access to the engine's default allocator */
        static Allocator* GetDefaultAllocator();

        /** Provides global access to an unmanaged system allocator */
        static Allocator* GetDefaultUnmanagedAllocator();

      private:
        friend class Game;

        Allocator* m_DefaultAllocator;

        void Initialize();

        MemorySystem();
        ~MemorySystem();
    };
} // namespace Screwjank
