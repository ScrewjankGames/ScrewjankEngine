module;

#include <algorithm>
#include <cstring>
#include <concepts>
#include <cstddef>
#include <memory_resource>
#include <span>
#include <ranges>

#include <ScrewjankStd/Assert.hpp>

export module sj.engine.ecs.Archetype;
import sj.std.containers.array;
import sj.std.containers.vector;
import sj.std.containers.type_list;
import sj.std.concepts;
import sj.std.type_info;

import sj.engine.ecs.Identifiers;
import sj.engine.system.threading.ThreadContext;

export namespace sj
{

class Archetype
{
public:
    Archetype(range_of<const type_info*> auto componentTypes, std::pmr::memory_resource* resource)
        : mResource(resource), mComponentTypes(std::from_range_t {}, componentTypes, mResource),
          mColumns(std::size(componentTypes), mResource)
    {
    }

    ~Archetype()
    {
        for(size_t idx = 0; auto&& [typeInfo, col] : std::views::zip(mComponentTypes, mColumns))
        {
            if(typeInfo->is_trivially_destructible)
                continue;

            std::span<std::byte> colBuff {col, typeInfo->size * mCapacity};

            typeInfo->destructor_fn(colBuff, mSize);
            idx++;
        }
    }

    template <class T>
    std::span<T> FindColumn()
    {
        auto it = std::ranges::find(mComponentTypes, &type_info_of<T>);
        SJ_ASSERT(it != mComponentTypes.end(), "Archetype does not contain requested component!");

        size_t colIdx = std::ranges::distance(std::ranges::begin(mComponentTypes), it);
        return std::span {reinterpret_cast<T*>(mColumns[colIdx]), mSize};
    }

    template <class T>
    std::span<T> GetColumn(size_t col)
    {
        SJ_ASSERT(GetTypeInfo<T>() == mComponentTypes[col], "Invalid component cast");

        return std::span {reinterpret_cast<T*>(mColumns[col]), mSize};
    }

    template <class T>
    T& GetComponent(size_t row)
    {
        return FindColumn<T>()[row];
    }

    size_t AddRow()
    {
        if(mSize == mCapacity)
            Resize(std::max(2uz, mSize * 2));

        for(auto&& [idx, column] : std::views::enumerate(mColumns))
        {
            const type_info* colTypeInfo = mComponentTypes[idx];

            std::byte* newComponentAddr =
                reinterpret_cast<std::byte*>(uintptr_t(column) + (colTypeInfo->size * mSize));

            colTypeInfo->constructor_fn(std::span {newComponentAddr, colTypeInfo->size}, 1);
        }

        return mSize++;
    }

    void Resize(size_t newCapacity)
    {
        range_of<size_t> auto sizes = mComponentTypes | std::views::transform(&type_info::size);
        const size_t rowSizeBytes = std::ranges::fold_left(sizes, 0, std::plus {});

        std::byte* newBuffer =
            reinterpret_cast<std::byte*>(mResource->allocate(rowSizeBytes * newCapacity));

        // Figure out where columns will be in new buffer
        scratchpad_scope scratchpad = ThreadContext::GetScratchpad();
        dynamic_array<std::byte*> newColumns(mColumns.size(), nullptr, &scratchpad.get_allocator());

        for(size_t idx = 0, cursor = uintptr_t(newBuffer); idx < newColumns.size(); idx++)
        {
            newColumns[idx] = reinterpret_cast<std::byte*>(cursor);
            cursor += newCapacity * mComponentTypes[idx]->size;
        }

        // Move data to new table
        for(auto&& [typeInfo, oldColumn, newColumn] :
            std::views::zip(mComponentTypes, mColumns, newColumns))
        {
            std::span<std::byte> oldCol {oldColumn, typeInfo->size * mCapacity};
            std::span<std::byte> newCol {newColumn, typeInfo->size * newCapacity};

            typeInfo->move_constructor_fn(oldCol, newCol, mSize);
        }

        // Update columns jump table
        for(auto&& [mColumn, tmp] : std::views::zip(mColumns, newColumns))
            mColumn = tmp;

        mResource->deallocate(mBuffer, rowSizeBytes * mSize);
        mBuffer = newBuffer;
        mCapacity = newCapacity;
    }

private:
    std::pmr::memory_resource* mResource = nullptr;

    dynamic_array<const type_info*> mComponentTypes;
    dynamic_array<std::byte*> mColumns;

    std::byte* mBuffer = nullptr;
    size_t mSize = 0;
    size_t mCapacity = 0;
};

} // namespace sj