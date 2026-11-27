#include "shekinah/ui/main_window.hpp"

#include "shekinah/storage/asset_importer.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/ui/sermon_editor_dialog.hpp"

#include <QDialog>

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace shekinah::ui {

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
