#include "shekinah/ui/main_window.hpp"

#include "asset_import_coordinator.hpp"

#include <filesystem>

namespace shekinah::ui {

domain::Result<domain::AssetRecord> ShekinahWindow::import_asset_file(
    const std::filesystem::path& source) {
    return asset_import_coordinator_->import_file(source);
}

}  // namespace shekinah::ui
