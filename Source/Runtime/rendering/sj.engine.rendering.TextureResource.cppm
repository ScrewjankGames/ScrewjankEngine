module;
#include <SDL3/SDL_gpu.h>

#include <utility>

export module sj.engine.rendering.TextureResource;

export namespace sj
{
class TextureResource
{
public:
    TextureResource() = default;
    TextureResource(SDL_GPUDevice* device, const SDL_GPUTextureCreateInfo& createInfo)
        : mDevice(device), mInfo(createInfo)
    {
        mHandle = SDL_CreateGPUTexture(mDevice, &mInfo);
    }
    ~TextureResource()
    {
        if(mHandle)
            SDL_ReleaseGPUTexture(mDevice, mHandle);
    }

    TextureResource(TextureResource&& other) noexcept
    {
        *this = std::move(other);
    }

    TextureResource& operator=(TextureResource&& other) noexcept
    {
        Release();

        mDevice = std::exchange(other.mDevice, nullptr);
        mHandle = std::exchange(other.mHandle, nullptr);
        mInfo = other.mInfo;

        return *this;
    }

    void Resize(uint32_t width, uint32_t height)
    {
        Release();
        mInfo.width = width;
        mInfo.height = height;
        *this = TextureResource(mDevice, mInfo);
    }

    void Release()
    {
        if(mHandle != nullptr)
            SDL_ReleaseGPUTexture(mDevice, mHandle);

        mHandle = nullptr;
    }

    [[nodiscard]] auto Get(this auto&& self)
    {
        return self.mHandle;
    }

    [[nodiscard]] uint32_t GetWidth() const
    {
        return mInfo.width;
    }

    [[nodiscard]] uint32_t GetHeight() const
    {
        return mInfo.height;
    }

    SDL_GPUTextureFormat GetFormat() const
    {
        return mInfo.format;
    }

private:
    SDL_GPUDevice* mDevice = nullptr;
    SDL_GPUTexture* mHandle = nullptr;
    SDL_GPUTextureCreateInfo mInfo {};
};
} // namespace sj