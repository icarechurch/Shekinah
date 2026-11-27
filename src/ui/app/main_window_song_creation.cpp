#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/song_authoring.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/reflow_editor_dialog.hpp"

namespace shekinah::ui {

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

}  // namespace shekinah::ui
