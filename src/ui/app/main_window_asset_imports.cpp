#include "shekinah/ui/main_window.hpp"

#include "shekinah/storage/asset_importer.hpp"
#include "shekinah/storage/project_store.hpp"

#include <filesystem>

namespace shekinah::ui {

domain::Result<domain::AssetRecord> ShekinahWindow::import_asset_file(
    const std::filesystem::path& source) {
    if (!store()) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before importing assets"});
    }
    return storage::AssetImporter{*store()}.import_file(source);
}

}  // namespace shekinah::ui
