#include "asset_import_coordinator.hpp"

#include "shekinah/storage/asset_importer.hpp"
#include "shekinah/storage/project_session.hpp"

namespace shekinah::ui {

AssetImportCoordinator::AssetImportCoordinator(storage::ProjectSession& session) noexcept
    : session_(session) {}

domain::Result<domain::AssetRecord> AssetImportCoordinator::import_file(
    const std::filesystem::path& source) const {
    auto* importer = session_.asset_importer();
    if (!session_.is_open() || importer == nullptr) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before importing assets"});
    }
    return importer->import_file(source);
}

}  // namespace shekinah::ui
