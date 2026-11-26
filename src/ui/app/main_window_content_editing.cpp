#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/song_authoring.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/reflow_editor_dialog.hpp"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace shekinah::ui {

domain::Status ShekinahWindow::set_slide_background(
    const domain::CueId& cue, const domain::AssetId& asset) {
    if (!repository()) {
        return domain::Status::fail(
            {domain::ErrorCode::storage_failure, "Open a project before setting a background"});
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return domain::Status::fail(
            {domain::ErrorCode::invalid_input, "Only selected Song cues accept editable backgrounds"});
    }
    const auto asset_record = repository()->load_asset(asset);
    if (!asset_record) {
        return domain::Status::fail(asset_record.error());
    }
    auto song = repository()->load_song(std::get<domain::SongId>(selected->content.id));
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
    const auto saved = repository()->save_song(*song);
    if (!saved) {
        return domain::Status::fail(saved.error());
    }
    presentation_.register_song(*saved);
    if (presentation_.program().live_cue.has_value() &&
        presentation_.program().live_cue->cue == cue) {
        update_program_status();
    }
    return domain::success();
}

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

std::string ShekinahWindow::allocate_song_id() {
    for (;;) {
        const auto candidate = "song-" + std::to_string(next_content_id_++);
        if (!repository() || !repository()->load_song(domain::SongId{candidate})) {
            return candidate;
        }
    }
}

std::string ShekinahWindow::allocate_sermon_id() {
    for (;;) {
        const auto candidate = "sermon-" + std::to_string(next_content_id_++);
        if (!repository() || !repository()->load_deck(domain::SermonId{candidate})) {
            return candidate;
        }
    }
}

void ShekinahWindow::create_song() {
    if (!repository()) {
        return;
    }
    auto* editor = new ReflowEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->load_song(domain::SongId{allocate_song_id()});
    editor->set_title(QStringLiteral("Untitled Song"));
    QObject::connect(editor, &QDialog::accepted, this, [this, editor] {
        const auto draft = editor->draft_song();
        if (!draft || !repository()) {
            return;
        }
        auto song = *draft;
        application::normalize_song_cue_ids(song);
        const auto saved = repository()->save_song(song);
        if (saved) {
            library_dock_->add_song(*saved);
        }
    });
    editor->show();
}

void ShekinahWindow::edit_song(const domain::SongId& id) {
    if (!repository()) {
        return;
    }
    const auto existing = repository()->load_song(id);
    const auto reflow = repository()->load_song_reflow(id);
    if (!existing || !reflow) {
        return;
    }

    auto* editor = new ReflowEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->load_song(existing->id);
    editor->set_title(QString::fromStdString(existing->title));
    editor->set_reflow_document(*reflow);
    QObject::connect(editor, &QDialog::accepted, this, [this, editor, existing] {
        const auto draft = editor->draft_song();
        if (!draft || !repository()) {
            return;
        }
        auto song = *draft;
        song.default_background = existing->default_background;
        for (std::size_t index = 0; index != song.slides.size(); ++index) {
            if (index < existing->slides.size()) {
                song.slides[index].id = existing->slides[index].id;
                song.slides[index].background = existing->slides[index].background;
            } else {
                song.slides[index].id = domain::CueId{
                    song.id.value + "-slide-" + std::to_string(index)};
            }
        }
        const auto saved = repository()->save_song(song);
        if (saved) {
            refresh_library();
            presentation_.register_song(*saved);
            refresh_cues();
            update_program_status();
        }
    });
    editor->show();
}


}  // namespace shekinah::ui

