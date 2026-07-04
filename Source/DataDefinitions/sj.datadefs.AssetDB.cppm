module;

#include <ScrewjankStd/Assert.hpp>

#include <glaze/glaze.hpp>

#include <cstdint>
#include <filesystem>
#include <random>
#include <string>
#include <flat_map>
#include <string_view>

export module sj.datadefs.AssetDB;
import sj.std;

export namespace sj
{
using AssetID = uint64_t;
inline constexpr AssetID kInvalidAssetID = 0;

class AssetDB
{
public:
    AssetDB() : mAssetIdDistribution(1) // 0 is not a valid asset ID
    {
        std::random_device device;
        mAssetIdGenerator.seed(device());
    }

    std::string_view GetAssetPath(AssetID id) const
    {
        auto it = mDB.find(id);
        SJ_ASSERT(it != mDB.end(), "Failed to locate asset id {}", id);
        return it->second;
    }

    std::optional<std::string_view> TryGetAssetPath(AssetID id) const
    {
        auto it = mDB.find(id);
        if(it == mDB.end())
            return std::nullopt;

        return it->second;
    }

    template <uint32_t tFormat = glz::BEVE>
    void Save(std::string_view path)
    {
        sj::dynamic_vector<char> buffer;

        const glz::error_ctx ec =
            glz::write<glz::opts {.format = tFormat, .prettify = true}>(mDB, buffer);
        SJ_ASSERT(!bool(ec), "Failed to serialize asset db to requested format");

        glz::error_code res = glz::buffer_to_file(buffer, path);
        SJ_ASSERT(res == glz::error_code::none, "Failed to save asset DB to location {path}");
    }

    template <uint32_t tFormat = glz::BEVE>
    void Load(std::string_view path)
    {
        sj::dynamic_vector<char> buffer;
        auto&& _ = glz::file_to_buffer(buffer, path);

        glz::error_ctx res = glz::read<glz::opts {.format = tFormat}>(mDB, buffer);
        SJ_ASSERT(res.ec == glz::error_code::none, "Failed to load asset DB from location {path}");
    }

    void AddImport(AssetID id, std::string_view path)
    {
        mDB[id] = path;
    }

    [[nodiscard]] AssetID NewAssetID()
    {
        // Generate an ID
        uint64_t assetID = mAssetIdDistribution(mAssetIdGenerator);

        // Make sure it's unique
        while(mDB.contains(assetID))
            assetID = mAssetIdDistribution(mAssetIdGenerator);

        return assetID;
    }

private:
    std::mt19937_64 mAssetIdGenerator;
    std::uniform_int_distribution<uint64_t> mAssetIdDistribution;

    dynamic_flat_map<AssetID, std::string> mDB;
};
} // namespace sj