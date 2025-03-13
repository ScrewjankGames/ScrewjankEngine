module;
// Global module fragment where #includes can happen
#include <memory>
#include <cstddef>
#include <array>
#include <algorithm>

#include <ScrewjankShared/utils/Assert.hpp>

// End global module fragment
export module sj.containers:vector;

namespace sj_2
{
    struct VectorOptions
    {
        bool stableOperations = true;
    };

    template <class Storage>
    concept vector_storage = requires 
    { 
      typename Storage::value_type;
      typename Storage::reference;
      typename Storage::const_reference;

      typename Storage::iterator;
      typename Storage::const_iterator;

      Storage::data();
      Storage::operator[];
    };

    template <class Storage>
    concept fixed_vector_storage = requires 
    {
      vector_storage<Storage>;
      Storage::max_size() -> size_t;
    };

    template <class Storage>
    concept dynamic_vector_storage = requires 
    {
      vector_storage<Storage>;
      Storage::reserve();
      Storage::capacity() -> size_t;
    };

    template<class T, size_t N>
    using static_vector_storage = std::array<T, N>;

    template<class T, vector_storage StorageType, VectorOptions tOpts = VectorOptions {}>
    class vector_interface
    {
        using iterator = StorageType::iterator;
        using const_iterator = StorageType::const_iterator;
        using value_type = StorageType::value_type;
        using reference = StorageType::reference;
        using const_reference = StorageType::const_reference;
        using size_type = StorageType::size_type;
        
        vector_interface(std::initializer_list<T> vals) noexcept
        {

        }

        vector_interface& operator=(std::initializer_list<T> vals) noexcept
        {

        }

        /** Array Index Operator */
        auto&& operator[](this auto&& self, const size_t index) noexcept // -> (const?) T&
        {
            return self.m_storage[index];
        }

        template <class... Args>
        iterator emplace(const_iterator pos, Args&&... args) noexcept
        {
            SJ_ASSERT(pos >= m_storage.begin() && pos <= m_storage.end(), "Emplace index out of bounds");

            if constexpr (fixed_vector_storage<StorageType>)
            {
                SJ_ASSERT(m_storage.begin() + m_count < m_storage.end(), "Cannot emplace into full vector");
            }
    
            if constexpr(tOpts.stableOperations)
            {
                if(m_count > 0)
                {
                    auto my_end = end();
                    std::ranges::move_backward(pos, my_end, my_end+1);
                }
    
                m_count++;
            }
            else
            {
                emplace_back(std::move(*pos));
            }

            auto address = std::addressof(*pos);
            new(address) T(std::forward<Args>(args)...);

            return pos;
        }

        template<class... Args>
        void emplace_back(Args&&... args) noexcept;

        void erase_element(const T& value) noexcept;
        void erase(size_type idx) noexcept;

        size_type size() const noexcept;

        void resize(size_type new_size) noexcept;
        void resize(size_type new_size, const T& value) noexcept;

        size_type capacity() const noexcept;

        T* data() noexcept;

        void clear() noexcept;

        decltype(auto) begin(this auto&& self) noexcept
        {
            return self.m_storage.begin();
        }

        decltype(auto) end(this auto&& self) noexcept
        {
            return self.m_storage.begin() + self.m_count;
        }

        [[nodiscard]] bool empty() const noexcept 
        {
            end() == begin(); 
        }

    private: 
        StorageType m_storage;
        size_t m_count;
    };

}

