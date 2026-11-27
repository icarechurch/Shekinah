#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/song_authoring.hpp"
#include "content_id_allocator.hpp"
#include "song_import_coordinator.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/reflow_editor_dialog.hpp"

#include <QDialog>
#include <QFileDialog>

namespace shekinah::ui {

void ShekinahWindow::import_song_text() {
    if (!songs()) {
        return;
    }
    const auto path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Import Song Text"), {}, QStringLiteral("Text files (*.txt *.lyrics);;All files (*)"));
    if (path.isEmpty()) {
        return;
    }
    const auto prepared = song_import_coordinator_->prepare_file(
        content_id_allocator_->allocate_song_id(), path.toStdString());
    if (!prepared) {
        return;
    }
    const auto imported = prepared.value();

    auto* editor = new ReflowEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->load_song(imported.id);
    editor->set_title(QString::fromStdString(imported.title));
    editor->set_reflow_document(application::reflow_document_for_song(imported));
    QObject::connect(editor, &QDialog::accepted, this, [this, editor] {
        const auto draft = editor->draft_song();
        if (!draft || !songs()) {
            return;
        }
        const auto saved = song_import_coordinator_->save_imported(*draft);
        if (saved) {
            library_dock_->add_song(*saved);
        }
    });
    editor->show();
}

}  // namespace shekinah::ui
