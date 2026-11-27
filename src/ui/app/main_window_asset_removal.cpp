#include "shekinah/ui/main_window.hpp"

#include "song_asset_coordinator.hpp"

namespace shekinah::ui {

void ShekinahWindow::remove_asset_from_song(const domain::CueId& cue) {
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return;
    }
    const auto background = presentation_.resolve_background(selected->content, cue);
    if (!background || !background->has_value()) {
        return;
    }
    if (!song_asset_coordinator_->remove(selected->content, background->value().asset)) {
        return;
    }
    refresh_cues();
    update_program_status();
}

}  // namespace shekinah::ui
