#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/song_authoring.hpp"
#include "content_persistence_coordinator.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/reflow_editor_dialog.hpp"

#include <string>
#include <utility>

namespace shekinah::ui {

std::string ShekinahWindow::allocate_song_id() {
    for (;;) {
        const auto candidate = "song-" + std::to_string(next_content_id_++);
        if (!songs() || !songs()->load_song(domain::SongId{candidate})) {
            return candidate;
        }
    }
}

std::string ShekinahWindow::allocate_sermon_id() {
    for (;;) {
        const auto candidate = "sermon-" + std::to_string(next_content_id_++);
        if (!sermons() || !sermons()->load_deck(domain::SermonId{candidate})) {
            return candidate;
        }
    }
}

void ShekinahWindow::edit_song(const domain::SongId& id) {
    if (!songs()) {
        return;
    }
    const auto existing = songs()->load_song(id);
    const auto reflow = songs()->load_song_reflow(id);
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
        if (!draft || !songs()) {
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
        const auto saved = content_persistence_coordinator_->save_song(song);
        if (saved) {
            refresh_library();
            refresh_cues();
            update_program_status();
        }
    });
    editor->show();
}

}  // namespace shekinah::ui
