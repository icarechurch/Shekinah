#include "asset_catalog_coordinator.hpp"

#include "shekinah/storage/asset_repository.hpp"
#include "shekinah/storage/project_session.hpp"

#include <filesystem>

namespace shekinah::ui {

AssetCatalogCoordinator::AssetCatalogCoordinator(storage::ProjectSession& session) noexcept
    : session_(session) {}

bool AssetCatalogCoordinator::available() const noexcept {
    return session_.assets() != nullptr;
}

domain::Result<std::vector<domain::AssetRecord>> AssetCatalogCoordinator::list() const {
    const auto* assets = session_.assets();
    if (assets == nullptr) {
        return domain::Result<std::vector<domain::AssetRecord>>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before listing assets"});
    }
    return assets->list_assets();
}

domain::Result<domain::AssetRecord> AssetCatalogCoordinator::load(
    const domain::AssetId& id) const {
    const auto* assets = session_.assets();
    if (assets == nullptr) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before loading assets"});
    }
    return assets->load_asset(id);
}

std::optional<std::string> AssetCatalogCoordinator::filename(
    const domain::AssetId& id) const {
    const auto asset = load(id);
    if (!asset) {
        return std::nullopt;
    }
    const auto filename = std::filesystem::path{asset->relative_path}.filename().string();
    return filename.empty() ? asset->relative_path : filename;
}

}  // namespace shekinah::ui
