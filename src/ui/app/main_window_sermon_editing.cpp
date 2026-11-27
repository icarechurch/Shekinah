#include "shekinah/ui/main_window.hpp"

#include "content_persistence_coordinator.hpp"
#include "shekinah/storage/asset_repository.hpp"
#include "shekinah/storage/sermon_repository.hpp"
#include "shekinah/ui/sermon_editor_dialog.hpp"

#include <QDialog>

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace shekinah::ui {

void ShekinahWindow::edit_sermon(const domain::SermonId& id) {
    if (!project_open() || !sermons() || !assets()) {
        return;
    }
    const auto existing = sermons()->load_deck(id);
    if (!existing) {
        return;
    }
    std::vector<std::filesystem::path> files;
    files.reserve(existing->slides.size());
    for (const auto& slide : existing->slides) {
        const auto path = assets()->resolve_asset_path(slide.image);
        if (!path) {
            return;
        }
        files.push_back(*path);
    }

    auto* editor = new SermonEditorDialog{this};
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->set_title(QString::fromStdString(existing->title));
    editor->set_files(std::move(files));
    QObject::connect(editor, &QDialog::accepted, this, [this, editor, existing] {
        if (!project_open() || !sermons()) {
            return;
        }
        const auto files = editor->files();
        std::vector<domain::SermonSlide> slides;
        slides.reserve(files.size());
        for (std::size_t index = 0; index != files.size(); ++index) {
            const auto imported = import_asset_file(files[index]);
            if (!imported) {
                return;
            }
            slides.push_back(domain::SermonSlide{
                domain::CueId{"slide-" + std::to_string(index)}, imported->id});
        }
        auto updated = *existing;
        updated.title = editor->title().toStdString();
        updated.slides = std::move(slides);
        const auto saved = content_persistence_coordinator_->save_sermon(updated);
        if (saved) {
            refresh_library();
            refresh_cues();
            update_program_status();
        }
    });
    editor->show();
}

}  // namespace shekinah::ui
