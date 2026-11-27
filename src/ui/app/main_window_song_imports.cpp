#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/song_authoring.hpp"
#include "shekinah/application/song_text_importer.hpp"
#include "shekinah/storage/song_repository.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/reflow_editor_dialog.hpp"

#include <QDialog>
#include <QFileDialog>

#include <fstream>
#include <iterator>
#include <string>

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
    std::ifstream input(path.toStdString(), std::ios::binary);
    if (!input) {
        return;
    }
    const std::string text{std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}};
    const auto parsed = application::SongTextImporter::parse(text);
    if (!parsed) {
        return;
    }
    auto imported = parsed.value();
    imported.id = domain::SongId{allocate_song_id()};
    application::normalize_song_cue_ids(imported);

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
        auto song = *draft;
        application::normalize_song_cue_ids(song);
        const auto saved = songs()->save_song(song);
        if (saved) {
            library_dock_->add_song(*saved);
        }
    });
    editor->show();
}

}  // namespace shekinah::ui
