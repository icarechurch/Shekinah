#include "shekinah/ui/main_window.hpp"

#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/sermon_editor_dialog.hpp"

#include <QDialog>

namespace shekinah::ui {

void ShekinahWindow::create_sermon() {
    if (!project_open()) {
        return;
    }
    auto* editor = new SermonEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->set_title(QStringLiteral("New Sermon"));
    QObject::connect(editor, &QDialog::accepted, this, [this, editor] {
        const auto imported = import_sermon_files(
            editor->files(), editor->title().toStdString(), true);
        if (imported) {
            library_dock_->add_sermon(*imported);
        }
    });
    editor->show();
}

}  // namespace shekinah::ui
