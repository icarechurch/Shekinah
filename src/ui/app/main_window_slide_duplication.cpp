#include "shekinah/ui/main_window.hpp"

#include "shekinah/storage/content_repository.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace shekinah::ui {

void ShekinahWindow::duplicate_slide(const domain::CueId& cue) {
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
        if (!song) {
            return;
        }
        const auto position = std::find_if(song->slides.begin(), song->slides.end(), [&](const auto& slide) {
            return slide.id == cue;
        });
        if (position == song->slides.end()) {
            return;
        }
        auto duplicate = *position;
        duplicate.id = domain::CueId{song->id.value + "-copy-" + std::to_string(song->slides.size())};
        song->slides.insert(position + 1, std::move(duplicate));
        const auto saved = repository()->save_song(*song);
        if (saved) {
            presentation_.register_song(*saved);
            refresh_cues();
        }
        return;
    }

    const auto id = std::get<domain::SermonId>(selected->content.id);
    auto sermon = repository()->load_deck(id);
    if (!sermon || sermon->slides.size() == 0) {
        return;
    }
    const auto position = std::find_if(sermon->slides.begin(), sermon->slides.end(), [&](const auto& slide) {
        return slide.id == cue;
    });
    if (position == sermon->slides.end()) {
        return;
    }
    auto duplicate = *position;
    duplicate.id = domain::CueId{sermon->id.value + "-copy-" + std::to_string(sermon->slides.size())};
    sermon->slides.insert(position + 1, std::move(duplicate));
    const auto saved = repository()->save_deck(*sermon);
    if (saved) {
        presentation_.register_sermon(*saved);
        refresh_cues();
    }
}

}  // namespace shekinah::ui
