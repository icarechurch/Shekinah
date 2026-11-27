#include "shekinah/ui/sermon_editor_dialog.hpp"

#include <QLineEdit>
#include <QListWidget>

#include <utility>

namespace shekinah::ui {

void SermonEditorDialog::set_title(QString title) { title_->setText(std::move(title)); }

QString SermonEditorDialog::title() const { return title_->text().trimmed(); }

int SermonEditorDialog::file_count() const noexcept { return files_->count(); }

void SermonEditorDialog::accept_if_valid() {
    if (!title().isEmpty() && files_->count() != 0) {
        accept();
    }
}

}  // namespace shekinah::ui
