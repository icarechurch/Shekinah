#include "shekinah/ui/main_window.hpp"

#include "content_persistence_coordinator.hpp"
#include "shekinah/storage/asset_repository.hpp"
#include "shekinah/storage/song_repository.hpp"
#include "shekinah/ui/operator_page.hpp"

#include <algorithm>

namespace shekinah::ui {

domain::Status ShekinahWindow::set_slide_background(
    const domain::CueId& cue, const domain::AssetId& asset) {
    if (!assets() || !songs()) {
        return domain::Status::fail(
            {domain::ErrorCode::storage_failure, "Open a project before setting a background"});
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return domain::Status::fail(
            {domain::ErrorCode::invalid_input, "Only selected Song cues accept editable backgrounds"});
    }
    const auto asset_record = assets()->load_asset(asset);
    if (!asset_record) {
        return domain::Status::fail(asset_record.error());
    }
    auto song = songs()->load_song(std::get<domain::SongId>(selected->content.id));
    if (!song) {
        return domain::Status::fail(song.error());
    }
    const auto slide = std::find_if(song->slides.begin(), song->slides.end(), [&](const auto& candidate) {
        return candidate.id == cue;
    });
    if (slide == song->slides.end()) {
        return domain::Status::fail({domain::ErrorCode::not_found, "Song cue was not found"});
    }
    slide->background = domain::BackgroundRef{asset_record->id, domain::FitMode::cover};
    const auto saved = content_persistence_coordinator_->save_song(*song);
    if (!saved) {
        return domain::Status::fail(saved.error());
    }
    if (presentation_.program().live_cue.has_value() &&
        presentation_.program().live_cue->cue == cue) {
        update_program_status();
    }
    return domain::success();
}

}  // namespace shekinah::ui
