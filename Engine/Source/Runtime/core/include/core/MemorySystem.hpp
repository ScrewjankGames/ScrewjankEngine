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
        /**
         * Provides global access to the memory system
         */
        static MemorySystem* Get();

        /**
         * Provides global access to the engine's default allocator
         * @note This allocator is invalid until the engine calls Initialize()
         */
        static Allocator* GetDefaultAllocator();

        /**
         * Provides global access to an unmanaged system allocator
         * @note This allocator is valid at any time
         */
        static Allocator* GetUnmanagedAllocator();

      private:
        friend class Game;
        /**
         * Initializes memory system and reserves a block of memory for the default allocator
         */
        void Initialize();

      private:
        /////////////////////////////////////////////////////////
        /// Do not access variables and functions in this section
        /////////////////////////////////////////////////////////

        Allocator* m_DefaultAllocator;
        MemorySystem();
        ~MemorySystem();
    };
} // namespace Screwjank
