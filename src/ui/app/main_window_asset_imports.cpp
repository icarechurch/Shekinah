#include "shekinah/ui/main_window.hpp"

#include "shekinah/storage/asset_importer.hpp"

#include <filesystem>

namespace shekinah::ui {

domain::Result<domain::AssetRecord> ShekinahWindow::import_asset_file(
    const std::filesystem::path& source) {
    if (!project_open() || !asset_importer()) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before importing assets"});
    }
    return asset_importer()->import_file(source);
}

}  // namespace shekinah::ui
