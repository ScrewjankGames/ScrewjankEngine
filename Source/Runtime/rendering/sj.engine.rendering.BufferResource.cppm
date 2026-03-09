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
    BufferResource() = default;
    BufferResource(SDL_GPUDevice* device, SDL_GPUBufferCreateInfo info)
        : mDevice((device)), mBuffer(SDL_CreateGPUBuffer(device, &info))
    {
    }

    ~BufferResource()
    {
        Release();
    }

    BufferResource(BufferResource&& other) noexcept
    {
        mDevice = std::exchange(other.mDevice, nullptr);
        mBuffer = std::exchange(other.mBuffer, nullptr);
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

struct MeshBuffer
{
    BufferResource buffer;
    uint32_t numIndices = 0;
    uint32_t indexBufferOffset = 0;

    [[nodiscard]] SDL_GPUBufferBinding GetVertexBinding() const
    {
        return SDL_GPUBufferBinding {.buffer = buffer.GetBuffer(), .offset = 0};
    }

    [[nodiscard]] SDL_GPUBufferBinding GetIndexBinding() const
    {
        return SDL_GPUBufferBinding {.buffer = buffer.GetBuffer(), .offset = indexBufferOffset};
    }
};

} // namespace sj