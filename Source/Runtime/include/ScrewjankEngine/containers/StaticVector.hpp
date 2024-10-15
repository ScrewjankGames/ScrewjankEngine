#pragma once

// STD Headers
#include <utility>

namespace sj
{
    /**
     * Statically sized vector class
     */
    template <class T, size_t N, class SizeType = uint32_t>
    class static_vector
    {
    public:
        using iterator = T*;
        using const_iterator = const T*;
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;

        constexpr static_vector() = default;
        constexpr ~static_vector() = default;

        static_vector(SizeType count) noexcept;
        static_vector(SizeType count, const T& value);

        static_vector(std::initializer_list<T> vals);

        static_vector& operator=(std::initializer_list<T> vals);

        /** Array Index Operator */
        T& operator[](const SizeType index);

        /** Array Index Operator */
        const T& operator[](const SizeType index) const;

        void push_back(const T& value);
        void push_back(T&& value);

        template<class... Args>
        void emplace_back(Args&&... args);

        void erase_element(const T& value);
        void erase(SizeType idx);

        SizeType size() const;

        void resize(SizeType new_size);
        void resize(SizeType new_size, const T& value);

        SizeType capacity() const;

        T* data();

        void clear();

        T* begin();
        T* end();

        [[nodiscard]] bool empty() const;

    private:
        friend void swap(static_vector<T, N, SizeType>& lhs, static_vector<T, N, SizeType>& rhs);

        T m_cArray[N] = {};
        SizeType m_Count = 0;
    };

    template <class T, size_t N, class SizeType = uint32_t>
    void swap(sj::static_vector<T, N, SizeType>& lhs,
              sj::static_vector<T, N, SizeType>& rhs) noexcept
    {
        std::swap(lhs.m_cArray, rhs.m_cArray, std::nothrow_t);
        std::swap(lhs.m_Count, rhs.m_Count, std::nothrow_t);
    }
} // namespace sj

// Include Inlines
#include <ScrewjankEngine/containers/StaticVector.inl>

