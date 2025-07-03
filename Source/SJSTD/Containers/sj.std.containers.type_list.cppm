module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory_resource>
#include <type_traits>
#include <utility>

#include <ScrewjankStd/Assert.hpp>

export module sj.std.containers.type_list;
import sj.std.type_traits;

export namespace sj
{
    template<class ... Types>
    class type_list
    {
    public:
        template<auto F, class ... Args>
        static constexpr void for_each(Args&&... args)
        {
            (F.template operator()<Types>(std::forward<Args>(args)...), ...);
        }

        static constexpr size_t size()
        {
            return sizeof...(Types);
        }

        template<auto tTransformFn, class ResultType>
        static constexpr auto transform_list() -> std::array<ResultType, size()>
        {
            std::array<ResultType, size()> result = {};

            int i = 0;
            auto emplaceResult = []<class T>(std::array<ResultType, size()>& container, int& index)
            {
                container[index] = tTransformFn.template operator()<T>();
                index++;
            };

            for_each<emplaceResult>(result, i);

            return result;
        }
    };

    template<class...>
    struct concat_type_lists;

    template<class... LHS, class ... RHS>
    struct concat_type_lists<type_list<LHS...>, type_list<RHS...>>
    {
        using list = type_list<LHS..., RHS...>;
    };

    template <auto tTypeList, class KeyType, class ValueType, auto tToKeyFn, auto tToValueFn>
    class type_map
    {
    public:
        constexpr type_map() 
            : m_keys(tTypeList.template transform_list<tToKeyFn, KeyType>()),
              m_values(tTypeList.template transform_list<tToValueFn, ValueType>())
        {

        }

        [[nodiscard]] constexpr auto get(const KeyType& key) const -> const ValueType&
        {
            auto keyIt = std::ranges::find(m_keys, key);
            size_t offset = std::distance(m_keys.begin(), keyIt);
            auto valueIt = m_values.begin() + offset;
            SJ_ASSERT(valueIt != m_values.end(), "Failed to find key in compile time map");
            return *valueIt;
        }

        [[nodiscard]] constexpr auto find(const KeyType& key) const -> const ValueType*
        {
            auto keyIt = std::ranges::find(m_keys, key);
            size_t offset = std::distance(m_keys.begin(), keyIt);
            auto valueIt = m_values.begin() + offset;
            if(valueIt == m_values.end())
                return nullptr;
            else
                return &*valueIt;
        }

    private:
        std::array<KeyType, tTypeList.size()> m_keys;
        std::array<ValueType, tTypeList.size()> m_values;
    };

}
