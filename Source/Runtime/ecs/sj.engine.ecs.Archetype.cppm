module;

#include <algorithm>
#include <array>
#include <cstring>
#include <concepts>
#include <cstddef>
#include <memory_resource>
#include <span>
#include <ranges>

#include <ScrewjankStd/Assert.hpp>
#include "vulkan/vulkan.hpp"

export module sj.engine.ecs.Archetype;
import sj.std;
import sj.engine.ecs.Identifiers;

export namespace sj
{
using ArchetypeId = uint32_t;

template <std::ranges::range R>
ArchetypeId ComputeArchetypeId(R&& componentTypes)
    requires std::convertible_to<std::ranges::range_value_t<R>, const type_info*>
{
    uint32_t hash = 0;

    // XOR associative and commutative - no threat of the same type id
    // appearing multiple times
    for(TypeId id : componentTypes | std::views::transform(&type_info::id))
        hash ^= FNV1a_32(std::as_bytes(std::span {&id, 1}));

    return hash;
}

class Archetype
{
public:
    using RowIdx = uint8_t;
    constexpr static RowIdx kMaxRows = std::numeric_limits<RowIdx>::max() - 1;
    constexpr static RowIdx kInvalidRowIdx = std::numeric_limits<RowIdx>::max();

    Archetype(range_of<const type_info*> auto componentTypes, std::pmr::memory_resource* resource)
        : mResource(resource), mGoIds(resource), mRows(resource)
    {
        mRows.resize(componentTypes.size());

        for(size_t idx = 0; const type_info* info : componentTypes)
            mRows[idx++] = Row(info);
    }

    ~Archetype()
    {
        for(Row& r : mRows)
            mResource->deallocate(r.buffer.data(), r.buffer.size());
    }

    Archetype(const Archetype&) = delete;
    Archetype(Archetype&& other) noexcept
        : mResource(other.mResource), mRows(std::move(other.mRows)),
          mSize(std::exchange(other.mSize, 0)), mCapacity(std::exchange(other.mCapacity, 0))
    {
    }

    [[nodiscard]] std::ranges::range auto GetTypeIds() const
    {
        return mRows | std::views::transform([](Row& r) {
                   return r.typeInfo->id;
               });
    }

    bool MatchesQuery(std::ranges::range auto&& queryIds)
    {
        auto&& archetypeIds = GetTypeIds();
        for(TypeId id : queryIds)
        {
            if(!std::ranges::contains(archetypeIds, id))
                return false;
        }

        return true;
    }

    RowIdx GetRowIdx(TypeId typeId)
    {
        for(RowIdx idx = 0; idx < mRows.size(); idx++)
            if(mRows[idx].typeInfo->id == typeId)
                return idx;

        return kInvalidRowIdx;
    }

    template <class T>
    std::span<T> GetRow()
    {
        Row* row = FindRow(type_id_of<T>);
        if(!row)
            return {};

        return row->GetElementsView<T>(mSize);
    }

    template <class T>
    std::span<T> GetRow(RowIdx rowIdx)
    {
        SJ_ASSERT(rowIdx < mRows.size(), "Row Index {} oor [0, {})]", rowIdx, mRows.size());
        SJ_ASSERT(mRows[rowIdx].typeInfo->id == type_id_of<T>,
                  "Requested type {} at row {} but archetype contains {} at that row index",
                  type_name_of<T>,
                  rowIdx,
                  mRows[rowIdx].typeInfo->name);

        return mRows[rowIdx].GetElementsView<T>(mSize);
    }

    typed_ptr GetEntry(TypeId id, size_t entryIdx)
    {
        Row* row = FindRow(id);

        SJ_ASSERT(row, "Invalid type id {} passed to archetype", id);
        SJ_ASSERT(entryIdx < mSize, "Entry index {} oor [0, {}) ", entryIdx, mSize);

        return typed_ptr {(*row)[entryIdx], id};
    }

    template <class T>
    T& GetEntry(size_t entryIdx)
    {
        return GetRow<T>()[entryIdx];
    }

