#include "shekinah/ui/sermon_editor_dialog.hpp"

#include <QFileDialog>
#include <QFileInfo>
#include <QListWidget>

namespace shekinah::ui {

void SermonEditorDialog::set_files(std::vector<std::filesystem::path> files) {
    files_->clear();
    for (auto& path : files) {
        auto* item = new QListWidgetItem{QString::fromStdString(path.filename().string()), files_};
        item->setData(Qt::UserRole, QString::fromStdString(path.string()));
    }
}

std::vector<std::filesystem::path> SermonEditorDialog::files() const {
    std::vector<std::filesystem::path> values;
    values.reserve(static_cast<std::size_t>(files_->count()));
    for (int index = 0; index != files_->count(); ++index) {
        values.emplace_back(files_->item(index)->data(Qt::UserRole).toString().toStdString());
    }
    return values;
}

void SermonEditorDialog::add_files() {
    const auto paths = QFileDialog::getOpenFileNames(
        this, QStringLiteral("Add Sermon Media"), {},
        QStringLiteral(
            "Media (*.png *.jpg *.jpeg *.webp *.bmp *.gif *.tif *.tiff *.mp4);;All files (*)"));
    for (const auto& path : paths) {
        bool duplicate = false;
        for (int index = 0; index != files_->count(); ++index) {
            duplicate = files_->item(index)->data(Qt::UserRole).toString() == path;
            if (duplicate) {
                break;
            }
        }
        if (!duplicate) {
            auto* item = new QListWidgetItem{QFileInfo{path}.fileName(), files_};
            item->setData(Qt::UserRole, path);
        }
    }
}

void SermonEditorDialog::move_current(int direction) {
    const int current = files_->currentRow();
    const int target = current + direction;
    if (current < 0 || target < 0 || target >= files_->count()) {
        return;
    }
    auto* item = files_->takeItem(current);
    files_->insertItem(target, item);
    files_->setCurrentRow(target);
}

void SermonEditorDialog::remove_current() {
    const int current = files_->currentRow();
    if (current >= 0) {
        delete files_->takeItem(current);
    }
}

}  // namespace shekinah::ui
