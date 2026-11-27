#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <vector>

namespace shekinah::storage {

class AssetRepository {
public:
    virtual ~AssetRepository() = default;

    virtual domain::Result<std::vector<domain::AssetRecord>> list_assets() const = 0;
    virtual domain::Result<domain::AssetRecord> load_asset(const domain::AssetId& id) const = 0;
};

}  // namespace shekinah::storage
