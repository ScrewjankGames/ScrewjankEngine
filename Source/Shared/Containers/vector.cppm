module;
// Global module fragment where #includes can happen
#include <iterator>
#include <type_traits>
#include <memory>
#include <cstddef>
#include <array>
#include <algorithm>
#include <cmath>
#include <memory_resource>
#include <ranges>

#include <ScrewjankShared/utils/Assert.hpp>

// End global module fragment
export module sj.shared.containers:vector;

export namespace sj
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
        requires vector_storage<Storage>;

        typename Storage::allocator_type;

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
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;

        dynamic_vector_storage() noexcept = default;

        dynamic_vector_storage(const Allocator& alloc) noexcept
            : allocator(alloc), buffer(nullptr), capacity(0)
        {

        }

        dynamic_vector_storage(const dynamic_vector_storage& other) noexcept
            : allocator(
                std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.allocator)
            ),
            capacity(other.capacity)
        {
            buffer = allocator.allocate(other.capacity);
            
            for(int i = 0; const auto& entry : other)
            {
                new (&buffer[i]) T(entry);
                i++;
            }
        }

        dynamic_vector_storage(dynamic_vector_storage&& other) noexcept
            : allocator(std::move(other.allocator)),
              buffer(std::move(other.buffer)),
              capacity(other.capacity)
        {
            other.buffer = nullptr;
            other.capacity = 0;
        }

        ~dynamic_vector_storage() noexcept
        {
            if(buffer)
                allocator.deallocate(buffer, capacity);
        }

        dynamic_vector_storage& operator=(const dynamic_vector_storage& other) noexcept
        {
            reserve(other.size());

            for(int i = 0; auto& entry : other)
            {
                new (&((*this)[i])) T(entry);
                i++;
            }

            capacity = other.size();

            return *this;
        }

        void reserve(size_t new_cap) noexcept
        {
            if(new_cap <= capacity)
                return;

            T* new_buffer = allocator.allocate(new_cap);
            
            // Move old buffer into new buffer
            for (size_t i = 0; i < capacity; i++)
            {
                new (&new_buffer[i]) T(std::move((*this)[i]));
            }

            if(buffer)
                allocator.deallocate(buffer, capacity);

            buffer = new_buffer;
            capacity = new_cap;
        }

        auto begin(this auto&& self) noexcept
        {
            return self.buffer;
        }  

        auto end(this auto&& self) noexcept
        {
            return &(self.buffer[self.capacity]);
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

        T& operator[](size_t index)
        {
            return buffer[index];
        }

    protected:
        T* buffer = nullptr;
        size_t capacity = 0;

    private:
        allocator_type allocator = {};
    };

    template<class T, vector_storage StorageType, VectorOptions tOpts = VectorOptions {}>
    class vector_interface : public StorageType
    {
    public:
        using iterator = StorageType::iterator;
        using const_iterator = StorageType::const_iterator;
        using size_type = size_t;

        using StorageType::StorageType;
        
        constexpr vector_interface() = default;
        constexpr vector_interface(const vector_interface& other) = default;

        constexpr vector_interface(vector_interface&& other) 
            : StorageType(std::move(other))
        {
            m_count = std::exchange(other.m_count, 0);
        };

        constexpr vector_interface(size_t count, T&& val = T())
        {
            resize(count, std::forward<T>(val));
        }

        constexpr vector_interface(std::initializer_list<T> vals) noexcept
            : vector_interface(std::from_range_t{}, vals)
        {

        }

        template<std::ranges::range R>
        constexpr vector_interface(std::from_range_t, R&& rg)
        {
            if constexpr (growable_vector_storage<StorageType>)
            {
                resize(rg.size());
            }

            std::move(rg.begin(), rg.end(), begin());

            m_count = rg.size();
        }

        template<std::ranges::range R, class Allocator>
        requires growable_vector_storage<StorageType>
        constexpr vector_interface(std::from_range_t, R&& rg, Allocator alloc)
        {
            static_assert(std::is_convertible_v<Allocator, typename StorageType::allocator_type>);

            if constexpr (growable_vector_storage<StorageType>)
            {
                resize(rg.size());
            }

            std::move(rg.begin(), rg.end(), begin());

            m_count = rg.size();
        }

        constexpr ~vector_interface()
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                for(auto& it : *this)
                {
                    it.~T();
                }
            }
        }

        constexpr vector_interface& operator=(std::initializer_list<T> vals) noexcept
        {
            clear();
            
            if constexpr (growable_vector_storage<StorageType>)
            {
                this->reserve(vals.size());
            }

            for(const T& val : vals)
            {
                emplace_back(val);
            }
    
            return *this;
        }

        constexpr vector_interface& operator=(const vector_interface& other)
        {
            clear();

            StorageType::operator=(other);
            m_count = other.m_count;

            return *this;
        }

        constexpr vector_interface& operator=(vector_interface&& other)
        {
            clear();

            StorageType::operator=(std::forward<vector_interface>(other));
            m_count = other.m_count;

            return *this;
        }


        /** Array Index Operator */
        constexpr auto&& operator[](this auto&& self, const size_t index) noexcept // -> (const?) T&
        {
            return self.StorageType::operator[](index);
        }

        template<class InputRange>
        constexpr iterator insert(const_iterator pos, InputRange&& r, std::from_range_t _)
        {
            return insert(pos, r.begin(), r.end());
        }

        template<class InputIterator>
        constexpr iterator insert(const_iterator pos, InputIterator first, InputIterator last)
        {
            size_t offset = pos - begin();
            size_t numNewElements = last - first;
            this->reserve(size() + (last - first));

            std::move_backward(begin() + offset, end(), end() + numNewElements);
            std::move(first, last, begin() + offset);

            m_count += numNewElements;

            return begin() + offset;
        }

        constexpr iterator insert(const_iterator pos, T&& elem)
        {
            return emplace(pos, std::forward<T>(elem));
        } 

        template <class... Args>
        constexpr iterator emplace(const_iterator pos, Args&&... args) noexcept
            requires std::contiguous_iterator<const_iterator> 
        {
            SJ_ASSERT(pos >= this->begin() && pos <= this->end(), "Emplace index out of bounds");
            size_t offset = pos - begin();
            iterator output_pos = begin() + offset;
            
            if constexpr (!growable_vector_storage<StorageType>)
            {
                SJ_ASSERT(m_count < capacity(), "Cannot emplace into full vector");
            }
            else
            {
                if(m_count >= capacity())
                {
                    grow();
                    output_pos = begin() + offset;
                }
            }
               
            if constexpr(tOpts.preserveRelativeOrderings)
            {
                if(m_count > 0)
                {
                    for(auto it = end(); it > output_pos; it-- )
                    {
                        new (std::to_address(it)) T(std::move(*(it-1)));
                    }
                }
    
                m_count++;
            }
            else
            {
                emplace_back(std::move(*output_pos));
            }

            new (std::to_address(output_pos)) T(std::forward<Args>(args)...);

            return output_pos;
        }

        template<class... Args>
        constexpr T& emplace_back(Args&&... args) noexcept
        {
            if constexpr (!growable_vector_storage<StorageType>)
            {
                SJ_ASSERT(m_count < capacity(), "Cannot emplace into full vector");
            }
            else 
            {
                if (m_count >= capacity()) grow();
            }            

            auto address = std::to_address(this->end());
            new (address) T(std::forward<Args>(args)...);

            m_count++;

            return *reinterpret_cast<T*>(address);
        }

        constexpr T& push_back(const T& elem)
        {
            return emplace_back(elem);
        }

        constexpr void erase_element(const T& value) noexcept
        {
            for(const T& elem : *this)
            {
                if(elem == value)
                {
                    erase(&elem);
                    break;
                }
            }
        }

        constexpr iterator erase(const_iterator pos) noexcept 
            requires std::contiguous_iterator<const_iterator> 
        {
            SJ_ASSERT(pos >= begin() && pos < end(), "Erase index out of bounds")
            pos->~T();

            size_t offset = pos - begin();
            iterator output_pos = begin() + offset;

            if(m_count > 0)
            {
                if constexpr (tOpts.preserveRelativeOrderings)
                {
                    for(auto it = output_pos + 1; it != end(); ++it)
                    {
                        new (std::to_address(it - 1)) T(std::move(*it));
                    }
                }
                else
                {
                    new (std::to_address(output_pos)) T(std::move(*(end() - 1)));
                }
            }

            m_count--;
            return output_pos;
        }

        constexpr size_type size() const noexcept 
        {
            return m_count;
        }

        constexpr void resize(size_type new_size, T&& value = T()) noexcept
        {
            if constexpr (growable_vector_storage<StorageType>)
            {
                this->reserve(new_size);
            }
            else
            {
                SJ_ASSERT(new_size < capacity(), "cannot grow statically sized vector");
            }

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

        constexpr size_type capacity() const noexcept
        {
            return StorageType::size();
        }

        constexpr auto data(this auto&& self) noexcept
        {
            return self.StorageType::data();
        }

        constexpr void clear() noexcept
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

        constexpr auto begin(this auto&& self) noexcept
        {
            return self.StorageType::begin();
        }

        constexpr const_iterator cbegin() const noexcept
        {
            return begin();
        }

        constexpr auto end(this auto&& self) noexcept
        {
            return &(self.begin()[self.m_count]);
        }

        constexpr const_iterator cend() const noexcept
        {
            return end();
        }

        [[nodiscard]] constexpr bool empty() const noexcept 
        {
            return end() == begin(); 
        }

        constexpr auto&& at(this auto&& self, size_t index)
        {
            SJ_ASSERT(index >= 0 && index < self.size(), "Out of bounds access");
            return self.operator[](index);
        }

        constexpr auto&& front(this auto&& self)
        {
            SJ_ASSERT(self.size() > 0, "Out of bounds access");
            return *self.begin();
        }

        constexpr auto&& back(this auto&& self)
        {
            SJ_ASSERT(self.size() > 0, "Out of bounds access");
            return *(self.end() - 1);
        }

    private: 
        void grow() 
            requires growable_vector_storage<StorageType> 
        {
            size_t new_capacity = static_cast<size_t>( std::ceil(capacity() * tOpts.growFactor) );
            if(new_capacity == 0)
                new_capacity = 1;
            this->reserve(new_capacity);
        }

        size_t m_count = 0;
    };

    template <class T, vector_storage Storage, VectorOptions tOpts>
    constexpr void swap(vector_interface<T, Storage, tOpts>& lhs,
        vector_interface<T, Storage, tOpts>& rhs) noexcept
    {
        vector_interface<T, Storage, tOpts> temp = std::move(lhs);
        lhs = std::move(rhs);
        rhs = std::move(temp);
    }

    template<class T, size_t N, VectorOptions tOpts = {}>
    using static_vector = vector_interface<T, std::array<T,N>, tOpts>;

    template<class T, VectorOptions tOpts = {}, class AllocatorType = std::pmr::polymorphic_allocator<T>>
    using dynamic_vector = vector_interface<T, dynamic_vector_storage<T, AllocatorType>, tOpts>;
}

