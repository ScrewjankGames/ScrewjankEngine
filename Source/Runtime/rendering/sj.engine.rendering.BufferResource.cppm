module;

#include <ScrewjankStd/Assert.hpp>

#include <SDL3/SDL_gpu.h>

#include <array>
#include <utility>
#include <span>

export module sj.engine.rendering.BufferResource;

export namespace sj
{
class BufferResource
{
public:
    BufferResource() = delete;
    BufferResource(SDL_GPUDevice* device, SDL_GPUBufferCreateInfo info)
        : mDevice((device)), mBuffer(SDL_CreateGPUBuffer(device, &info))
    {
    }

    ~BufferResource()
    {
    }

    BufferResource(BufferResource&& other) noexcept
    {
        *this = std::move(other);
    }

    BufferResource& operator=(BufferResource&& other) noexcept
    {
        Release();
        mDevice = std::exchange(other.mDevice, nullptr);
        mBuffer = std::exchange(other.mBuffer, nullptr);

        return *this;
    }

    auto GetBuffer(this auto&& self)
    {
        return self.mBuffer;
    }

    void Release()
    {
        if(mBuffer)
            SDL_ReleaseGPUBuffer(mDevice, mBuffer);

        mBuffer = nullptr;
    }

protected:
    SDL_GPUDevice* mDevice = nullptr;
    SDL_GPUBuffer* mBuffer = nullptr;
};

template <size_t tNumBindings>
class BoundBufferResource : public BufferResource
{
public:
    using BufferResource::BufferResource;

    void AddBindings(std::span<uint32_t> bindingOffsets)
    {
        SJ_ASSERT(bindingOffsets.size() <= tNumBindings, "Too many buffer bindings");
        std::ranges::copy(bindingOffsets, mBindings.begin());
    }

    SDL_GPUBufferBinding operator[](size_t idx) const
    {
        return SDL_GPUBufferBinding {.buffer = mBuffer, .offset = mBindings.at(idx)};
    }

private:
    std::array<uint32_t, tNumBindings> mBindings;
};

} // namespace sj