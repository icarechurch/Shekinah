#include "shekinah/ui/operator_page.hpp"

#include "shekinah/ui/cue_grid_widget.hpp"

namespace shekinah::ui {

void OperatorPage::set_cue_asset_drop_handler(
    std::function<void(domain::CueId, domain::AssetId)> handler) {
    cue_grid_->set_asset_drop_handler(std::move(handler));
}

void OperatorPage::set_cue_asset_insert_handler(
    std::function<void(std::size_t, domain::AssetId)> handler) {
    cue_grid_->set_asset_insert_handler(std::move(handler));
}

}  // namespace shekinah::ui
