#include "shekinah/storage/content_repository.hpp"

#include "internal/sqlite_asset_repository.hpp"

namespace shekinah::storage {

domain::Result<std::vector<domain::AssetRecord>> SqliteContentRepository::list_assets() const {
    return assets_->list_assets();
}

domain::Result<domain::AssetRecord> SqliteContentRepository::load_asset(
    const domain::AssetId& id) const {
    return assets_->load_asset(id);
}

}  // namespace shekinah::storage
