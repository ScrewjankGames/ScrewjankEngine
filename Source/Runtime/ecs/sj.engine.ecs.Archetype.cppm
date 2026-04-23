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
import sj.std.hash;
import sj.std.concepts;
import sj.std.type_info;

import sj.engine.ecs.Identifiers;
import sj.engine.system.threading.ThreadContext;

export namespace sj
{

class Archetype
{
public:
    using ArchetypeId = uint32_t;

    Archetype(range_of<const type_info*> auto componentTypes, std::pmr::memory_resource* resource)
        : mResource(resource), mRowDescriptors(resource)
    {
        auto ids = componentTypes | std::views::transform([](const type_info* info) {
                       return info->id;
                   });

        mId = FNV1a_32(ids);

        mRowDescriptors.resize(componentTypes.size());
        for(int i = 0; Row& row : mRowDescriptors)
        {
            row.typeInfo = componentTypes[i];
            i++;
        }
    }

    ~Archetype()
    {
        for(size_t idx = 0; auto&& [typeInfo, rowData] : mRowDescriptors)
        {
            if(typeInfo->is_trivially_destructible)
                continue;

            typeInfo->destructor_fn(rowData);
            idx++;
        }
    }

    [[nodiscard]] uint32_t GetId() const
    {
        return mId;
    }

    template <class T>
    std::span<T> GetComponents()
    {
        auto it = std::ranges::find(mRowDescriptors, &type_info_of<T>, &Row::typeInfo);
        SJ_ASSERT(it != mRowDescriptors.end(), "Archetype does not contain requested component!");

        return byte_span_cast<T>(it->data);
    }

    template <class T>
    T& GetComponent(size_t idx)
    {
        return GetComponents<T>()[idx];
    }

    size_t AddEntry()
    {
        if(mSize == mCapacity)
            Resize(std::max(2uz, mSize * 2));

        for(Row& row : mRowDescriptors)
        {
            std::byte* newComponentAddr = reinterpret_cast<std::byte*>(
                uintptr_t(row.data.data()) + (row.typeInfo->size * mSize));

            row.typeInfo->constructor_fn(std::span {newComponentAddr, row.typeInfo->size});
        }

        return mSize++;
    }

private:
    void Resize(size_t newCapacity)
    {
        range_of<size_t> auto sizes = mRowDescriptors | std::views::transform(&Row::typeInfo) |
                                      std::views::transform(&type_info::size);

        const size_t rowSizeBytes = std::ranges::fold_left(sizes, 0, std::plus {});

        std::byte* newBuffer =
            reinterpret_cast<std::byte*>(mResource->allocate(rowSizeBytes * newCapacity));

        // Figure out where rows will be in new buffer
        scratchpad_scope scratchpad = ThreadContext::GetScratchpad();
        dynamic_array<std::span<std::byte>> newRows(mRowDescriptors.size(),
                                                    {},
                                                    &scratchpad.get_allocator());

        for(size_t idx = 0, cursor = uintptr_t(newBuffer); idx < newRows.size(); idx++)
        {
            size_t sizeInBytes = newCapacity * mRowDescriptors[idx].typeInfo->size;
            newRows[idx] = {reinterpret_cast<std::byte*>(cursor), sizeInBytes};
            cursor += sizeInBytes;
        }

        // Move data to new table
        for(auto&& [rowHeader, newRowData] : std::views::zip(mRowDescriptors, newRows))
        {
            rowHeader.typeInfo->move_constructor_fn(rowHeader.data, newRowData);
            rowHeader.data = newRowData;
        }

        mResource->deallocate(mBuffer, rowSizeBytes * mSize);
        mBuffer = newBuffer;
        mCapacity = newCapacity;
    }

    std::pmr::memory_resource* mResource = nullptr;

    struct Row
    {
        const type_info* typeInfo;
        std::span<std::byte> data;
    };

    dynamic_array<Row> mRowDescriptors;
    std::byte* mBuffer = nullptr;
    size_t mSize = 0;
    size_t mCapacity = 0;

    uint32_t mId = 0;
};

} // namespace sj