#pragma once

// STD Headers
#include <utility>

namespace sj
{
    struct VectorOptions
    {
        bool kStableOperations = true;
    };

    /**
     * Statically sized vector class
     */
    template <class T, size_t N, VectorOptions tOpts = VectorOptions {}, class SizeType = uint32_t>
    class static_vector
    {
    public:
        using iterator = T*;
        using const_iterator = const T*;
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;

        constexpr static_vector() noexcept = default;
        constexpr ~static_vector() = default;

        static_vector(SizeType count) noexcept;
        static_vector(SizeType count, const T& value) noexcept;

        static_vector(std::initializer_list<T> vals) noexcept;

        static_vector& operator=(std::initializer_list<T> vals) noexcept;

        /** Array Index Operator */
        auto&& operator[](this auto&& self, const SizeType index) noexcept; // -> (const?) T&

        void push_back(const T& value) noexcept;
        void push_back(T&& value) noexcept;

        iterator insert(const_iterator pos, const T& value);
        iterator insert(const_iterator pos, T&& value);

        template <class... Args>
        iterator emplace(const_iterator pos, Args&&... args) noexcept;

        template<class... Args>
        void emplace_back(Args&&... args) noexcept;

        void erase_element(const T& value) noexcept;
        void erase(SizeType idx) noexcept;

        SizeType size() const noexcept;

        void resize(SizeType new_size) noexcept;
        void resize(SizeType new_size, const T& value) noexcept;

        SizeType capacity() const noexcept;

        T* data() noexcept;

        void clear() noexcept;

        decltype(auto) begin(this auto&& self) noexcept; // -> const? T*
        decltype(auto) end(this auto&& self) noexcept;

        [[nodiscard]] bool empty() const noexcept;

    private:
        T m_cArray[N] = {};
        SizeType m_Count = 0;
    };

    template <class T, size_t N, VectorOptions tOpts, class SizeType = uint32_t>
    void swap(sj::static_vector<T, N, tOpts, SizeType>& lhs,
              sj::static_vector<T, N, tOpts, SizeType>& rhs) noexcept
    {
        std::swap(lhs.m_cArray, rhs.m_cArray, std::nothrow);
        std::swap(lhs.m_Count, rhs.m_Count, std::nothrow);
    }
} // namespace sj

// Include Inlines
#include <ScrewjankEngine/containers/StaticVector.inl>

