module;

#include <utility>
#include <cstddef>

export module sj.shared.containers:type_list;

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
    };
}