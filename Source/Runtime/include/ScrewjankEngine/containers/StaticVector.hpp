#pragma once

// STD Headers
#include <utility>

namespace sj
{
    /**
     * Statically sized vector class
     */
    template <class T, size_t N>
    class static_vector
    {
    public:
        constexpr static_vector() = default;
        constexpr ~static_vector() = default;

        static_vector(std::initializer_list<T> vals);

        static_vector& operator=(std::initializer_list<T> vals);

        /** Array Index Operator */
        T& operator[](const size_t index);

        /** Array Index Operator */
        const T& operator[](const size_t index) const;

        void add(const T& value);
        void add(T&& value);

        void erase_element(const T& value);
        void erase(size_t idx);

        size_t count() const;

        size_t capacity() const;

        T* data();

        void clear();

        T* begin();
        T* end();

    private:
        T m_cArray[N] = {};
        size_t m_Count = 0;
    };
}

// Include Inlines
#include <ScrewjankEngine/containers/StaticVector.inl>