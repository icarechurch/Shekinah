#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/song_authoring.hpp"
#include "shekinah/application/song_text_importer.hpp"
#include "shekinah/storage/asset_importer.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/storage/project_store.hpp"
#include "shekinah/storage/sermon_deck_importer.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/reflow_editor_dialog.hpp"
#include "shekinah/ui/sermon_editor_dialog.hpp"

#include <QFileDialog>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace shekinah::ui {

void ShekinahWindow::import_song_text() {
    if (!repository()) {
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
        if (!draft || !repository()) {
            return;
        }
        auto song = *draft;
        application::normalize_song_cue_ids(song);
        const auto saved = repository()->save_song(song);
        if (saved) {
            library_dock_->add_song(*saved);
        }
    });
    editor->show();
}

domain::Result<domain::Song> ShekinahWindow::import_song_file(
    const std::filesystem::path& source) {
    if (!repository()) {
        return domain::Result<domain::Song>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before importing songs"});
    }
    std::ifstream input(source, std::ios::binary);
    if (!input) {
        return domain::Result<domain::Song>::fail(
            {domain::ErrorCode::io_failure, "Unable to open song text"});
    }
    const std::string text{std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}};
    const auto parsed = application::SongTextImporter::parse(text);
    if (!parsed) {
        return domain::Result<domain::Song>::fail(parsed.error());
    }
    auto song = parsed.value();
    song.id = domain::SongId{allocate_song_id()};
    application::normalize_song_cue_ids(song);
    const auto saved = repository()->save_song(song);
    if (!saved) {
        return domain::Result<domain::Song>::fail(saved.error());
    }
    return saved;
}

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

