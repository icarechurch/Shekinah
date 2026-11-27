#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/song_backgrounds.hpp"
#include "shekinah/storage/song_repository.hpp"

namespace shekinah::ui {

void ShekinahWindow::remove_asset_from_song(const domain::CueId& cue) {
    if (!songs()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return;
    }
    const auto background = presentation_.resolve_background(selected->content, cue);
    if (!background || !background->has_value()) {
        return;
    }
    auto song = songs()->load_song(std::get<domain::SongId>(selected->content.id));
    if (!song) {
        return;
    }
    application::remove_asset_from_song(*song, background->value().asset);
    const auto saved = songs()->save_song(*song);
    if (!saved) {
        return;
    }
    presentation_.register_song(*saved);
    refresh_cues();
    update_program_status();
}

}  // namespace shekinah::ui
