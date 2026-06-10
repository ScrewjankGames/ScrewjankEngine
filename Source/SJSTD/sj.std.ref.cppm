module;
#include <atomic>
#include <concepts>
#include <cstdint>
export module sj.std.ref;

export namespace sj
{

// TODO [NL]: Using strictest memory order for all operations because I don't know how they work yet
template <class T>
class ref
{
public:
    ref() = default;

    ref(const T& data)
        requires std::copy_constructible<T>
        : mData(data)
    {
    }

    ref(T&& data) : mData(std::move(data))
    {
    }

    ref(ref<T>&& other) noexcept : mData(std::move(other.mData))
    {
        mRefCnt.exchange(other.mRefCnt);
        other.mRefCnt.store(0);
    }

    ref<T>& operator=(ref<T>&& other) noexcept
    {
        if(this != &other)
        {
            mRefCnt.exchange(other.mRefCnt);
            other.mRefCnt.store(0);
        }

        return *this;
    }

    auto&& operator*(this auto&& self)
    {
        return self.mData;
    }

    auto operator->(this auto&& self)
    {
        return &self.mData;
    }

    void refcount_increment()
    {
        mRefCnt.fetch_add(1);
    }

    void refcount_decrement()
    {
        // TODO [NL] how to memory orders work
        mRefCnt.fetch_sub(1);
    }

private:
    T mData;
    std::atomic<int64_t> mRefCnt = 0;
};
} // namespace sj