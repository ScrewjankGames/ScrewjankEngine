module;
#include <memory_resource>
#include <utility>
#include <span>
#include <ScrewjankStd/Assert.hpp>
#include <tuple>
#include <ranges>

// End global module fragment
export module sj.std.containers:sparse_set;
import :array;
import :vector;

export namespace sj 
{
    template<class T>
    concept sparse_set_id = requires(T elem)
    {
        elem.sparseIndex;
        elem.generation;
    };

    template<sparse_set_id IdType, template<class Elem> class AllocatorType, class ... DenseElements>
    class sparse_set_base
    {
    public:
        using IndexType = decltype(IdType::sparseIndex);

        sparse_set_base() = default;

        sparse_set_base(std::pmr::memory_resource* memoryResource)
            : m_freeListHeadIndex(0),
              m_sparse(0, InternalIdType{}, memoryResource),
              m_denseElements { 
                std::make_tuple<>(
                    DenseContainer<IdType>(memoryResource), 
                    DenseContainer<DenseElements>(memoryResource)...) 
                } 
        {

        }

        sparse_set_base(IndexType capacity, std::pmr::memory_resource* memoryResource)
            : m_freeListHeadIndex(0),
              m_sparse(capacity, InternalIdType{}, memoryResource),
              m_denseElements { 
                std::make_tuple<>(
                    DenseContainer<IdType>(memoryResource), 
                    DenseContainer<DenseElements>(memoryResource)...) 
                } 
        {
            std::apply([capacity](auto&&... containers) {
                (containers.reserve(capacity), ...);
            }, m_denseElements);

            LinkFreeList(0, capacity);
        }

        IdType create(DenseElements&& ... elements)
        {
            return create(m_freeListHeadIndex, std::forward<DenseElements>(elements)...);
        }

        IdType create(IdType id, DenseElements&& ... elements)
        {
            return create(id.sparseIndex, std::forward<DenseElements>(elements)...);
        }

        void release(IdType id)
        {
            SJ_ASSERT(
                m_sparse[id.sparseIndex].generation == id.generation,
                "Accesing sparse set with stale handle"
                );

            const IndexType releasedSparseIndex = id.sparseIndex;
            const IndexType releasedDenseIndex = m_sparse[releasedSparseIndex].dense;

            m_sparse[releasedSparseIndex].dense = kInvalidIndex;
            m_sparse[releasedSparseIndex].generation++;

            if(m_freeListHeadIndex == kInvalidIndex)
            {
                m_freeListHeadIndex = releasedSparseIndex;
                m_sparse[releasedSparseIndex].next = kInvalidIndex;
                m_sparse[releasedSparseIndex].prev = kInvalidIndex;
            }
            else
            {
                m_sparse[releasedSparseIndex].prev = m_sparse[m_freeListHeadIndex].prev;
                m_sparse[releasedSparseIndex].next = m_freeListHeadIndex;
                m_freeListHeadIndex = releasedSparseIndex;
            }


            // Fill gaps in dense arrays
            std::apply(
                [releasedDenseIndex](auto&&... containers) {
                    (containers.erase_unordered(containers.begin() + releasedDenseIndex), ...);
                }, 
                m_denseElements
            );

            auto& denseIdList = std::get<0>(m_denseElements);
            if(releasedDenseIndex < denseIdList.size())
            {
                const IdType& movedElement = denseIdList[releasedDenseIndex];
                m_sparse[movedElement.sparseIndex].dense = releasedDenseIndex;
            }
        }

        IndexType get_sparse_size() const
        {
            return m_sparse.size();
        }

        template<class DenseElement>
        auto get(this auto&& self, IdType id) //-> const? DenseElement*
        {
            IndexType denseIndex = self.m_sparse[id.sparseIndex].dense;

            SJ_ASSERT(
                self.m_sparse[id.sparseIndex].generation == id.generation,
                 "Accesing sparse set with stale handle"
                );

            if(self.m_sparse[id.sparseIndex].dense == kInvalidIndex)
            {
                DenseElement* ret = nullptr;
                return ret;
            }
            else
                return &std::get<DenseContainer<DenseElement>>(self.m_denseElements)[denseIndex];
        }

        template<class DenseElement>
        decltype(auto) get_set(this auto&& self) // -> std::span<const? DenseElement>
        {
            return std::span(std::get<DenseContainer<DenseElement>>(self.m_denseElements));
        }

        auto get_all()
        {
            return std::apply(std::views::zip, m_denseElements);
        }

    private:
        struct InternalIdType
        {
            IndexType dense = kInvalidIndex;
            IndexType generation = 0;
            
            IndexType prev = kInvalidIndex;
            IndexType next = kInvalidIndex;
        };

        void LinkFreeList(IndexType startIndex, IndexType end)
        {
            m_freeListHeadIndex = startIndex;
            m_sparse[startIndex].prev = kInvalidIndex;
            for(IndexType i = startIndex + 1; i < end; i++)
            {
                m_sparse[i-1].next = i;
                m_sparse[i].prev = i-1;
            }

            m_sparse[end-1].next = kInvalidIndex;
        }

        IdType create(IndexType requestedSparseIndex, DenseElements&& ... elements)
        {
            SJ_ASSERT(requestedSparseIndex >= 0 && requestedSparseIndex < m_sparse.size(), "Requested index out of bounds!");
            
            InternalIdType& interalId = m_sparse[requestedSparseIndex];
            SJ_ASSERT(interalId.dense == kInvalidIndex, "Cannot insert into occupied sparse index");

            if(requestedSparseIndex == m_freeListHeadIndex)
                m_freeListHeadIndex = interalId.next;

            if(interalId.prev != kInvalidIndex)
                m_sparse[interalId.prev].next = interalId.next;

            if(interalId.next != kInvalidIndex)
                m_sparse[interalId.next].prev = interalId.prev;

            auto& denseIdList = std::get<0>(m_denseElements);
            interalId.dense = denseIdList.size();

            IdType& id = denseIdList.emplace_back(requestedSparseIndex, interalId.generation);

            // Visit each dense array and add element
            if constexpr (sizeof...(elements) > 0)
                EmplaceDenseElements(std::forward<DenseElements>(elements) ...);
            
            return id;
        }

        template<class T, class ... MoreDenseElements>
        void EmplaceDenseElements(T&& elem, MoreDenseElements&& ... rest)
        {
            auto& targetContainer = std::get<DenseContainer<T>>(m_denseElements);
            targetContainer.emplace_back(std::forward<T>(elem));

            if constexpr (sizeof...(rest) > 0) {
                EmplaceDenseElements(std::forward<MoreDenseElements>(rest)...);
            }
        }

        static constexpr IndexType kInvalidIndex = std::numeric_limits<IndexType>::max();
        IndexType m_freeListHeadIndex;
        dynamic_array<InternalIdType, IndexType, AllocatorType<InternalIdType>> m_sparse;

        template<class T>
        using DenseContainer = dynamic_vector<T, {}, AllocatorType<T>>;

        std::tuple<
            DenseContainer<IdType>,
            DenseContainer<DenseElements>...
        > m_denseElements;
    };

    template<sparse_set_id IdType, class ... DenseElements>
    using sparse_set = sparse_set_base<IdType, std::pmr::polymorphic_allocator, DenseElements...>;
}
