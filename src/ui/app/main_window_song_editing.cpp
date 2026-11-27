#include "shekinah/ui/main_window.hpp"

#include "song_authoring_coordinator.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/storage/song_repository.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/reflow_editor_dialog.hpp"

#include <string>
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
    const auto edit_state = song_authoring_coordinator_->load_for_edit(id);
    if (!edit_state.has_value()) {
        return;
    }

    auto* editor = new ReflowEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->load_song(edit_state->existing.id);
    editor->set_title(QString::fromStdString(edit_state->existing.title));
    editor->set_reflow_document(edit_state->reflow);
    QObject::connect(editor, &QDialog::accepted, this, [this, editor, existing = edit_state->existing] {
        const auto draft = editor->draft_song();
        if (!draft) {
            return;
        }
        const auto saved = song_authoring_coordinator_->save_edited_song(*draft, existing);
        if (saved) {
            refresh_library();
            refresh_cues();
            update_program_status();
        }
    });
    editor->show();
}

}  // namespace shekinah::ui
