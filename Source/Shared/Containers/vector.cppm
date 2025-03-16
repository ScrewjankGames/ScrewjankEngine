module;
// Global module fragment where #includes can happen
#include <iterator>
#include <type_traits>
#include <memory>
#include <cstddef>
#include <array>
#include <vector>
#include <algorithm>
#include <cmath>
#include <memory_resource>

#include <ScrewjankShared/utils/Assert.hpp>

// End global module fragment
export module sj.containers:vector;

export namespace sj_2
{
    struct VectorOptions
    {
        bool preserveRelativeOrderings = true;
        float growFactor = 2.0;
    };

    template <class Storage>
    concept vector_storage = requires(Storage instance)
    { 
        typename Storage::iterator;
        typename Storage::const_iterator;

        instance.data();
        { instance.size() } -> std::convertible_to<size_t>;
        instance[0];
    };

    template <class Storage>
    concept growable_vector_storage = requires(Storage instance)
    {
        vector_storage<Storage>;

        typename Storage::allocator_type;

        instance.replace_buffer(nullptr, 0);
        instance.get_allocator();
    };

    template<class T, size_t N>
    using static_vector_storage = std::array<T, N>;

    template<class T, class Allocator = std::pmr::polymorphic_allocator<T>>
    class dynamic_vector_storage 
    {
    public:
        using iterator = T*;
        using const_iterator = const T*;
        using allocator_type = Allocator;

        dynamic_vector_storage() noexcept
            : allocator(Allocator()) 
        {

        }

        dynamic_vector_storage(const Allocator& alloc) noexcept
            : allocator(alloc)
        {

        }

        auto begin(this auto&& self) noexcept
        {
            return self.buffer;
        }  

        auto end(this auto&& self) noexcept
        {
            return self.data[self.capacity];
        }
        
        size_t size() const noexcept
        { 
            return capacity; 
        }

        auto data(this auto&& self) noexcept
        {
            return self.buffer;
        }

        decltype(auto) operator[](this auto&& self, size_t idx) noexcept
        {
            return self.buffer[idx];
        }

        allocator_type get_allocator()
        {
            return allocator;
        }

        void replace_buffer(T* new_buffer, size_t new_capacity)
        {
            if(buffer)
            {
                allocator.deallocate(buffer, capacity);
            }

            buffer = new_buffer;
            capacity = new_capacity;
        }

        T& operator[](size_t index)
        {
            return buffer[index];
        }

    protected:
        T* buffer;
        size_t capacity;

    private:
        allocator_type allocator;
    };

    template<class T, vector_storage StorageType, VectorOptions tOpts = VectorOptions {}>
    class vector_interface : public StorageType
    {
    public:
        using iterator = StorageType::iterator;
        using const_iterator = StorageType::const_iterator;
        using size_type = size_t;

        using StorageType::StorageType;
        
        vector_interface() = default;

        vector_interface(std::initializer_list<T> vals) noexcept
        {
            if constexpr (growable_vector_storage<StorageType>)
            {
                reserve(vals.size());
            }

            for(const T& val : vals)
            {
                emplace_back(val);
            }
        }

        vector_interface& operator=(std::initializer_list<T> vals) noexcept
        {
            clear();
            
            if constexpr (growable_vector_storage<StorageType>)
            {
                reserve(vals.size());
            }

            for(const T& val : vals)
            {
                push_back(val);
            }
    
            return *this;
        }

        /** Array Index Operator */
        auto&& operator[](this auto&& self, const size_t index) noexcept // -> (const?) T&
        {
            return self.StorageType::operator[](index);
        }

        template <class... Args>
        iterator emplace(const_iterator pos, Args&&... args) noexcept
            requires std::contiguous_iterator<const_iterator> 
        {
            SJ_ASSERT(pos >= this->begin() && pos <= this->end(), "Emplace index out of bounds");

            if constexpr (!growable_vector_storage<StorageType>)
            {
                SJ_ASSERT(m_count < capacity(), "Cannot emplace into full vector");
            }
            else
            {
                if(m_count >= capacity())
                {
                    size_t offset = std::distance(begin(), pos);
                    grow();
                    pos = begin() + offset;
                }
            }
    
            if constexpr(tOpts.preserveRelativeOrderings)
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

            new (std::to_address(pos)) T(std::forward<Args>(args)...);

            return pos;
        }

