#include "shekinah/ui/main_window.hpp"

#include "content_persistence_coordinator.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"
#include "shekinah/ui/operator_page.hpp"

#include <algorithm>

#include <string>
#include <utility>
#include <vector>

namespace shekinah::ui {

void ShekinahWindow::show_inspector(const domain::CueId& cue) {
    if (!songs() || !sermons()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    if (selected->content.kind == domain::ContentKind::song) {
        const auto song_id = std::get<domain::SongId>(selected->content.id);
        const auto song = songs()->load_song(song_id);
        if (!song) {
            return;
        }
        const auto slide = std::find_if(song->slides.begin(), song->slides.end(), [&](const auto& candidate) {
            return candidate.id == cue;
        });
        if (slide == song->slides.end()) {
            return;
        }
        const auto fit = slide->background.has_value() ? slide->background->fit : domain::FitMode::cover;
        operator_page_->inspector()->show_slide(
            selected->content, slide->id, slide->section, slide->lines, fit);
        return;
    }

    const auto sermon_id = std::get<domain::SermonId>(selected->content.id);
    const auto sermon = sermons()->load_deck(sermon_id);
    if (!sermon) {
        return;
    }
    const auto slide = std::find_if(sermon->slides.begin(), sermon->slides.end(), [&](const auto& candidate) {
        return candidate.id == cue;
    });
    if (slide != sermon->slides.end()) {
        operator_page_->inspector()->show_slide(
            selected->content, slide->id, "Sermon slide", {slide->image.value}, domain::FitMode::cover);
    }
}

void ShekinahWindow::save_inspected_slide(domain::CueId cue,
                                          std::string section,
                                          std::vector<std::string> lines,
                                          domain::FitMode fit) {
    if (!songs()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return;
    }
    const auto song_id = std::get<domain::SongId>(selected->content.id);
    auto song = songs()->load_song(song_id);
    if (!song) {
        return;
    }
    const auto slide = std::find_if(song->slides.begin(), song->slides.end(), [&](const auto& candidate) {
        return candidate.id == cue;
    });
    if (slide == song->slides.end()) {
        return;
    }
    slide->section = std::move(section);
    slide->lines = std::move(lines);
    if (slide->background.has_value()) {
        slide->background->fit = fit;
    }
    const auto saved = content_persistence_coordinator_->save_song(*song);
    if (!saved) {
        return;
    }
    refresh_library();
    refresh_cues();
    if (presentation_.program().live_cue.has_value() &&
        presentation_.program().live_cue->cue == cue) {
        update_program_status();
    }
}

}  // namespace shekinah::ui
