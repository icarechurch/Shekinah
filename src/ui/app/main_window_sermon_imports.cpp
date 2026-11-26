#include "shekinah/ui/main_window.hpp"

#include "shekinah/storage/asset_importer.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/storage/sermon_deck_importer.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/sermon_editor_dialog.hpp"

#include <QFileDialog>

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace shekinah::ui {

void ShekinahWindow::import_sermon_deck() {
    if (!store()) {
        return;
    }
    const auto paths = QFileDialog::getOpenFileNames(
        this, QStringLiteral("Import Sermon Deck"), {},
        QStringLiteral(
            "Media (*.png *.jpg *.jpeg *.webp *.bmp *.gif *.tif *.tiff *.mp4);;All files (*)"));
    if (paths.isEmpty()) {
        return;
    }
    std::vector<std::filesystem::path> files;
    files.reserve(static_cast<std::size_t>(paths.size()));
    for (const auto& path : paths) {
        files.emplace_back(path.toStdString());
    }
    auto* editor = new SermonEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->set_title(QStringLiteral("Imported Sermon"));
    editor->set_files(std::move(files));
    QObject::connect(editor, &QDialog::accepted, this, [this, editor] {
        const auto imported = import_sermon_files(
            editor->files(), editor->title().toStdString(), true);
        if (imported) {
            library_dock_->add_sermon(*imported);
        }
    });
    editor->show();
}

domain::Result<domain::SermonDeck> ShekinahWindow::import_sermon_files(
    std::vector<std::filesystem::path> sources, std::string title, bool preserve_order) {
    if (!store()) {
        return domain::Result<domain::SermonDeck>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before importing sermons"});
    }
    return storage::SermonDeckImporter{*store()}.import_files(
        domain::SermonId{allocate_sermon_id()}, std::move(title),
        std::move(sources), preserve_order);
}

void ShekinahWindow::create_sermon() {
    if (!store()) {
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

void ShekinahWindow::edit_sermon(const domain::SermonId& id) {
    if (!store() || !repository()) {
        return;
    }
    const auto existing = repository()->load_deck(id);
    if (!existing) {
        return;
    }
    std::vector<std::filesystem::path> files;
    files.reserve(existing->slides.size());
    for (const auto& slide : existing->slides) {
        const auto asset = repository()->load_asset(slide.image);
        if (!asset) {
            return;
        }
        files.push_back(store()->root() / asset->relative_path);
    }

    auto* editor = new SermonEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->set_title(QString::fromStdString(existing->title));
    editor->set_files(std::move(files));
    QObject::connect(editor, &QDialog::accepted, this, [this, editor, existing] {
        if (!store() || !repository()) {
            return;
        }
        const auto files = editor->files();
        std::vector<domain::SermonSlide> slides;
        slides.reserve(files.size());
        storage::AssetImporter assets{*store()};
        for (std::size_t index = 0; index != files.size(); ++index) {
            const auto imported = assets.import_file(files[index]);
            if (!imported) {
                return;
            }
            slides.push_back(domain::SermonSlide{
                domain::CueId{"slide-" + std::to_string(index)}, imported->id});
        }
        auto updated = *existing;
        updated.title = editor->title().toStdString();
        updated.slides = std::move(slides);
        const auto saved = repository()->save_deck(updated);
        if (saved) {
            refresh_library();
            presentation_.register_sermon(*saved);
            refresh_cues();
            update_program_status();
        }
    });
    editor->show();
}

}  // namespace shekinah::ui
