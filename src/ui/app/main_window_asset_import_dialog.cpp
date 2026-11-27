#include "shekinah/ui/main_window.hpp"

#include "shekinah/storage/project_store.hpp"
#include "shekinah/ui/library_dock_widget.hpp"

#include <QFileDialog>
#include <QMessageBox>

namespace shekinah::ui {

void ShekinahWindow::import_asset() {
    if (!store()) {
        return;
    }
    const auto path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Import Asset"), {},
        QStringLiteral(
            "Media (*.png *.jpg *.jpeg *.webp *.bmp *.gif *.tif *.tiff *.mp4);;All files (*)"));
    if (path.isEmpty()) {
        return;
    }
    const auto imported = import_asset_file(path.toStdString());
    if (imported) {
        refresh_library();
        library_dock_->show_assets();
    } else {
        QMessageBox::warning(
            this, QStringLiteral("Asset import failed"), QString::fromStdString(imported.error().message));
    }
}

}  // namespace shekinah::ui