    size_t AddEntry(GameObjectId goId)
    {
        if(mSize == mCapacity)
            Resize(std::max(1uz, mSize * 2));

        for(auto&& row : mRows)
            std::invoke(row.typeInfo->constructor_fn, row[mSize]);

        mGoIds[mSize] = goId;
        return mSize++;
    }

    template <std::ranges::range CtorCallbacks>
        requires std::invocable<std::ranges::range_value_t<CtorCallbacks>, typed_ptr>
    size_t AddEntry(GameObjectId goId, CtorCallbacks&& constructCallbacks)
    {
        SJ_ASSERT(constructCallbacks.size() == mRows.size(),
                  "Incorrect number of constructor callbacks- expected {} got {}",
                  mRows.size(),
                  constructCallbacks.size());

        if(mSize == mCapacity)
            Resize(std::max(1uz, mSize * 2));

        for(auto&& [row, ctorCallback] : std::views::zip(mRows, constructCallbacks))
            std::invoke(ctorCallback, typed_ptr(row[mSize], row.typeInfo->id));

        mGoIds[mSize] = goId;
        return mSize++;
    }

    void RemoveEntry(size_t idx)
    {
        for(Row& row : mRows)
        {
            if(!row.typeInfo->is_trivially_destructible)
                row.typeInfo->destructor_fn(row[idx]);

            // Move last element to deleted location
            if(mSize > 1)
            {
                row.typeInfo->move_constructor_fn(row[idx], row[mSize - 1]);
                mGoIds[idx] = mGoIds[mSize - 1];
            }
        }

        mSize--;
    }

    std::span<const GameObjectId> GetGameObjects()
    {
        return std::span(mGoIds.data(), mSize);
    }

private:
    struct Row
    {
        Row() = default;
        Row(const Row&) = delete;
        Row(Row&& other) noexcept
            : typeInfo(other.typeInfo), buffer(std::exchange(other.buffer, std::span<std::byte> {}))
        {
        }

        Row(const type_info* info) : typeInfo(info)
        {
        }

        void* operator[](size_t idx)
        {
            return &buffer[idx * typeInfo->size];
        }

        Row& operator=(Row&& other) noexcept
        {
            typeInfo = other.typeInfo;
            buffer = std::exchange(other.buffer, std::span<std::byte> {});
            return *this;
        }

        template <class T>
        std::span<T> GetElementsView(size_t mElementCount)
        {
            std::span<std::byte> activeView = buffer.subspan(0, typeInfo->size * mElementCount);
            return byte_span_cast<T>(activeView);
        }

        const type_info* typeInfo = nullptr;
        std::span<std::byte> buffer;
    };

    void Resize(size_t newCapacity)
    {
        mGoIds.resize(newCapacity);

        for(Row& row : mRows)
        {
            const size_t newSizeBytes = newCapacity * row.typeInfo->size;
            void* newBufferAddr = mResource->allocate(newSizeBytes, row.typeInfo->alignment);
            std::span<std::byte> newBuffer = {
                reinterpret_cast<std::byte*>(newBufferAddr),
                newSizeBytes,
            };

            for(size_t elementIdx = 0; elementIdx < mSize; elementIdx++)
            {
                row.typeInfo->move_constructor_fn(&newBuffer[elementIdx * row.typeInfo->size],
                                                  row[elementIdx]);
            }

            if(row.buffer.data())
                mResource->deallocate(row.buffer.data(), row.buffer.size());

            row.buffer = newBuffer;
        }

        mCapacity = newCapacity;
    }

    Row* FindRow(TypeId id)
    {
        auto it = std::ranges::find(mRows, id, [](const Row& r) {
            return r.typeInfo->id;
        });

        if(it == mRows.end())
            return nullptr;

        return &(*it);
    }

    std::pmr::memory_resource* mResource = nullptr;
    dynamic_array<GameObjectId> mGoIds;
    dynamic_array<Row> mRows;
    size_t mSize = 0;
    size_t mCapacity = 0;
};

} // namespace sj