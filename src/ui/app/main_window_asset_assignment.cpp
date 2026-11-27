#include "shekinah/ui/main_window.hpp"

#include "song_asset_coordinator.hpp"

namespace shekinah::ui {

void ShekinahWindow::apply_asset_to_song(domain::ContentRef content, domain::AssetId asset) {
    if (!song_asset_coordinator_->assign_default(content, asset)) {
        return;
    }
    refresh_cues();
    update_program_status();
}

void ShekinahWindow::apply_asset_to_slide(domain::CueId cue, domain::AssetId asset) {
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return;
    }
    if (!song_asset_coordinator_->assign_from_slide(selected->content, cue, asset)) {
        return;
    }
    refresh_cues();
    update_program_status();
}

}  // namespace shekinah::ui
