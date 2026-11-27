#pragma once

#include "shekinah/storage/asset_repository.hpp"
#include "shekinah/storage/project_store.hpp"

namespace shekinah::storage {

class SqliteAssetRepository final : public AssetRepository {
public:
    explicit SqliteAssetRepository(ProjectStore& store) : store_(store) {}

    domain::Result<std::vector<domain::AssetRecord>> list_assets() const override;
    domain::Result<domain::AssetRecord> load_asset(const domain::AssetId& id) const override;

private:
    ProjectStore& store_;
};

}  // namespace shekinah::storage
