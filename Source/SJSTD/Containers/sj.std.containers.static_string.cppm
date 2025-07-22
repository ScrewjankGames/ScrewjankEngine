module;
#include <ScrewjankStd/Assert.hpp>

#include <array>
#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>

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
        using value_type = CharT;
        using size_type = size_t;
        using string_view_type = std::basic_string_view<CharT>;
        constexpr static_string() = default;

        constexpr static_string(const string_view_type& sv)
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

        constexpr explicit static_string(const CharT* cstr) 
            : static_string(std::string_view(cstr))
        {

        }

        constexpr auto operator[](this auto&& self, size_t idx)
        {
            return self.m_data[idx];
        }

        constexpr operator std::string_view() const noexcept 
        {
            return std::string_view(m_data.data(), m_size);
        }

        constexpr void resize( size_type count, CharT ch )
        {
            SJ_ASSERT(count < tExtent, "Cannot resize static string larger than capacity");
            
            for(size_t i = m_size; i < count; i++)
            {
                m_data[i] = ch;
            }

            m_size = count;
        }

        constexpr void resize(size_type count)
        {
            resize(count, CharT());
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

        constexpr auto c_str() const -> const CharT*
        {
            return m_data.data();
        }

        [[nodiscard]] constexpr auto size() const -> size_t
        {
            return m_size;
        }

    private:
        std::array<value_type, tExtent> m_data = {0};
        size_t m_size = 0;
    };
} // namespace sj