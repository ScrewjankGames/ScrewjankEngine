module;
#include <SDL3/SDL_gpu.h>
#include <utility>

export module sj.engine.rendering.SamplerResource;

export namespace sj
{
inline const SDL_GPUTextureCreateInfo gDefaultSamplerTextureCreateInfo =
    SDL_GPUTextureCreateInfo {.type = SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D,
                              .format = SDL_GPUTextureFormat::SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                              .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
                              //.width = width,
                              //.height = height,
                              .layer_count_or_depth = 1,
                              .num_levels = 1};

inline const SDL_GPUSamplerCreateInfo gDefaultSamplerCreateInfo {
    .min_filter = SDL_GPUFilter::SDL_GPU_FILTER_LINEAR,
    .mag_filter = SDL_GPUFilter::SDL_GPU_FILTER_LINEAR,
    .mipmap_mode = SDL_GPUSamplerMipmapMode::SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
    .address_mode_u = SDL_GPUSamplerAddressMode::SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    .address_mode_v = SDL_GPUSamplerAddressMode::SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    .address_mode_w = SDL_GPUSamplerAddressMode::SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    .mip_lod_bias = 0.0f,
    .max_anisotropy = 1.0f,
    .compare_op = SDL_GPU_COMPAREOP_ALWAYS,
    .min_lod = 0.0f,
    .max_lod = 1.0f,
    .enable_anisotropy = false,
    .enable_compare = false};

class SamplerResource
{
public:
    SamplerResource() = default;
    SamplerResource(SDL_GPUDevice* device,
                    uint32_t width,
                    uint32_t height,
                    SDL_GPUSamplerCreateInfo samplerInfo = gDefaultSamplerCreateInfo)
        : mDevice(device), mWidth(width), mHeight(height), mCreateInfo(samplerInfo)
    {
        SDL_GPUTextureCreateInfo textureInfo = gDefaultSamplerTextureCreateInfo;
        textureInfo.width = width;
        textureInfo.height = height;

        mTexture = SDL_CreateGPUTexture(mDevice, &textureInfo);
        mSampler = SDL_CreateGPUSampler(mDevice, &mCreateInfo);
    }

    ~SamplerResource()
    {
        Release();
    }

    SamplerResource(SamplerResource&& other) noexcept
    {
        *this = std::move(other);
    }

    SamplerResource& operator=(SamplerResource&& other) noexcept
    {
        Release();

        mDevice = std::exchange(other.mDevice, nullptr);
        mTexture = std::exchange(other.mTexture, nullptr);
        mWidth = std::exchange(other.mWidth, 0);
        mWidth = std::exchange(other.mHeight, 0);
        mSampler = std::exchange(other.mSampler, nullptr);

        mCreateInfo = std::move(other.mCreateInfo);

        return *this;
    }

    void Resize(uint32_t width, uint32_t height)
    {
        *this = SamplerResource(mDevice, width, height);
    }

    void Release()
    {
        if(mTexture)
        {
            SDL_ReleaseGPUTexture(mDevice, mTexture);
            mTexture = nullptr;
        }

        if(mSampler)
        {
            SDL_ReleaseGPUSampler(mDevice, mSampler);
            mSampler = nullptr;
        }
    }

    [[nodiscard]] auto GetSampler(this auto&& self)
    {
        return self.mSampler;
    }

    [[nodiscard]] auto GetTexture(this auto&& self)
    {
        return self.mTexture;
    }

    [[nodiscard]] uint32_t GetWidth() const
    {
        return mWidth;
    }

    [[nodiscard]] uint32_t GetHeight() const
    {
        return mHeight;
    }

    [[nodiscard]] SDL_GPUTextureFormat GetFormat() const
    {
        return gDefaultSamplerTextureCreateInfo.format;
    }

private:
    SDL_GPUDevice* mDevice = nullptr;
    SDL_GPUTexture* mTexture = nullptr;
    SDL_GPUSampler* mSampler = nullptr;
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;
    SDL_GPUSamplerCreateInfo mCreateInfo {};
};
} // namespace sj