#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/song_backgrounds.hpp"
#include "content_persistence_coordinator.hpp"
#include "shekinah/storage/asset_repository.hpp"
#include "shekinah/storage/song_repository.hpp"

namespace shekinah::ui {

void ShekinahWindow::apply_asset_to_song(domain::ContentRef content, domain::AssetId asset) {
    if (!assets() || !songs() || content.kind != domain::ContentKind::song) {
        return;
    }
    const auto asset_record = assets()->load_asset(asset);
    if (!asset_record) {
        return;
    }
    auto song = songs()->load_song(std::get<domain::SongId>(content.id));
    if (!song) {
        return;
    }
    application::set_default_background(
        *song, domain::BackgroundRef{asset_record->id, domain::FitMode::cover});
    const auto saved = content_persistence_coordinator_->save_song(*song);
    if (!saved) {
        return;
    }
    refresh_cues();
    update_program_status();
}

void ShekinahWindow::apply_asset_to_slide(domain::CueId cue, domain::AssetId asset) {
    if (!assets() || !songs()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return;
    }
    const auto asset_record = assets()->load_asset(asset);
    if (!asset_record) {
        return;
    }
    auto song = songs()->load_song(std::get<domain::SongId>(selected->content.id));
    if (!song) {
        return;
    }
    const auto changed = application::apply_background_from_slide(
        *song, cue, domain::BackgroundRef{asset_record->id, domain::FitMode::cover});
    if (!changed) {
        return;
    }
    const auto saved = content_persistence_coordinator_->save_song(*song);
    if (!saved) {
        return;
    }
    refresh_cues();
    update_program_status();
}

}  // namespace shekinah::ui
