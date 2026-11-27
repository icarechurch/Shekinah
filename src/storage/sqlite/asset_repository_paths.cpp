#include "internal/sqlite_asset_repository.hpp"

namespace shekinah::storage {

domain::Result<std::filesystem::path> SqliteAssetRepository::resolve_asset_path(
    const domain::AssetId& id) const {
    const auto asset = load_asset(id);
    if (!asset) {
        return domain::Result<std::filesystem::path>::fail(asset.error());
    }
    return domain::Result<std::filesystem::path>::ok(store_.root() / asset->relative_path);
}

}  // namespace shekinah::storage
