#include "shekinah/ui/main_window.hpp"

#include "shekinah/storage/content_repository.hpp"
#include "shekinah/ui/operator_page.hpp"

#include <algorithm>

#include <string>
#include <utility>
#include <vector>

namespace shekinah::ui {

void ShekinahWindow::show_inspector(const domain::CueId& cue) {
    if (!repository()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    if (selected->content.kind == domain::ContentKind::song) {
        const auto song_id = std::get<domain::SongId>(selected->content.id);
        const auto song = repository()->load_song(song_id);
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
    const auto sermon = repository()->load_deck(sermon_id);
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
    if (!repository()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return;
    }
    const auto song_id = std::get<domain::SongId>(selected->content.id);
    auto song = repository()->load_song(song_id);
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
    const auto saved = repository()->save_song(*song);
    if (!saved) {
        return;
    }
    presentation_.register_song(*saved);
    refresh_library();
    refresh_cues();
    if (presentation_.program().live_cue.has_value() &&
        presentation_.program().live_cue->cue == cue) {
        update_program_status();
    }
}

}  // namespace shekinah::ui
