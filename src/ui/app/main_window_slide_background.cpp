#include "shekinah/ui/main_window.hpp"

#include "song_asset_coordinator.hpp"
#include "shekinah/ui/operator_page.hpp"

namespace shekinah::ui {

domain::Status ShekinahWindow::set_slide_background(
    const domain::CueId& cue, const domain::AssetId& asset) {
    const auto status = song_asset_coordinator_->set_slide_background(
        presentation_.selected_item(), cue, asset);
    if (!status) {
        return status;
    }
    if (presentation_.program().live_cue.has_value() &&
        presentation_.program().live_cue->cue == cue) {
        update_program_status();
    }
    return domain::success();
}

}  // namespace shekinah::ui