        template<class... Args>
        void emplace_back(Args&&... args) noexcept
        {
            if constexpr (!growable_vector_storage<StorageType>)
            {
                SJ_ASSERT(m_count < capacity(), "Cannot emplace into full vector");
            }
            else 
            {
                if (m_count >= capacity()) grow();
            }            

            auto address = std::addressof(*this->end());
            new(address) T(std::forward<Args>(args)...);
        }

        void erase_element(const T& value) noexcept
        {
            for(int i = 0; i < m_count; i++)
            {
                if((*this)[i] == value)
                {
                    erase(i);
                    break;
                }
            }
        }

        void erase(const_iterator iter) noexcept 
            requires std::contiguous_iterator<const_iterator> 
        {
            SJ_ASSERT(iter >= begin() && iter < end(), "Erase index out of bounds")

            iter->~T();

            if(m_count > 0)
            {
                if constexpr (tOpts.preserveRelativeOrderings)
                {
                    auto my_end = end();
                    std::ranges::move(iter+1, my_end+1, iter);
                }
                else
                {
                    new (std::to_address(iter)) T(std::move(*(end() - 1)));
                }
            }

            m_count--;
        }

        size_type size() const noexcept 
        {
            return m_count;
        }

        void resize(size_type new_size, T&& value = T()) noexcept
            requires growable_vector_storage<StorageType> 
        {
            reserve(new_size);

            if (new_size > m_count)
            {
                for (auto i = m_count; i < new_size; i++)
                {
                    new (&(*this)[i]) T(value);
                }
            }
            else
            {
                if constexpr (!std::is_trivially_destructible_v<T>)
                {
                    // If the vector is shrinking, destroy the elements that aren't getting copied
                    for (size_t i = new_size; i < m_count; i++)
                    {
                        (*this)[i].~T();
                    }
                }
            }
    
            m_count = new_size;
        }

        void reserve(size_type new_cap) noexcept
            requires growable_vector_storage<StorageType>
        {
            if(new_cap <= capacity())
                return;

            auto allocator = this->get_allocator();
            T* new_buffer = allocator.allocate(new_cap);
            
            // Move old buffer into new buffer
            for (size_t i = 0; i < this->size(); i++)
            {
                new (&new_buffer[i]) T(std::move((*this)[i]));
            }

            StorageType::replace_buffer(new_buffer, new_cap);
        }

        size_type capacity() const noexcept
        {
            return StorageType::size();
        }

        auto data(this auto&& self) noexcept
        {
            return self.StorageType::data();
        }

        void clear() noexcept
        {
            if constexpr(!std::is_trivially_destructible<T>::value)
            {
                for(T& entry : *this)
                {
                    entry.~T();
                }
            }
    
            m_count = 0;
        }

        auto begin(this auto&& self) noexcept
        {
            return self.StorageType::begin();
        }

        auto end(this auto&& self) noexcept
        {
            return std::to_address(self.begin() + self.m_count);
        }

        [[nodiscard]] bool empty() const noexcept 
        {
            end() == begin(); 
        }

    private: 
        void grow() 
            requires growable_vector_storage<StorageType> 
        {
            size_t new_capacity = static_cast<size_t>( std::ceil(capacity() * tOpts.growFactor) );
            reserve(new_capacity);
        }

        size_t m_count = 0;
    };

    template<class T, size_t N, VectorOptions tOpts = {}>
    using static_vector = vector_interface<T, std::array<T,N>, tOpts>;

    template<class T, class AllocatorType = std::pmr::polymorphic_allocator<T>, VectorOptions tOpts = {}>
    using dynamic_vector = vector_interface<T, dynamic_vector_storage<T, AllocatorType>, tOpts>;
}

