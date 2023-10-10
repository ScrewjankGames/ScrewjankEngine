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
        constexpr static_vector() = default;
        constexpr ~static_vector() = default;

        static_vector(std::initializer_list<T> vals);

        static_vector& operator=(std::initializer_list<T> vals);

        /** Array Index Operator */
        T& operator[](const SizeType index);

        /** Array Index Operator */
        const T& operator[](const SizeType index) const;

        void add(const T& value);
        void add(T&& value);

        void erase_element(const T& value);
        void erase(SizeType idx);

        SizeType size() const;

        SizeType capacity() const;

        T* data();

        void clear();

        T* begin();
        T* end();

    private:
        T m_cArray[N] = {};
        SizeType m_Count = 0;
    };
}

// Include Inlines
#include <ScrewjankEngine/containers/StaticVector.inl>