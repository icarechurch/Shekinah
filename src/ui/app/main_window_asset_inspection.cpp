#include "shekinah/ui/main_window.hpp"

#include "asset_catalog_coordinator.hpp"

namespace shekinah::ui {

QString ShekinahWindow::asset_name_for_cue(const domain::CueId& cue) const {
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return {};
    }
    const auto background = presentation_.resolve_background(selected->content, cue);
    if (!background || !background->has_value()) {
        return {};
    }
    const auto filename = asset_catalog_coordinator_->filename(background->value().asset);
    if (!filename.has_value()) {
        return {};
    }
    return QString::fromStdString(*filename);
}

}  // namespace shekinah::ui
