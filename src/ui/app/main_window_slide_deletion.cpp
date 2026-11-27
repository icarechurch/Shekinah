#include "shekinah/ui/main_window.hpp"

#include "content_persistence_coordinator.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"

#include <algorithm>

namespace shekinah::ui {

void ShekinahWindow::delete_slide(const domain::CueId& cue) {
    if (!songs() || !sermons()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    if (selected->content.kind == domain::ContentKind::song) {
        const auto id = std::get<domain::SongId>(selected->content.id);
        auto song = songs()->load_song(id);
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
        const auto saved = content_persistence_coordinator_->save_song(*song);
        if (saved) {
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
    auto sermon = sermons()->load_deck(id);
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
    const auto saved = content_persistence_coordinator_->save_sermon(*sermon);
    if (saved) {
        if (presentation_.program().live_cue.has_value() &&
            presentation_.program().live_cue->cue == cue) {
            presentation_.clear();
        }
        refresh_cues();
        update_program_status();
    }
}

}  // namespace shekinah::ui
