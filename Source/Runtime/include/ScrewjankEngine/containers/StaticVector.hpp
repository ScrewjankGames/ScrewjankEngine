#pragma once

// STD Headers
#include <utility>

namespace sj
{
    /**
     * Statically sized vector class
     */
    template <class T, size_t N>
    class StaticVector
    {
    public:
        StaticVector();
        ~StaticVector() = default;

        /** Array Index Operator */
        T& operator[](const size_t index);

        /** Array Index Operator */
        const T& operator[](const size_t index) const;

        void Add(const T& value);
        void Add(T&& value);

        void EraseElement(const T& value);
        void Erase(size_t idx);

        size_t Count() const;

        size_t Capacity() const;

        T* begin();
        T* end();

    private:
        T m_cArray[N];
        size_t m_Count;
    };
}

// Include Inlines
#include <ScrewjankEngine/containers/StaticVector.inl>