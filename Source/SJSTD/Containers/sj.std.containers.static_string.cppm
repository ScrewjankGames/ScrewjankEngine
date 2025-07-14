module;
#include <array>
#include <algorithm>
#include <cstddef>
#include <string>
export module sj.std.containers.static_string;

export namespace sj
{
    template<class T, class CharT = char>
    concept string_view_like = requires
    {
        std::is_convertible_v<const T&, std::basic_string_view<char, std::char_traits<CharT>>> == true;
        std::is_convertible_v<const T&, const CharT*> == false;
    };

    template <size_t tExtent = 128, class CharT = char>
    class static_string
    {
    public:
        constexpr static_string() = default;

        template<string_view_like svT>
        constexpr static_string(svT&& sv)
        {
            size_t incoming_length = std::size(sv);
            size_t copy_size = std::min(incoming_length, tExtent - 1);

            CharT* outIt = std::copy(std::begin(sv), std::begin(sv) + copy_size, m_data.begin());
            if(*(outIt - 1) != CharT(0))
            {
                *outIt = CharT(0);
                m_size = copy_size;
            }
            else
                m_size = copy_size - 1; // copy_size included null terminator
        }

        explicit static_string(const CharT* cstr) 
            : static_string(std::string_view(cstr))
        {

        }

        constexpr auto begin(this auto&& self)
        {
            return self.m_data.begin();
        }

        constexpr auto end(this auto&& self)
        {
            return self.m_data.end();
        }

        constexpr auto data(this auto&& self)
        {
            return self.m_data.data();
        }

        constexpr auto c_str() -> const char*
        {
            return m_data.data();
        }

        constexpr auto size() const -> size_t
        {
            return m_size;
        }

    private:
        std::array<CharT, tExtent> m_data = {0};
        size_t m_size = 0;
    };
} // namespace sj