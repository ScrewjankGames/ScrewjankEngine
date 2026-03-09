module;

#include <cstdint>
#include <concepts>

export module sj.datadefs.assets.AssetType;

export namespace sj
{
enum class AssetType : uint8_t
{
    kInvalid,
    kTexture,
    kMesh
};

template <class T>
concept Asset = requires(T obj) {
    { obj.assetType } -> std::convertible_to<AssetType>;
};

} // namespace sj