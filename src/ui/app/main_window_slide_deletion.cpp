#include "shekinah/ui/main_window.hpp"

#include "shekinah/storage/content_repository.hpp"

#include <algorithm>

namespace shekinah::ui {

void ShekinahWindow::delete_slide(const domain::CueId& cue) {
    if (!repository()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    if (selected->content.kind == domain::ContentKind::song) {
        const auto id = std::get<domain::SongId>(selected->content.id);
        auto song = repository()->load_song(id);
        if (!song || song->slides.size() <= 1) {
            return;
        }
        const auto position = std::find_if(song->slides.begin(), song->slides.end(), [&](const auto& slide) {
            return slide.id == cue;
        });
        if (position == song->slides.end()) {
            return;
        }
        song->slides.erase(position);
        const auto saved = repository()->save_song(*song);
        if (saved) {
            presentation_.register_song(*saved);
            if (presentation_.program().live_cue.has_value() &&
                presentation_.program().live_cue->cue == cue) {
                presentation_.clear();
            }
            refresh_cues();
            update_program_status();
        }
        return;
    }

    const auto id = std::get<domain::SermonId>(selected->content.id);
    auto sermon = repository()->load_deck(id);
    if (!sermon || sermon->slides.size() <= 1) {
        return;
    }
    const auto position = std::find_if(sermon->slides.begin(), sermon->slides.end(), [&](const auto& slide) {
        return slide.id == cue;
    });
    if (position == sermon->slides.end()) {
        return;
    }
    sermon->slides.erase(position);
    const auto saved = repository()->save_deck(*sermon);
    if (saved) {
        presentation_.register_sermon(*saved);
        if (presentation_.program().live_cue.has_value() &&
            presentation_.program().live_cue->cue == cue) {
            presentation_.clear();
        }
        refresh_cues();
        update_program_status();
    }
}

}  // namespace shekinah::ui
