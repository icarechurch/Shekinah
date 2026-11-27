#include "shekinah/ui/main_window.hpp"

#include "sermon_authoring_coordinator.hpp"
#include "shekinah/ui/sermon_editor_dialog.hpp"

#include <QDialog>

#include <utility>

namespace shekinah::ui {

void ShekinahWindow::edit_sermon(const domain::SermonId& id) {
    const auto edit_state = sermon_authoring_coordinator_->load_for_edit(id);
    if (!edit_state.has_value()) {
        return;
    }

    auto* editor = new SermonEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->set_title(QString::fromStdString(edit_state->existing.title));
    editor->set_files(std::move(edit_state->files));
    QObject::connect(editor, &QDialog::accepted,
                     this, [this, editor, existing = edit_state->existing] {
        const auto saved = sermon_authoring_coordinator_->save_edited_deck(
            existing, editor->title().toStdString(), editor->files());
        if (saved) {
            refresh_library();
            refresh_cues();
            update_program_status();
        }
    });
    editor->show();
}

}  // namespace shekinah::ui
