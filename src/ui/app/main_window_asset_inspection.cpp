#include "shekinah/ui/main_window.hpp"

#include "shekinah/storage/asset_repository.hpp"

#include <filesystem>

namespace shekinah::ui {

QString ShekinahWindow::asset_name_for_cue(const domain::CueId& cue) const {
    if (!assets()) {
        return {};
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return {};
    }
    const auto background = presentation_.resolve_background(selected->content, cue);
    if (!background || !background->has_value()) {
        return {};
    }
    const auto asset = assets()->load_asset(background->value().asset);
    if (!asset) {
        return {};
    }
    const auto filename = std::filesystem::path{asset->relative_path}.filename().string();
    return QString::fromStdString(filename.empty() ? asset->relative_path : filename);
}

}  // namespace shekinah::ui
