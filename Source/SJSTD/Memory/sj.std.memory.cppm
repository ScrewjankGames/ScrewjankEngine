module;

#include <memory>
#include <cstdint>
#include <cstddef>
#include <new>
#include <memory_resource>

#include <ScrewjankStd/Assert.hpp>

export module sj.std.memory;
export import sj.std.memory.resources;
export import sj.std.memory.literals;
export import sj.std.memory.utils;
export import sj.std.memory.scratchpad_scope;